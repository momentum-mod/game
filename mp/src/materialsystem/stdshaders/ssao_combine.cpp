//========= Copyright &copy; 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"
#include "ssao_combine_ps30.inc"
#include "ssao_combine_vs30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS(SSAOCOMBINE, "Help for SSAO Combine", SHADER_NOT_EDITABLE)
BEGIN_SHADER_PARAMS
SHADER_PARAM(SSAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_SSAO", "SSAO")
SHADER_PARAM(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer")
END_SHADER_PARAMS

SHADER_INIT_PARAMS() { SET_FLAGS2(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE); }

SHADER_FALLBACK { return 0; }

SHADER_INIT
{
    if (params[SSAOTEXTURE]->IsDefined())
    {
        LoadTexture(SSAOTEXTURE);
    }

    if (params[BASETEXTURE]->IsDefined())
    {
        LoadTexture(BASETEXTURE);
    }
}

SHADER_DRAW
{
    SHADOW_STATE
    {
        pShaderShadow->EnableDepthWrites(false);

        pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

        DECLARE_STATIC_VERTEX_SHADER(ssao_combine_vs30);
        SET_STATIC_VERTEX_SHADER(ssao_combine_vs30);

        pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
        pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);

        DECLARE_STATIC_PIXEL_SHADER(ssao_combine_ps30);
        SET_STATIC_PIXEL_SHADER(ssao_combine_ps30);
    }

    DYNAMIC_STATE
    {
        BindTexture(SHADER_SAMPLER0, SSAOTEXTURE, -1);
        BindTexture(SHADER_SAMPLER1, BASETEXTURE, -1);

        DECLARE_DYNAMIC_VERTEX_SHADER(ssao_combine_vs30);
        SET_DYNAMIC_VERTEX_SHADER(ssao_combine_vs30);

        DECLARE_DYNAMIC_PIXEL_SHADER(ssao_combine_ps30);
        SET_DYNAMIC_PIXEL_SHADER(ssao_combine_ps30);
    }
    Draw();
}
END_SHADER