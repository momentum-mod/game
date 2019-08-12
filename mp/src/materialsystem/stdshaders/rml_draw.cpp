//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "rml_draw_vs20.inc"
#include "rml_draw_ps20.inc"
#include "rml_draw_ps20b.inc"

BEGIN_SHADER( rml_draw, "Draws RmlUi textures" )
    BEGIN_SHADER_PARAMS
        SHADER_PARAM( TEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
        SHADER_PARAM( OFFSET_X, SHADER_PARAM_TYPE_FLOAT, "0", "" )
        SHADER_PARAM( OFFSET_Y, SHADER_PARAM_TYPE_FLOAT, "0", "" )
    END_SHADER_PARAMS

    SHADER_INIT
    {
        LoadTexture(TEXTURE);
    }

    SHADER_FALLBACK
    {
        // Requires DX9 + above
        if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
        {
            Assert( false );
            return "Wireframe";
        }
        return 0;
    }

    SHADER_DRAW
    {
        SHADOW_STATE
        {
            pShaderShadow->SetDefaultState();
            pShaderShadow->EnableDepthWrites(false);
            pShaderShadow->EnableDepthTest(false);
            pShaderShadow->EnableBlending(true);
            pShaderShadow->BlendFunc(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
            pShaderShadow->BlendOp(SHADER_BLEND_OP_ADD);
            pShaderShadow->EnableCulling(false);

            if (params[TEXTURE]->IsDefined())
            {
                pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
            }

            int fmt = VERTEX_POSITION | VERTEX_COLOR;
            pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );


            // Pre-cache shaders
            DECLARE_STATIC_VERTEX_SHADER(rml_draw_vs20);
            SET_STATIC_VERTEX_SHADER(rml_draw_vs20);

            if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
            {
                DECLARE_STATIC_PIXEL_SHADER(rml_draw_ps20b);
                SET_STATIC_PIXEL_SHADER(rml_draw_ps20b);
            }
            else
            {
                DECLARE_STATIC_PIXEL_SHADER(rml_draw_ps20);
                SET_STATIC_PIXEL_SHADER(rml_draw_ps20);
            }
        }

        DYNAMIC_STATE
        {
            pShaderAPI->SetDefaultState();

            float translation[4];
            translation[0] = params[OFFSET_X]->GetFloatValue();
            translation[1] = params[OFFSET_Y]->GetFloatValue();
            translation[2] = 0.0f;
            translation[3] = 0.0f;
            pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, translation);
            
            if (params[TEXTURE]->IsDefined())
            {
                BindTexture(SHADER_SAMPLER0, params[TEXTURE]->GetTextureValue());
            }

            DECLARE_DYNAMIC_VERTEX_SHADER(rml_draw_vs20);
            SET_DYNAMIC_VERTEX_SHADER(rml_draw_vs20);

            if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
            {
                DECLARE_DYNAMIC_PIXEL_SHADER(rml_draw_ps20b);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(HAS_TEXTURE, params[TEXTURE]->IsDefined());
                SET_DYNAMIC_PIXEL_SHADER(rml_draw_ps20b);
            }
            else
            {
                DECLARE_DYNAMIC_PIXEL_SHADER(rml_draw_ps20);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(HAS_TEXTURE, params[TEXTURE]->IsDefined());
                SET_DYNAMIC_PIXEL_SHADER(rml_draw_ps20);
            }
        }
        Draw();
    }
END_SHADER
