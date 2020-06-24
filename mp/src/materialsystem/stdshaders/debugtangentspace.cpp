//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "shaderlib/cshader.h"

#include "debugtangentspace_vs20.inc"
#include "unlitgeneric_notexture_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_SHADER( DebugTangentSpace, "Help for DebugTangentSpace" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/basetexture", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( FRAME, SHADER_PARAM_TYPE_INTEGER, "0", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( BASETEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		if (g_pHardwareConfig->SupportsVertexAndPixelShaders())
		{
			SHADOW_STATE
			{
				// Set stream format (note that this shader supports compression)
				unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
				int nTexCoordCount = 0;
				int userDataSize = 4;
				pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

				DECLARE_STATIC_VERTEX_SHADER( debugtangentspace_vs20 );
				SET_STATIC_VERTEX_SHADER( debugtangentspace_vs20 );

				DECLARE_STATIC_PIXEL_SHADER( unlitgeneric_notexture_ps20b );
				SET_STATIC_PIXEL_SHADER( unlitgeneric_notexture_ps20b );
			}
			DYNAMIC_STATE
			{		
				DECLARE_DYNAMIC_VERTEX_SHADER( debugtangentspace_vs20 );
				SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
				SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
				SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
				SET_DYNAMIC_VERTEX_SHADER( debugtangentspace_vs20 );

				DECLARE_DYNAMIC_PIXEL_SHADER( unlitgeneric_notexture_ps20b );
				SET_DYNAMIC_PIXEL_SHADER( unlitgeneric_notexture_ps20b );
			}
			Draw();
		}
	}
END_SHADER

