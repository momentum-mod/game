//===== Copyright 2011, GearDev Software, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//=====================================================================//

#include "BaseVSShader.h"

#include "fxaa_vs20.inc"
#include "fxaa_ps20.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS( FXAA, "Help for FXAA", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( FRAMEBUFFER, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
		SHADER_PARAM( QUALITY, SHADER_PARAM_TYPE_INTEGER, "4", "" )
	END_SHADER_PARAMS
	SHADER_INIT_PARAMS()
	{
		if( !params[QUALITY]->IsDefined() || params[QUALITY]->GetIntValue() < 0 || params[QUALITY]->GetIntValue() > 4 )
		{
			params[QUALITY]->SetIntValue( 4 ); // Default to Very High, it's not exactly expensive anyway.
		}
	}

	SHADER_INIT
	{
		if( params[FRAMEBUFFER]->IsDefined() )
		{
			LoadTexture( FRAMEBUFFER );
		}
	}
	
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 ) // || !g_pHardwareConfig->SupportsShaderModel_3_0() )
		{
			Assert( 0 );
			return "Wireframe";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			pShaderShadow->EnableColorWrites( true );
			pShaderShadow->EnableAlphaWrites( true );

			DECLARE_STATIC_VERTEX_SHADER( fxaa_vs20 );
			SET_STATIC_VERTEX_SHADER( fxaa_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( fxaa_ps20 );
			SET_STATIC_PIXEL_SHADER( fxaa_ps20 );
		}

		DYNAMIC_STATE
		{
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );

			float g_const0[4] = { 1.0f/(float)nWidth, 1.0f/(float)nHeight, 0.0f, 0.0f };
			pShaderAPI->SetPixelShaderConstant( 0, g_const0 );

			BindTexture( SHADER_SAMPLER0, FRAMEBUFFER );

			DECLARE_DYNAMIC_VERTEX_SHADER( fxaa_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( fxaa_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER( fxaa_ps20 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( QUALITY, params[QUALITY]->GetIntValue() );
			SET_DYNAMIC_PIXEL_SHADER( fxaa_ps20 );
		}
		Draw();
	}
END_SHADER
