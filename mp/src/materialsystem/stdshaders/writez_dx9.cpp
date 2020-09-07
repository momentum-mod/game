//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "writez_vs20.inc"
#include "white_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( WriteZ, WriteZ_DX9 )

BEGIN_VS_SHADER_FLAGS( WriteZ_DX9, "Help for WriteZ", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( false );

			DECLARE_STATIC_VERTEX_SHADER( writez_vs20 );
			SET_STATIC_VERTEX_SHADER( writez_vs20 );

			// No pixel shader on Direct3D, doubles fill rate.
			if ( g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() )
			{
				DECLARE_STATIC_PIXEL_SHADER( white_ps20b );
				SET_STATIC_PIXEL_SHADER( white_ps20b );
			}

			// Set stream format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			int nTexCoordCount = 1;
			int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );
		}
		DYNAMIC_STATE
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( writez_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER( writez_vs20 );

			// No pixel shader on Direct3D, doubles fill rate
			if ( g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( white_ps20b );
				SET_DYNAMIC_PIXEL_SHADER( white_ps20b );
			}
		}
		Draw();
	}
END_SHADER

