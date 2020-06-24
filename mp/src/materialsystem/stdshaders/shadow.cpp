//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "shadow_ps20b.inc"
#include "shadow_vs20.inc"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


BEGIN_VS_SHADER_FLAGS( Shadow, "Help for Shadow", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BLOBBYSHADOWS, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( DEPTHTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		/*
		The alpha blending state either must be:

		Src Color * Dst Color + Dst Color * 0	
		(src color = C*A + 1-A)

		or

		// Can't be this, doesn't work with fog
		Src Color * Dst Color + Dst Color * (1-Src Alpha)	
		(src color = C * A, Src Alpha = A)
		*/
		if ( !params[BLOBBYSHADOWS]->IsDefined() )
		{
			params[BLOBBYSHADOWS]->SetIntValue( 0 );
		}
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			bool bBlobbyShadows = ( params[BLOBBYSHADOWS]->GetIntValue() != 0 );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			// NOTE: This is deliberately this way round (instead of "DST_COLOR, ZERO"),
			//       since these two permutations produce *different* values on 360!!
			//       This was causing undue darkening of the framebuffer by these
			//       shadows, which was highly noticeable in very dark areas:
			EnableAlphaBlending( SHADER_BLEND_ZERO, SHADER_BLEND_SRC_COLOR );

			unsigned int flags = VERTEX_POSITION | VERTEX_COLOR;
			int numTexCoords = 2;
			int texCoordDims[2] = { 3, 3 };
			pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, texCoordDims, 0 );

			DECLARE_STATIC_VERTEX_SHADER( shadow_vs20 );
			SET_STATIC_VERTEX_SHADER( shadow_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( shadow_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( BLOBBY_SHADOWS, bBlobbyShadows );
			SET_STATIC_PIXEL_SHADER( shadow_ps20b );

			pShaderShadow->EnableSRGBWrite( true );

			// We need to fog to *white* regardless of overbrighting...
			FogToWhite();
		}
		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );

			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );
			SetPixelShaderConstantGammaToLinear( 1, COLOR );	// shadow color

			// Get texture dimensions...
			int nWidth = 16;
			int nHeight = 16;
			ITexture *pTexture = params[BASETEXTURE]->GetTextureValue();
			if (pTexture)
			{
				nWidth = pTexture->GetActualWidth();
				nHeight = pTexture->GetActualHeight();
			}

			Vector4D vecJitter( 1.0 / nWidth, 1.0 / nHeight, 0.0, 0.0 );
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, vecJitter.Base() );

			vecJitter.y *= -1.0f;
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_3, vecJitter.Base() );

			DECLARE_DYNAMIC_VERTEX_SHADER( shadow_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( shadow_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER( shadow_ps20b );
			SET_DYNAMIC_PIXEL_SHADER( shadow_ps20b );

			float eyePos[4];
			pShaderAPI->GetWorldSpaceCameraPosition( eyePos );
			pShaderAPI->SetPixelShaderConstant( 2, eyePos, 1 );
			pShaderAPI->SetPixelShaderFogParams( 3 );
		}
		Draw( );
	}
END_SHADER
