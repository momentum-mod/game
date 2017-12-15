#include "BaseVSShader.h"
#include "tier0/memdbgon.h"

BEGIN_SHADER(fringe, "")

BEGIN_SHADER_PARAMS
SHADER_PARAM(SAMPLE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer Sampler")
SHADER_PARAM(MASK, SHADER_PARAM_TYPE_TEXTURE, "_rt_MaskGameUI", "")
END_SHADER_PARAMS

SHADER_INIT
{
    LoadTexture(MASK);
    LoadTexture(SAMPLE);
}

SHADER_FALLBACK
{
    return nullptr;
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
        pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

        // Render targets are pegged as sRGB on POSIX, so just force these reads and writes
        bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
        pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, bForceSRGBReadAndWrite);
        pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, bForceSRGBReadAndWrite);
        pShaderShadow->EnableSRGBWrite(bForceSRGBReadAndWrite);

        pShaderShadow->SetVertexShader("fringe_vs30", 0);
        pShaderShadow->SetPixelShader("fringe_ps30", 0);
    }

    DYNAMIC_STATE
    {
        pShaderAPI->SetDefaultState();
        BindTexture(SHADER_SAMPLER0, SAMPLE);
        BindTexture(SHADER_SAMPLER1, MASK);
    
    }
    Draw();
}

END_SHADER