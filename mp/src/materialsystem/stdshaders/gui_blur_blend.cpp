#include "BaseVSShader.h"
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER(gui_blur_blend, "")

BEGIN_SHADER_PARAMS
SHADER_PARAM(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullscreenPP", "")
SHADER_PARAM(BLUR, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullscreenPP", "The Blur render target")
SHADER_PARAM(MASK_GAMEUI, SHADER_PARAM_TYPE_TEXTURE, "_rt_MaskGameUI", "")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
    
}

SHADER_FALLBACK
{
    return nullptr;
}

SHADER_INIT
{
    LoadTexture(BASETEXTURE);
    LoadTexture(BLUR);
    LoadTexture(MASK_GAMEUI);
}

SHADER_DRAW
{
    SHADOW_STATE
    {
        pShaderShadow->SetDefaultState();
        pShaderShadow->EnableDepthWrites(false);
        pShaderShadow->EnableAlphaWrites(true);

        pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
        pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
        pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);

        pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

        // Render targets are pegged as sRGB on POSIX, so just force these reads and writes
        bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
        pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, bForceSRGBReadAndWrite);
        pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, bForceSRGBReadAndWrite);
        pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, bForceSRGBReadAndWrite);
        pShaderShadow->EnableSRGBWrite(bForceSRGBReadAndWrite);
    
        pShaderShadow->SetVertexShader("gui_blur_blend_vs30", 0);
        pShaderShadow->SetPixelShader("gui_blur_blend_ps30", 0);
    }

    DYNAMIC_STATE
    {
        pShaderAPI->SetDefaultState();
        BindTexture(SHADER_SAMPLER0, BASETEXTURE);
        BindTexture(SHADER_SAMPLER1, BLUR);
        BindTexture(SHADER_SAMPLER2, MASK_GAMEUI);
    
    }
    Draw();
}
END_SHADER