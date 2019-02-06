#include "BaseVSShader.h"

#include "ul_fill_vs30.inc"
#include "ul_fill_ps30.inc"

BEGIN_VS_SHADER(UL_FILL, "Fill shader for Ultralight UI")

    BEGIN_SHADER_PARAMS
        SHADER_PARAM(STATE, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "State vector")

        SHADER_PARAM(TRANSFORM, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Transform matrix")

        SHADER_PARAM(SCALAR0, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Scalar vector 0")
        SHADER_PARAM(SCALAR1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Scalar vector 1")
        
        SHADER_PARAM(VECTOR0, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 0")
        SHADER_PARAM(VECTOR1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 1")
        SHADER_PARAM(VECTOR2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 2")
        SHADER_PARAM(VECTOR3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 3")
        SHADER_PARAM(VECTOR4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 4")
        SHADER_PARAM(VECTOR5, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 5")
        SHADER_PARAM(VECTOR6, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 6")
        SHADER_PARAM(VECTOR7, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Vector 7")

        SHADER_PARAM(CLIP0, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 0")
        SHADER_PARAM(CLIP1, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 1")
        SHADER_PARAM(CLIP2, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 2")
        SHADER_PARAM(CLIP3, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 3")
        SHADER_PARAM(CLIP4, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 4")
        SHADER_PARAM(CLIP5, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 5")
        SHADER_PARAM(CLIP6, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 6")
        SHADER_PARAM(CLIP7, SHADER_PARAM_TYPE_MATRIX, "[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]", "Clip matrix 7")
        SHADER_PARAM(CLIP_SIZE, SHADER_PARAM_TYPE_INTEGER, "0", "Clip size")

        SHADER_PARAM(TEXTURE0, SHADER_PARAM_TYPE_TEXTURE, "", "Texture 0")
        SHADER_PARAM(TEXTURE1, SHADER_PARAM_TYPE_TEXTURE, "", "Texture 1")
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

    inline IMaterialVar *GetULVector(IMaterialVar** params, int index)
    {
        switch (index)
        {
        case 0: return params[VECTOR0];
        case 1: return params[VECTOR1];
        case 2: return params[VECTOR2];
        case 3: return params[VECTOR3];
        case 4: return params[VECTOR4];
        case 5: return params[VECTOR5];
        case 6: return params[VECTOR6];
        case 7: return params[VECTOR7];
        default:
            AssertMsg(false, "Invalid index");
            return nullptr;
        }
    }

    SHADER_INIT
    {
        if (params[TEXTURE0]->IsDefined())
        {
            LoadTexture(TEXTURE0);
        }
        if (params[TEXTURE1]->IsDefined())
        {
            LoadTexture(TEXTURE1);
        }
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
            pShaderShadow->EnableBlending(true);

            int dimensions[8] = {2, 2, 4, 4, 4, 4, 4, 4};

            pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2) |
                                                        VERTEX_TEXCOORD_SIZE(1, 2) | VERTEX_TEXCOORD_SIZE(2, 4) |
                                                        VERTEX_TEXCOORD_SIZE(3, 4) | VERTEX_TEXCOORD_SIZE(4, 4) |
                                                        VERTEX_TEXCOORD_SIZE(5, 4) | VERTEX_TEXCOORD_SIZE(6, 4) |
                                                        VERTEX_TEXCOORD_SIZE(7, 4) | VERTEX_USERDATA_SIZE(4),
                8, dimensions, 4);

            pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
            pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);

            DECLARE_STATIC_VERTEX_SHADER(ul_fill_vs30);
            SET_STATIC_VERTEX_SHADER(ul_fill_vs30);
            DECLARE_STATIC_PIXEL_SHADER(ul_fill_ps30);
            SET_STATIC_PIXEL_SHADER(ul_fill_ps30);
        }
        DYNAMIC_STATE
        {
            pShaderAPI->SetDefaultState();

            DECLARE_DYNAMIC_VERTEX_SHADER(ul_fill_vs30);
            SET_DYNAMIC_VERTEX_SHADER(ul_fill_vs30);
            DECLARE_DYNAMIC_PIXEL_SHADER(ul_fill_ps30);
            SET_DYNAMIC_PIXEL_SHADER(ul_fill_ps30);

            // Vertex shader constants
            pShaderAPI->SetVertexShaderConstant(217, params[STATE]->GetVecValue());
            pShaderAPI->SetVertexShaderConstant(218, params[TRANSFORM]->GetMatrixValue().Base(), 4);

            // Pixel shader constants
            pShaderAPI->SetPixelShaderConstant(0, params[STATE]->GetVecValue());
            pShaderAPI->SetPixelShaderConstant(1, params[SCALAR0]->GetVecValue());
            pShaderAPI->SetPixelShaderConstant(2, params[SCALAR1]->GetVecValue());
            for (int i = 0; i < 8; i++)
            {
                pShaderAPI->SetPixelShaderConstant(3 + i, GetULVector(params, i)->GetVecValue());
            }
            pShaderAPI->SetPixelShaderConstant(11, params[TRANSFORM]->GetMatrixValue().Base(), 4);

            int clipsize[4];
            clipsize[0] = params[CLIP_SIZE]->GetIntValue();
            clipsize[1] = 0; // Unused
            clipsize[2] = 0; // Unused
            clipsize[3] = 0; // Unused
            pShaderAPI->SetIntegerPixelShaderConstant(0, clipsize, 1);

            for (int i = 0; i < clipsize[0]; i++)
            {
                const VMatrix &clipmat = GetULClipMatrix(params, i)->GetMatrixValue();
                pShaderAPI->SetPixelShaderConstant(15 + i * 4, clipmat.Base(), 4);
            }

            if (params[TEXTURE0]->IsDefined() && params[TEXTURE0]->GetTextureValue())
            {
                BindTexture(SHADER_SAMPLER0, TEXTURE0, -1);
            }
            if (params[TEXTURE1]->IsDefined() && params[TEXTURE1]->GetTextureValue())
            {
                BindTexture(SHADER_SAMPLER1, TEXTURE1, -1);
            }
        }

        Draw();
    }

END_SHADER