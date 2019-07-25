//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Physically Based Rendering shader for brushes and models
//
//==================================================================================================

// Bugs:
// Cubemap and ambient cube can change on models based on flashlight state and direction you're looking.

// Includes for all shaders.
#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"

// Includes specific to this shader.
#include "pbr_ps30.inc"
#include "pbr_vs20.inc"

// Defining samplers.
#define SAMPLER_ALBEDO SHADER_SAMPLER0
#define SAMPLER_NORMAL SHADER_SAMPLER1
#define SAMPLER_MRAO SHADER_SAMPLER2
#define SAMPLER_EMISSIVE SHADER_SAMPLER3
#define SAMPLER_ENVMAP SHADER_SAMPLER4
#define SAMPLER_FLASHLIGHT SHADER_SAMPLER5
#define SAMPLER_SHADOWDEPTH SHADER_SAMPLER6
#define SAMPLER_RANDOMROTATION SHADER_SAMPLER7
#define SAMPLER_LIGHTMAP SHADER_SAMPLER8

// Convars.
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);

// Variables for this shader.
struct PBR_DX9_Vars_t
{
    PBR_DX9_Vars_t() { memset(this, 0xFF, sizeof(*this)); }

    int Albedo;
    int Normal;
    int Mrao;
    int Emissive;
    int Envmap;
    int AlphaTestReference;
    int BaseTextureFrame;
    int BaseTextureTransform;
    int FlashlightTexture;
    int FlashlightTextureFrame;
};

// Beginning the shader.
DEFINE_FALLBACK_SHADER(PBR, PBR_DX9)
BEGIN_VS_SHADER(PBR_DX9, "Physically Based Rendering shader for brushes and models")

// Setting up vmt parameters.
BEGIN_SHADER_PARAMS
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "Alpha Test Reference")
SHADER_PARAM(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Albedo Texture")      // changing this name breaks basetexturetransform
SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Bump Map")                // changing this name breaks dynamic lighting
SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal Map")        // this is here for backwards compatibility
SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Metal/Rough/AO Map")  // changing this name breaks backwards compatibility
SHADER_PARAM(EMISSIVE, SHADER_PARAM_TYPE_TEXTURE, "", "Emissive Texture")       // you could probably get away with changing this name
SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Environment Map")          // don't try it for this though
END_SHADER_PARAMS

// Setting up variables for this shader.
void SetupVars(PBR_DX9_Vars_t &info)
{
    info.AlphaTestReference = ALPHATESTREFERENCE;
    info.Albedo = BASETEXTURE;
    info.Normal = BUMPMAP;
    info.Mrao = MRAOTEXTURE;
    info.Emissive = EMISSIVE;
    info.Envmap = ENVMAP;
    info.BaseTextureFrame = FRAME;
    info.BaseTextureTransform = BASETEXTURETRANSFORM;
    info.FlashlightTexture = FLASHLIGHTTEXTURE;
    info.FlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
}

// Initializing parameters.
SHADER_INIT_PARAMS()
{
    // This is here for backwards compatibility with Empires Mod.
    if (params[NORMALTEXTURE]->IsDefined())
    {
        params[BUMPMAP]->SetStringValue(params[NORMALTEXTURE]->GetStringValue());
    }

    // Without this, dynamic lighting breaks.
    if (!params[BUMPMAP]->IsDefined())
    {
        params[BUMPMAP]->SetStringValue("dev/flat_normal");
    }

    // Check if the hardware supports flashlight border color.
    if (g_pHardwareConfig->SupportsBorderColor())
    {
        params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
    }
    else
    {
        params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
    }

    // Set material var2 flags specific to models.
    if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
    {
        SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning.
        SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting.
        SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting.
        SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube.
    }

    // Set material var2 flags specific to brushes.
    else
    {
        SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps.
        SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps.
    }
}

// Telling the game what shader to use in case this one doesn't work.
SHADER_FALLBACK { return 0; }

// Initializing shader.
SHADER_INIT
{
    PBR_DX9_Vars_t info;
    SetupVars(info);

    // Setting up albedo texture.
    bool bIsAlbedoTranslucent = false;
    if (params[info.Albedo]->IsDefined())
    {
        LoadTexture(info.Albedo, TEXTUREFLAGS_SRGB); // Albedo is sRGB.

        // If albedo is translucent, save that info for later.
        if (params[info.Albedo]->GetTextureValue()->IsTranslucent())
        {
            bIsAlbedoTranslucent = true;
        }
    }

    // Setting up normal Map
    if (info.Normal != -1 && params[info.Normal]->IsDefined())
    {
        LoadBumpMap(info.Normal); // Normal is a bump map.
    }

    // Setting up mrao map.
    if (info.Mrao != -1 && params[info.Mrao]->IsDefined())
    {
        LoadTexture(info.Mrao);
    }

    // Setting up emissive texture.
    if (info.Emissive != -1 && params[info.Emissive]->IsDefined())
    {
        LoadTexture(info.Emissive, TEXTUREFLAGS_SRGB); // Emissive is sRGB
    }

    // Setting up environment map.
    if (info.Envmap != -1 && params[info.Envmap]->IsDefined())
    {
        // These two flags only work on custom cubemaps, not on env_cubemap.
        int flags = TEXTUREFLAGS_ALL_MIPS | TEXTUREFLAGS_NOLOD;
        if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
        {
            flags |= TEXTUREFLAGS_SRGB; // Envmap is only sRGB with HDR disabled?
        }

        // If the hardware doesn't support cubemaps, use spheremaps instead.
        if (!g_pHardwareConfig->SupportsCubeMaps())
        {
            SET_FLAGS(MATERIAL_VAR_ENVMAPSPHERE);
        }

        // Done like this so the user could set $envmapsphere themselves if they wanted to.
        if (!IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE))
        {
            LoadCubeMap(info.Envmap, flags);
        }
        else
        {
            LoadTexture(info.Envmap, flags);
        }
    }

    // Setting up flashlight texture.
    if (info.FlashlightTexture != -1 && params[info.FlashlightTexture]->IsDefined())
    {
        LoadTexture(info.FlashlightTexture, TEXTUREFLAGS_SRGB); // Flashlight texture is sRGB.
    }
};

// Drawing the shader.
SHADER_DRAW
{
    PBR_DX9_Vars_t info;
    SetupVars(info);

    // Setting up some booleans.
    bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
    bool bHasAlbedo = (info.Albedo != -1) && params[info.Albedo]->IsTexture();
    bool bHasNormal = (info.Normal != -1) && params[info.Normal]->IsTexture();
    bool bHasMrao = (info.Mrao != -1) && params[info.Mrao]->IsTexture();
    bool bHasEmissive = (info.Emissive != -1) && params[info.Emissive]->IsTexture();
    bool bHasEnvmap = (info.Envmap != -1) && params[info.Envmap]->IsTexture();
    bool bHasFlashlight = UsingFlashlight(params);
    bool bLightmapped = IS_FLAG_SET(MATERIAL_VAR_MODEL) == 0;

    // Determining whether we're dealing with a fully opaque material.
    BlendType_t nBlendType = EvaluateBlendRequirements(info.Albedo, true);
    bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;

    SHADOW_STATE
    {
        // If alphatest is on, enable it.
        pShaderShadow->EnableAlphaTest(bIsAlphaTested);

        // If the user set an alpha reference, use it.
        if (info.AlphaTestReference != -1 && params[info.AlphaTestReference]->GetFloatValue() > 0.0f)
        {
            pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.AlphaTestReference]->GetFloatValue());
        }

        // If the flashlight is on, get the shadow filter mode.
        int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

        // If the albedo texture exists, set the default blendding shadow state.
        if (params[info.Albedo]->IsTexture())
        {
            SetDefaultBlendingShadowState(info.Albedo, true);
        }

        // Setting up samplers.
        pShaderShadow->EnableTexture(SAMPLER_ALBEDO, true);
        pShaderShadow->EnableSRGBRead(SAMPLER_ALBEDO, true);

        pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);

        pShaderShadow->EnableTexture(SAMPLER_MRAO, true);

        pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);
        pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);

        // Setting up envmap.
        if (bHasEnvmap)
        {
            pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true);
            if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
            {
                pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
            }
        }

        // If the flashlight is on, set up its textures.
        if (bHasFlashlight)
        {
            pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHT, true);      // Flashlight cookie.
            pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHT, true);     // Flashlight cookie is sRGB.
            pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);     // Shadow depth map.
            pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH); // Set shadow depth filtering.
            pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);   // Shadow depth map is not sRGB.
            pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);  // Random rotation map.
        }

        // If lightmaps are used, enable the sampler.
        if (bLightmapped)
        {
            pShaderShadow->EnableTexture(SAMPLER_LIGHTMAP, true);
        }

        // Enabling sRGB writing.
        // See common_ps_fxc.h line 349.
        // PS2b shaders and up write sRGB.
        pShaderShadow->EnableSRGBWrite(true);

        // Set up vertex format.
        if (bLightmapped) // Brush.
        {
            // We only need the position and surface normal.
            unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
            // We need three texcoords, all in the default float2 size.
            pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);
        }
        else // Model.
        {
            // We need the position, surface normal, and vertex compression format.
            unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
            // We only need one texcoord, in the default float2 size.
            pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
        }

        // Setting up static vertex shader.
        DECLARE_STATIC_VERTEX_SHADER(pbr_vs20);
        SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAPPED, bLightmapped);
        SET_STATIC_VERTEX_SHADER(pbr_vs20);

        // Setting up static pixel shader.
        DECLARE_STATIC_PIXEL_SHADER(pbr_ps30);
        SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
        SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
        SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightmapped);
        SET_STATIC_PIXEL_SHADER(pbr_ps30);

        // Setting up fog.
        DefaultFog(); // This is probably correct.

        // Enable alpha writes all the time so that we have them for underwater stuff.
        pShaderShadow->EnableAlphaWrites(bFullyOpaque);
    }

    DYNAMIC_STATE
    {
        // Setting up albedo texture.
        if (bHasAlbedo)
        {
            BindTexture(SAMPLER_ALBEDO, info.Albedo, info.BaseTextureFrame);
        }
        else
        {
            pShaderAPI->BindStandardTexture(SAMPLER_ALBEDO, TEXTURE_GREY);
        }

        // Setting up normal map.
        if (bHasNormal)
        {
            BindTexture(SAMPLER_NORMAL, info.Normal);
        }
        else
        {
            pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
        }

        // Setting up mrao map.
        if (bHasMrao)
        {
            BindTexture(SAMPLER_MRAO, info.Mrao);
        }
        else
        {
            pShaderAPI->BindStandardTexture(SAMPLER_MRAO, TEXTURE_WHITE);
        }

        // Setting up emissive texture.
        if (bHasEmissive)
        {
            BindTexture(SAMPLER_EMISSIVE, info.Emissive);
        }
        else
        {
            pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE, TEXTURE_BLACK);
        }

        // Setting up environment map.
        if (bHasEnvmap)
        {
            BindTexture(SAMPLER_ENVMAP, info.Envmap);
        }
        else
        {
            pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_GREY);
        }

        // Setting up the flashlight related textures and variables.
        bool bFlashlightShadows = false;
        if (bHasFlashlight)
        {
            Assert(info.FlashlightTexture >= 0 && info.FlashlightTextureFrame >= 0);
            BindTexture(SAMPLER_FLASHLIGHT, info.FlashlightTexture, info.FlashlightTextureFrame);

            VMatrix worldToTexture;
            ITexture *pFlashlightDepthTexture;
            FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);

            bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);
            SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

            if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
            {
                BindTexture(SAMPLER_SHADOWDEPTH, pFlashlightDepthTexture, 0);
                pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
            }
        }

        // Setting lightmap texture.
        if (bLightmapped)
        {
            pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED);
        }

        // Getting the light state.
        LightState_t lightState;
        pShaderAPI->GetDX9LightState(&lightState);

        // Getting fog info.
        MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
        int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

        // Some debugging stuff I think.
        bool bWriteDepthToAlpha = false;
        bool bWriteWaterFogToAlpha = false;
        if (bFullyOpaque)
        {
            bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
            bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
            AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha),
                      "Can't write two values to alpha simultaneously.");
        }

        // Getting skinning info.
        int numBones = pShaderAPI->GetCurrentNumBones();

        // Setting up some more booleans.
        bool bNoSpecular = mat_specular.GetInt() == 0 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
        bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
        bool bLightingPreview = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0;

        // Dynamic lighting on brushes is glitchy because they don't set their own light state,
        // and instead use the one from the last drawn model.
        // Besides, the lightmap has diffuse lighting anyway so we're not losing too much.
        // That's why NUM_LIGHTS is forced to 0 when bLightMapped is true.

        // Setting up dynamic vertex shader.
        DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs20);
        SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
        SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
        SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, bLightingPreview);
        SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
        SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights * !bLightmapped);
        SET_DYNAMIC_VERTEX_SHADER(pbr_vs20);

        // Setting up dynamic pixel shader.
        DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps30);
        SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights * !bLightmapped);
        SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
        SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
        SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
        SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
        SET_DYNAMIC_PIXEL_SHADER(pbr_ps30);

        // Setting up base texture transform.
        SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.BaseTextureTransform);

        // This is probably important.
        SetModulationPixelShaderDynamicState_LinearColorSpace(1);

        // Sending ambient cube to the pixel shader.
        pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight);

        // Sending lighting info to the pixel shader.
        pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);

        // Sending fog info to the pixel shader.
        pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

        // Handle mat_specular 0 (no envmap reflections).
        if (bNoSpecular)
        {
            pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
        }

        // Handle mat_fullbright 2 (diffuse lighting only).
        if (bLightingOnly)
        {
            pShaderAPI->BindStandardTexture(SAMPLER_ALBEDO, TEXTURE_GREY);
        }

        // Setting up eye position and sending to the pixel shader.
        float vEyePos_SpecExponent[4];
        pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);
        vEyePos_SpecExponent[3] = 0.0f;
        pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

        // More flashlight related stuff.
        if (bHasFlashlight)
        {
            VMatrix worldToTexture;
            float atten[4], pos[4], tweaks[4];

            const FlashlightState_t &state = pShaderAPI->GetFlashlightState(worldToTexture);
            SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

            BindTexture(SAMPLER_FLASHLIGHT, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);

            // Set the flashlight attenuation factors.
            atten[0] = state.m_fConstantAtten;
            atten[1] = state.m_fLinearAtten;
            atten[2] = state.m_fQuadraticAtten;
            atten[3] = state.m_FarZ;
            pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

            // Set the flashlight origin.
            pos[0] = state.m_vecLightOrigin[0];
            pos[1] = state.m_vecLightOrigin[1];
            pos[2] = state.m_vecLightOrigin[2];
            pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

            pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

            // Tweaks associated with a given flashlight.
            tweaks[0] = ShadowFilterFromState(state);
            tweaks[1] = ShadowAttenFromState(state);
            HashShadow2DJitter(state.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
            pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);

            // Dimensions of screen, used for screen-space noise map sampling.
            float vScreenScale[4] = {1280.0f / 32.0f, 720.0f / 32.0f, 0, 0};
            int nWidth, nHeight;
            pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
            vScreenScale[0] = (float)nWidth / 32.0f;
            vScreenScale[1] = (float)nHeight / 32.0f;
            pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1);
        }

        // Set up shader modulation color.
        float modulationColor[4] = {1.0, 1.0, 1.0, 1.0};
        ComputeModulationColor(modulationColor);
        float flLScale = pShaderAPI->GetLightMapScaleFactor();
        modulationColor[0] *= flLScale;
        modulationColor[1] *= flLScale;
        modulationColor[2] *= flLScale;
        pShaderAPI->SetPixelShaderConstant(PSREG_DIFFUSE_MODULATION, modulationColor);
    }

    // Actually draw the shader.
    Draw();
}

// Closing it off.
END_SHADER
