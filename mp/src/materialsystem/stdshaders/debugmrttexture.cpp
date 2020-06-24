//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "debugmrttexture_ps20b.inc"
#include "debugmrttexture_vs20.inc"

BEGIN_VS_SHADER_FLAGS( DebugMRTTexture, "Help for DebugMRTTexture", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( MRTINDEX, SHADER_PARAM_TYPE_INTEGER, "", "" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
//		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
//		{
//			return "UnlitGeneric_DX8";
//		}
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			DECLARE_STATIC_VERTEX_SHADER( debugmrttexture_vs20 );
			SET_STATIC_VERTEX_SHADER( debugmrttexture_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( debugmrttexture_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( MRTINDEX,  params[MRTINDEX]->GetIntValue() );
			SET_STATIC_PIXEL_SHADER( debugmrttexture_ps20b );

			int numTexCoords = 2;
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, numTexCoords, 0, 0 );
		}
		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );

			DECLARE_DYNAMIC_VERTEX_SHADER( debugmrttexture_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( debugmrttexture_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER( debugmrttexture_ps20b );
			SET_DYNAMIC_PIXEL_SHADER( debugmrttexture_ps20b );
		}
		Draw();
	}
END_SHADER

