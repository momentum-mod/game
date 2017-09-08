#include "BaseVSShader.h"

BEGIN_VS_SHADER(COLORPICKER_BLEND, "")
    BEGIN_SHADER_PARAMS
        SHADER_PARAM(COLOR_00, SHADER_PARAM_TYPE_VEC3, "", "")
        SHADER_PARAM(COLOR_10, SHADER_PARAM_TYPE_VEC3, "", "")
        SHADER_PARAM(COLOR_11, SHADER_PARAM_TYPE_VEC3, "", "")
        SHADER_PARAM(COLOR_01, SHADER_PARAM_TYPE_VEC3, "", "")
    END_SHADER_PARAMS

    SHADER_INIT
    {
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
            pShaderShadow->EnableDepthTest(false);
            pShaderShadow->EnableSRGBWrite(false);

            pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, nullptr, 0);

            pShaderShadow->SetVertexShader("colorpicker_blend_vs20", 0);
            pShaderShadow->SetPixelShader("colorpicker_blend_ps20", 0);
        }
        DYNAMIC_STATE
        {
            pShaderAPI->SetDefaultState();

            pShaderAPI->SetVertexShaderIndex();
            pShaderAPI->SetPixelShaderIndex(0);

            float data[4] = { 0, 0, 0, 0 };
            for (int i = 0; i < 4; i++)
            {
                params[COLOR_00 + i]->GetVecValue(data, 3);
                pShaderAPI->SetPixelShaderConstant(i, data);
            }
        }

        Draw();
    }
END_SHADER