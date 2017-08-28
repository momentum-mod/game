//========= Copyright &copy; 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"
#include "gauss_blur_3x3_vs30.inc"
#include "gauss_blur_3x3_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS( GAUSSBLUR3X3, "Help for GAUSSBLUR3X3", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Render Target" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		if( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE );
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableAlphaWrites( true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );
	
			// Render targets are pegged as sRGB on POSIX, so just force these reads and writes
			bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, bForceSRGBReadAndWrite );
			pShaderShadow->EnableSRGBWrite( bForceSRGBReadAndWrite );
			
			DECLARE_STATIC_VERTEX_SHADER( gauss_blur_3x3_vs30 );
			SET_STATIC_VERTEX_SHADER( gauss_blur_3x3_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( gauss_blur_3x3_ps30 );
			SET_STATIC_PIXEL_SHADER( gauss_blur_3x3_ps30 );
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, -1 );

			ITexture *src_texture = params[BASETEXTURE]->GetTextureValue();

			int width = src_texture->GetActualWidth();
			int height = src_texture->GetActualHeight();

			float dX = 1.0f / width;
			float dY = 1.0f / height;

			float fTexelSize[2] = { dX, dY };

			pShaderAPI->SetPixelShaderConstant( 0, fTexelSize );

			DECLARE_DYNAMIC_VERTEX_SHADER( gauss_blur_3x3_vs30 );
			SET_DYNAMIC_VERTEX_SHADER( gauss_blur_3x3_vs30 );

			DECLARE_DYNAMIC_PIXEL_SHADER( gauss_blur_3x3_ps30 );
			SET_DYNAMIC_PIXEL_SHADER( gauss_blur_3x3_ps30 );
		}
		Draw();
	}
END_SHADER
