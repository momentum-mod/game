//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "BaseVSShader.h"
#include "tier1/convar.h"
#include "mathlib/vmatrix.h"
#include "eyes_dx8_dx9_helper.h"
#include "cpp_shader_constant_register_map.h"

#include "eyes_vs20.inc"
#include "eyes_ps20b.inc"
#include "eyes_flashlight_vs20.inc"
#include "eyes_flashlight_ps20b.inc"

#include "eyes_vs30.inc"
#include "eyes_ps30.inc"
#include "eyes_flashlight_vs30.inc"
#include "eyes_flashlight_ps30.inc"

void InitParamsEyes_DX8_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, 
							Eyes_DX8_DX9_Vars_t &info )
{
	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}

	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	Assert( info.m_nIntro != -1 );
	if( info.m_nIntro != -1 && !params[info.m_nIntro]->IsDefined() )
	{
		params[info.m_nIntro]->SetIntValue( 0 );
	}
}

void InitEyes_DX8_DX9( CBaseVSShader *pShader, IMaterialVar** params, Eyes_DX8_DX9_Vars_t &info )
{
	pShader->LoadTexture( FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nIris, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nGlint );

	// Be sure dilation is zeroed if undefined
	if( !params[info.m_nDilation]->IsDefined() )
	{
		params[info.m_nDilation]->SetFloatValue( 0.0f );
	}
}

static void SetDepthFlashlightParams( CBaseVSShader *pShader, IShaderDynamicAPI *pShaderAPI, const VMatrix& worldToTexture, const FlashlightState_t& flashlightState ) 
{
	float atten[4], pos[4], tweaks[4];
	atten[0] = flashlightState.m_fConstantAtten;		// Set the flashlight attenuation factors
	atten[1] = flashlightState.m_fLinearAtten;
	atten[2] = flashlightState.m_fQuadraticAtten;
	atten[3] = flashlightState.m_FarZ;
	pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, atten, 1 );

	pos[0] = flashlightState.m_vecLightOrigin[0];		// Set the flashlight origin
	pos[1] = flashlightState.m_vecLightOrigin[1];
	pos[2] = flashlightState.m_vecLightOrigin[2];
	pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1 );

	pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4 );

	// Tweaks associated with a given flashlight
	tweaks[0] = ShadowFilterFromState( flashlightState );
	tweaks[1] = ShadowAttenFromState( flashlightState );
	pShader->HashShadow2DJitter( flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );
	pShaderAPI->SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1 );

	// Dimensions of screen, used for screen-space noise map sampling
	float vScreenScale[4] = {1280.0f / 32.0f, 720.0f / 32.0f, 0, 0};
	int nWidth, nHeight;
	pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );
	vScreenScale[0] = (float) nWidth  / 32.0f;
	vScreenScale[1] = (float) nHeight / 32.0f;
	pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1 );
}


static void DrawFlashlight( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, 
						   IShaderShadow* pShaderShadow, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	if( pShaderShadow )
	{
		pShaderShadow->EnableDepthWrites( false );

		pShader->EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );	// Write over the eyes that were already there 

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );			// Spot
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );			// Base
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );			// Normalizing cubemap
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );			// Iris

		// Set stream format (note that this shader supports compression)
		int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		// Be sure not to write to dest alpha
		pShaderShadow->EnableAlphaWrites( false );

		int nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_STATIC_VERTEX_SHADER( eyes_flashlight_vs20 );
			SET_STATIC_VERTEX_SHADER( eyes_flashlight_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( eyes_flashlight_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER( eyes_flashlight_ps20b );
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

			DECLARE_STATIC_VERTEX_SHADER( eyes_flashlight_vs30 );
			SET_STATIC_VERTEX_SHADER( eyes_flashlight_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( eyes_flashlight_ps30 );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER( eyes_flashlight_ps30 );
		}

		// On DX9, get the gamma read and write correct
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );			// Spot
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// Base
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, true );			// Iris
		pShaderShadow->EnableSRGBWrite( true );

		if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );			// Shadow depth map
			pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );			// Shadow noise rotation map
		}
		
		pShader->FogToBlack();
	}
	else
	{	
		VMatrix worldToTexture;
		ITexture *pFlashlightDepthTexture;
		FlashlightState_t flashlightState = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );

		pShader->BindTexture( SHADER_SAMPLER0, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame );
		pShader->BindTexture( SHADER_SAMPLER1, info.m_nBaseTexture, info.m_nFrame );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER2, TEXTURE_NORMALIZATION_CUBEMAP );
		pShader->BindTexture( SHADER_SAMPLER3, info.m_nIris, info.m_nIrisFrame );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( eyes_flashlight_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER( eyes_flashlight_vs20 );
		}
		else
		{
			pShader->SetHWMorphVertexShaderState( VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, VERTEX_SHADER_SHADER_SPECIFIC_CONST_11, SHADER_VERTEXTEXTURE_SAMPLER0 );

			DECLARE_DYNAMIC_VERTEX_SHADER( eyes_flashlight_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( MORPHING, pShaderAPI->IsHWMorphingEnabled() );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER( eyes_flashlight_vs30 );
		}

//			float vPSConst[4] = {params[info.m_nDilation]->GetFloatValue(), 0.0f, 0.0f, 0.0f};
//			pShaderAPI->SetPixelShaderConstant( 0, vPSConst, 1 );

		SetFlashLightColorFromState( flashlightState, pShaderAPI );

		if( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && flashlightState.m_bEnableShadows )
		{
			pShader->BindTexture( SHADER_SAMPLER4, pFlashlightDepthTexture, 0 );
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D );
		}

		pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );
		vEyePos_SpecExponent[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( eyes_flashlight_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, flashlightState.m_bEnableShadows && ( pFlashlightDepthTexture != NULL ) );
			SET_DYNAMIC_PIXEL_SHADER( eyes_flashlight_ps20b );

			SetDepthFlashlightParams( pShader, pShaderAPI, worldToTexture, flashlightState );
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( eyes_flashlight_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, flashlightState.m_bEnableShadows && ( pFlashlightDepthTexture != NULL ) );
			SET_DYNAMIC_PIXEL_SHADER( eyes_flashlight_ps30 );

			SetDepthFlashlightParams( pShader, pShaderAPI, worldToTexture, flashlightState );
		}

		// This uses from VERTEX_SHADER_SHADER_SPECIFIC_CONST_0 to VERTEX_SHADER_SHADER_SPECIFIC_CONST_5
		pShader->SetFlashlightVertexShaderConstants( false, -1, false, -1, false );

		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, info.m_nEyeOrigin );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, info.m_nEyeUp );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, info.m_nIrisU );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_9, info.m_nIrisV );
	}
	pShader->Draw();
}

static void DrawUsingVertexShader( CBaseVSShader *pShader, IMaterialVar** params, 
								  IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
								  Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );	// Base
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Iris
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Glint

		// Set stream format (note that this shader supports compression)
		int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		pShaderShadow->EnableAlphaWrites( true ); //we end up hijacking destination alpha for opaques most of the time.
		
		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

			DECLARE_STATIC_VERTEX_SHADER( eyes_vs20 );
			SET_STATIC_VERTEX_SHADER_COMBO( HALFLAMBERT, IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
			SET_STATIC_VERTEX_SHADER_COMBO( INTRO, params[info.m_nIntro]->GetIntValue() ? 1 : 0 );
			SET_STATIC_VERTEX_SHADER_COMBO( USE_STATIC_CONTROL_FLOW, bUseStaticControlFlow );
			SET_STATIC_VERTEX_SHADER( eyes_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( eyes_ps20b );
			SET_STATIC_PIXEL_SHADER( eyes_ps20b );
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

			DECLARE_STATIC_VERTEX_SHADER( eyes_vs30 );
			SET_STATIC_VERTEX_SHADER_COMBO( HALFLAMBERT, IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
			SET_STATIC_VERTEX_SHADER_COMBO( INTRO, params[info.m_nIntro]->GetIntValue() ? 1 : 0 );
			SET_STATIC_VERTEX_SHADER( eyes_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( eyes_ps30 );
			SET_STATIC_PIXEL_SHADER( eyes_ps30 );
		}

		// On DX9, get the gamma read and write correct
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );			// Base
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// White
		pShaderShadow->EnableSRGBWrite( true );

		pShader->FogToFogColor();
	}
	DYNAMIC_STATE
	{
		pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nFrame );
		pShader->BindTexture( SHADER_SAMPLER1, info.m_nIris, info.m_nIrisFrame );
		pShader->BindTexture( SHADER_SAMPLER2, info.m_nGlint );
		pShader->SetAmbientCubeDynamicStateVertexShader();
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nEyeOrigin );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, info.m_nEyeUp );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, info.m_nIrisU );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_3, info.m_nIrisV );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, info.m_nGlintU );
		pShader->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_5, info.m_nGlintV );

		LightState_t lightState;
		pShaderAPI->GetDX9LightState( &lightState );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

			DECLARE_DYNAMIC_VERTEX_SHADER( eyes_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DYNAMIC_LIGHT, lightState.HasDynamicLight() );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( STATIC_LIGHT,  lightState.m_bStaticLightVertex  ? 1 : 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( NUM_LIGHTS, bUseStaticControlFlow ? 0 : lightState.m_nNumLights );
			SET_DYNAMIC_VERTEX_SHADER( eyes_vs20 );
		}
		else
		{
			pShader->SetHWMorphVertexShaderState( VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, SHADER_VERTEXTEXTURE_SAMPLER0 );

			DECLARE_DYNAMIC_VERTEX_SHADER( eyes_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DYNAMIC_LIGHT, lightState.HasDynamicLight() );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( STATIC_LIGHT,  lightState.m_bStaticLightVertex  ? 1 : 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( MORPHING, pShaderAPI->IsHWMorphingEnabled() );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER( eyes_vs30 );
		}

		// Get luminance of ambient cube and saturate it
		float fGlintDamping = max(0.0f, min( pShaderAPI->GetAmbientLightCubeLuminance(), 1.0f ) );
		const float fDimGlint = 0.01f;

		// Remap so that glint damping smooth steps to zero for low luminances
		if ( fGlintDamping > fDimGlint )
			fGlintDamping = 1.0f;
		else
			fGlintDamping *= SimpleSplineRemapVal( fGlintDamping, 0.0f, fDimGlint, 0.0f, 1.0f );

		// Special constant for DX9 eyes: { Dilation, ambient, x, x };
		float vPSConst[4] = {params[info.m_nDilation]->GetFloatValue(), fGlintDamping, 0.0f, 0.0f};
		pShaderAPI->SetPixelShaderConstant( 0, vPSConst, 1 );

		pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );
		vEyePos_SpecExponent[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( eyes_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha() );
			SET_DYNAMIC_PIXEL_SHADER( eyes_ps20b );
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( eyes_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha() );
			SET_DYNAMIC_PIXEL_SHADER( eyes_ps30 );
		}

		Assert( info.m_nIntro != -1 );
		if( params[info.m_nIntro]->GetIntValue() )
		{
			float curTime = params[info.m_nWarpParam]->GetFloatValue();
			float timeVec[4] = { 0.0f, 0.0f, 0.0f, curTime };
			Assert( params[info.m_nEntityOrigin]->IsDefined() );
			params[info.m_nEntityOrigin]->GetVecValue( timeVec, 3 );
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, timeVec, 1 );
		}
	}
	pShader->Draw();
}

static void DrawEyes_DX8_DX9_Internal( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow, bool bHasFlashlight, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	if( !bHasFlashlight )
	{
		DrawUsingVertexShader( pShader, params, pShaderAPI, pShaderShadow, info, vertexCompression );
	}
	else
	{
		DrawFlashlight( pShader, params, pShaderAPI, pShaderShadow, info, vertexCompression );
	}
}

void DrawEyes_DX8_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
					  IShaderShadow* pShaderShadow, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	}
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	DrawEyes_DX8_DX9_Internal( pShader, params, pShaderAPI, pShaderShadow, bHasFlashlight, info, vertexCompression );
}


