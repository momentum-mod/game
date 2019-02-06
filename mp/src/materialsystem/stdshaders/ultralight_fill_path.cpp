#include "BaseVSShader.h"

#include "ul_fill_path_vs30.inc"
#include "ul_fill_path_ps30.inc"

BEGIN_VS_SHADER(UL_FILL_PATH, "Fill path shader for Ultralight UI")

    BEGIN_SHADER_PARAMS
        SHADER_PARAM(STATE, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "State vector")

        SHADER_PARAM(TRANSFORM, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Transform matrix")

        SHADER_PARAM(CLIP0, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 0")
        SHADER_PARAM(CLIP1, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 1")
        SHADER_PARAM(CLIP2, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 2")
        SHADER_PARAM(CLIP3, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 3")
        SHADER_PARAM(CLIP4, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 4")
        SHADER_PARAM(CLIP5, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 5")
        SHADER_PARAM(CLIP6, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 6")
        SHADER_PARAM(CLIP7, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 7")
        SHADER_PARAM(CLIP_SIZE, SHADER_PARAM_TYPE_INTEGER, "0", "Clip size")
    END_SHADER_PARAMS

    // YUCK!
    // Can't figure out a way to get array-ish shader params so this 
    // is best i can do :(
    inline IMaterialVar *GetULClipMatrix(IMaterialVar** params, int index)
    {
        switch (index)
        {
        case 0: return params[CLIP0];
        case 1: return params[CLIP1];
        case 2: return params[CLIP2];
        case 3: return params[CLIP3];
        case 4: return params[CLIP4];
        case 5: return params[CLIP5];
        case 6: return params[CLIP6];
        case 7: return params[CLIP7];
        default:
            AssertMsg(false, "Invalid index");
            return nullptr;
        }
    }

    SHADER_INIT
    {
    }

    SHADER_FALLBACK { return nullptr; }

    SHADER_DRAW
    {
        SHADOW_STATE
        {
            pShaderShadow->SetDefaultState();
            pShaderShadow->EnableDepthWrites(false);
            pShaderShadow->EnableDepthTest(false);
            pShaderShadow->EnableSRGBWrite(false);

            pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2), 1,
                                                    nullptr, 0);

            DECLARE_STATIC_VERTEX_SHADER(ul_fill_path_vs30);
            SET_STATIC_VERTEX_SHADER(ul_fill_path_vs30);
            DECLARE_STATIC_PIXEL_SHADER(ul_fill_path_ps30);
            SET_STATIC_PIXEL_SHADER(ul_fill_path_ps30);
        }
        DYNAMIC_STATE
        {
            pShaderAPI->SetDefaultState();

            DECLARE_DYNAMIC_VERTEX_SHADER(ul_fill_path_ps30);
            SET_DYNAMIC_VERTEX_SHADER(ul_fill_path_ps30);
            DECLARE_DYNAMIC_PIXEL_SHADER(ul_fill_path_ps30);
            SET_DYNAMIC_PIXEL_SHADER(ul_fill_path_ps30);

            pShaderAPI->SetVertexShaderConstant(217, params[STATE]->GetVecValue());
            pShaderAPI->SetVertexShaderConstant(218, params[TRANSFORM]->GetMatrixValue().Base(), 4);

            int clipsize[4];
            clipsize[0] = params[CLIP_SIZE]->GetIntValue();
            clipsize[1] = 0; // Unused
            clipsize[2] = 0; // Unused
            clipsize[3] = 0; // Unused
            pShaderAPI->SetIntegerPixelShaderConstant(0, clipsize, 1);

            for (int i = 0; i < clipsize[0]; i++)
            {
                const VMatrix &clipmat = GetULClipMatrix(params, i)->GetMatrixValue();
                pShaderAPI->SetPixelShaderConstant(i * 4, clipmat.Base(), 4);
            }
        }

        Draw();
    }

END_SHADER