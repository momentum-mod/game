//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "shaderlib/BaseShader.h"
#include "shaderlib/ShaderDLL.h"
#include "tier0/dbg.h"
#include "shaderDLL_Global.h"
#include "../IShaderSystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/materialsystem_config.h"
#include "shaderlib/cshader.h"
#include "shaderlib/commandbuilder.h"
#include "renderparm.h"
#include "mathlib/vmatrix.h"
#include "tier1/strtools.h"
#include "convar.h"
#include "tier0/vprof.h"

// NOTE: This must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
const char *CBaseShader::s_pTextureGroupName = NULL;
IMaterialVar **CBaseShader::s_ppParams;
IShaderShadow *CBaseShader::s_pShaderShadow;
IShaderDynamicAPI *CBaseShader::s_pShaderAPI;
IShaderInit *CBaseShader::s_pShaderInit;
int CBaseShader::s_nModulationFlags;

bool g_shaderConfigDumpEnable = false; //true;		//DO NOT CHECK IN ENABLED FIXME
static ConVar mat_fullbright( "mat_fullbright","0", FCVAR_CHEAT );
	
//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
CBaseShader::CBaseShader()
{
	GetShaderDLL()->InsertShader( this );
}


//-----------------------------------------------------------------------------
// Shader parameter info
//-----------------------------------------------------------------------------
// Look in BaseShader.h for the enumeration for these.
// Update there if you update here.
static ShaderParamInfo_t s_StandardParams[NUM_SHADER_MATERIAL_VARS] =
{
	{ "$flags",				"flags",			SHADER_PARAM_TYPE_INTEGER,	"0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined",		"flags_defined",	SHADER_PARAM_TYPE_INTEGER,	"0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags2",  			"flags2",			SHADER_PARAM_TYPE_INTEGER,	"0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined2",	"flags2_defined",	SHADER_PARAM_TYPE_INTEGER,	"0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color",		 		"color",			SHADER_PARAM_TYPE_COLOR,	"[1 1 1]", 0 },
	{ "$alpha",	   			"alpha",			SHADER_PARAM_TYPE_FLOAT,	"1.0", 0 },
	{ "$basetexture",  		"Base Texture with lighting built in", SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", 0 },
	{ "$frame",	  			"Animation Frame",	SHADER_PARAM_TYPE_INTEGER,	"0", 0 },
	{ "$basetexturetransform", "Base Texture Texcoord Transform",SHADER_PARAM_TYPE_MATRIX,	"center .5 .5 scale 1 1 rotate 0 translate 0 0", 0 },
	{ "$flashlighttexture",  		"flashlight spotlight shape texture", SHADER_PARAM_TYPE_TEXTURE, "effects/flashlight001", SHADER_PARAM_NOT_EDITABLE },
	{ "$flashlighttextureframe",	"Animation Frame for $flashlight",	SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color2",		 		"color2",			SHADER_PARAM_TYPE_COLOR,	"[1 1 1]", 0 },
	{ "$srgbtint",		 		"tint value to be applied when running on new-style srgb parts",			SHADER_PARAM_TYPE_COLOR,	"[1 1 1]", 0 },
};


int CBaseShader::GetNumParams() const
{
	return NUM_SHADER_MATERIAL_VARS;
}

//-----------------------------------------------------------------------------
// Necessary to snag ahold of some important data for the helper methods
//-----------------------------------------------------------------------------
void CBaseShader::InitShaderParams( IMaterialVar** ppParams, const char *pMaterialName )
{
	// Re-entrancy check
	Assert( !s_ppParams );

	s_ppParams = ppParams;

	OnInitShaderParams( ppParams, pMaterialName );

	s_ppParams = NULL;
}

void CBaseShader::InitShaderInstance( IMaterialVar** ppParams, IShaderInit *pShaderInit, const char *pMaterialName, const char *pTextureGroupName )
{
	// Re-entrancy check
	Assert( !s_ppParams );

	s_ppParams = ppParams;
	s_pShaderInit = pShaderInit;
	s_pTextureGroupName = pTextureGroupName;

	OnInitShaderInstance( ppParams, pShaderInit, pMaterialName );

	s_pTextureGroupName = NULL;
	s_ppParams = NULL;
	s_pShaderInit = NULL;
}

void CBaseShader::DrawElements( IMaterialVar **ppParams, int nModulationFlags,
		IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )
{
	VPROF("CBaseShader::DrawElements");
	// Re-entrancy check
	Assert( !s_ppParams );

	s_ppParams = ppParams;
	s_pShaderAPI = pShaderAPI;
	s_pShaderShadow = pShaderShadow;
	s_nModulationFlags = nModulationFlags;

	if ( IsSnapshotting() )
	{
		// Set up the shadow state
		SetInitialShadowState( );
	}

	OnDrawElements( ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr );

	s_nModulationFlags = 0;
	s_ppParams = NULL;
	s_pShaderAPI = NULL;
	s_pShaderShadow = NULL;
}

char const* CBaseShader::GetParamName( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pName;
}

char const* CBaseShader::GetParamHelp( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pHelp;
}

ShaderParamType_t CBaseShader::GetParamType( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_Type;
}

char const* CBaseShader::GetParamDefault( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pDefaultValue;
}

int CBaseShader::GetParamFlags( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_nFlags;
}

//-----------------------------------------------------------------------------
// Sets the default shadow state
//-----------------------------------------------------------------------------
void CBaseShader::SetInitialShadowState( )
{
	// Set the default state
	s_pShaderShadow->SetDefaultState();

	// Init the standard states...
	int flags = s_ppParams[FLAGS]->GetIntValue();
	if (flags & MATERIAL_VAR_IGNOREZ)
	{
		s_pShaderShadow->EnableDepthTest( false );
		s_pShaderShadow->EnableDepthWrites( false );
	}

	if (flags & MATERIAL_VAR_DECAL)
	{
		s_pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
		s_pShaderShadow->EnableDepthWrites( false );
	}

	if (flags & MATERIAL_VAR_NOCULL)
	{
		s_pShaderShadow->EnableCulling( false );
	}

	if (flags & MATERIAL_VAR_ZNEARER)
	{
		s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_NEARER );
	}

	if (flags & MATERIAL_VAR_WIREFRAME)
	{
		s_pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE );
	}

	// Set alpha to coverage
	if (flags & MATERIAL_VAR_ALLOWALPHATOCOVERAGE)
	{
		// Force the bit on and then check against alpha blend and test states in CShaderShadowDX8::ComputeAggregateShadowState()
		s_pShaderShadow->EnableAlphaToCoverage( true );
	}
}


//-----------------------------------------------------------------------------
// Draws a snapshot
//-----------------------------------------------------------------------------
void CBaseShader::Draw( bool bMakeActualDrawCall )
{
	if ( IsSnapshotting() )
	{
		// Turn off transparency if we're asked to....
		if (g_pConfig->bNoTransparency && 
			((s_ppParams[FLAGS]->GetIntValue() & MATERIAL_VAR_NO_DEBUG_OVERRIDE) == 0))
		{
			s_pShaderShadow->EnableDepthWrites( true );
 			s_pShaderShadow->EnableBlending( false );
		}

		GetShaderSystem()->TakeSnapshot();
	}
	else
	{
		GetShaderSystem()->DrawSnapshot( bMakeActualDrawCall );
	}
}


//-----------------------------------------------------------------------------
// Finds a particular parameter	(works because the lowest parameters match the shader)
//-----------------------------------------------------------------------------
int CBaseShader::FindParamIndex( const char *pName ) const
{
	int numParams = GetNumParams();
	for( int i = 0; i < numParams; i++ )
	{
		if( Q_strnicmp( GetParamName( i ), pName, 64 ) == 0 )
		{
			return i;
		}
	}
	return -1;
}


//-----------------------------------------------------------------------------
// Are we using graphics?
//-----------------------------------------------------------------------------
bool CBaseShader::IsUsingGraphics()
{
	return GetShaderSystem()->IsUsingGraphics();
}


//-----------------------------------------------------------------------------
// Are we using graphics?
//-----------------------------------------------------------------------------
bool CBaseShader::CanUseEditorMaterials()
{
	return GetShaderSystem()->CanUseEditorMaterials();
}


//-----------------------------------------------------------------------------
// Loads a texture
//-----------------------------------------------------------------------------
void CBaseShader::LoadTexture( int nTextureVar, int nAdditionalCreationFlags )
{
	if ((!s_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = s_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		s_pShaderInit->LoadTexture( pNameVar, s_pTextureGroupName, nAdditionalCreationFlags );
	}
}


//-----------------------------------------------------------------------------
// Loads a bumpmap
//-----------------------------------------------------------------------------
void CBaseShader::LoadBumpMap( int nTextureVar )
{
	if ((!s_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = s_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		s_pShaderInit->LoadBumpMap( pNameVar, s_pTextureGroupName );
	}
}


//-----------------------------------------------------------------------------
// Loads a cubemap
//-----------------------------------------------------------------------------
void CBaseShader::LoadCubeMap( int nTextureVar, int nAdditionalCreationFlags )
{
	if ((!s_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = s_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		s_pShaderInit->LoadCubeMap( s_ppParams, pNameVar, nAdditionalCreationFlags );
	}
}


ShaderAPITextureHandle_t CBaseShader::GetShaderAPITextureBindHandle( int nTextureVar, int nFrameVar, int nTextureChannel )
{
	Assert( !IsSnapshotting() );
	Assert( nTextureVar != -1 );
	Assert ( s_ppParams );

	IMaterialVar* pTextureVar = s_ppParams[nTextureVar];
	IMaterialVar* pFrameVar = (nFrameVar != -1) ? s_ppParams[nFrameVar] : NULL;
	int nFrame = pFrameVar ? pFrameVar->GetIntValue() : 0;
	return GetShaderSystem()->GetShaderAPITextureBindHandle( pTextureVar->GetTextureValue(), nFrame, nTextureChannel );
}

//-----------------------------------------------------------------------------
// Four different flavors of BindTexture(), handling the two-sampler
// case as well as ITexture* versus textureVar forms
//-----------------------------------------------------------------------------

void CBaseShader::BindTexture( Sampler_t sampler1, int nTextureVar, int nFrameVar /* = -1 */ )
{
	BindTexture( sampler1, (Sampler_t) -1, nTextureVar, nFrameVar );
}


void CBaseShader::BindTexture( Sampler_t sampler1, Sampler_t sampler2, int nTextureVar, int nFrameVar /* = -1 */ )
{
	Assert( !IsSnapshotting() );
	Assert( nTextureVar != -1 );
	Assert ( s_ppParams );

	IMaterialVar* pTextureVar = s_ppParams[nTextureVar];
	IMaterialVar* pFrameVar = (nFrameVar != -1) ? s_ppParams[nFrameVar] : NULL;
	if (pTextureVar)
	{
		int nFrame = pFrameVar ? pFrameVar->GetIntValue() : 0;

		if ( sampler2 == -1 )
		{
			GetShaderSystem()->BindTexture( sampler1, pTextureVar->GetTextureValue(), nFrame );
		}
		else
		{
			GetShaderSystem()->BindTexture( sampler1, sampler2, pTextureVar->GetTextureValue(), nFrame );
		}
	}
}


void CBaseShader::BindTexture( Sampler_t sampler1, ITexture *pTexture, int nFrame /* = 0 */ )
{
	BindTexture( sampler1, (Sampler_t) -1, pTexture, nFrame );
}

void CBaseShader::BindTexture( Sampler_t sampler1, Sampler_t sampler2, ITexture *pTexture, int nFrame /* = 0 */ )
{
	Assert( !IsSnapshotting() );

	if ( sampler2 == -1 )
	{
		GetShaderSystem()->BindTexture( sampler1, pTexture, nFrame );
	}
	else
	{
		GetShaderSystem()->BindTexture( sampler1, sampler2, pTexture, nFrame );
	}
}

void CBaseShader::GetTextureDimensions( float* pOutWidth, float* pOutHeight, int nTextureVar )
{
	IMaterialVar* pMaterialVar = s_ppParams[nTextureVar];
	if ( pMaterialVar->IsDefined() && pMaterialVar->IsTexture() )
	{
		ITexture* pTexture = pMaterialVar->GetTextureValue();
		*pOutWidth = pTexture->GetActualWidth();
		*pOutHeight = pTexture->GetActualHeight();
	}
	else
	{
		*pOutWidth = 0.0f;
		*pOutHeight = 0.0f;
	}
}


//-----------------------------------------------------------------------------
// Does the texture store translucency in its alpha channel?
//-----------------------------------------------------------------------------
bool CBaseShader::TextureIsTranslucent( int textureVar, bool isBaseTexture )
{
	if (textureVar < 0)
		return false;

	IMaterialVar** params = s_ppParams;
	if (params[textureVar]->GetType() == MATERIAL_VAR_TYPE_TEXTURE)
	{
		if (!isBaseTexture)
		{
			return params[textureVar]->GetTextureValue()->IsTranslucent();
		}
		else
		{
			// Override translucency settings if this flag is set.
			if (IS_FLAG_SET(MATERIAL_VAR_OPAQUETEXTURE))
				return false;

			bool bHasSelfIllum				= ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_SELFILLUM ) != 0 );
			bool bHasBaseAlphaEnvmapMask	= ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_BASEALPHAENVMAPMASK ) != 0 );
			// Check if we are using base texture alpha for something other than translucency.
			if ( !bHasSelfIllum && !bHasBaseAlphaEnvmapMask )
			{
				// We aren't using base alpha for anything other than trancluceny.

				// check if the material is marked as translucent or alpha test.
				if ((CurrentMaterialVarFlags() & MATERIAL_VAR_TRANSLUCENT) ||
					(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST))
				{
					// Make sure the texture has an alpha channel.
					return params[textureVar]->GetTextureValue()->IsTranslucent();
				}
			}
		}
	}

	return false;
}

float CBaseShader::GetAlpha( IMaterialVar** ppParams )
{
	if ( !ppParams )
	{
		ppParams = s_ppParams;
	}

	if ( ppParams[FLAGS]->GetIntValue() & MATERIAL_VAR_NOALPHAMOD )
	{
		return 1.0f;
	}

	return ppParams[ALPHA]->GetFloatValue();
}

//-----------------------------------------------------------------------------
//
// Helper methods for color modulation
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Are we alpha or color modulating?
//-----------------------------------------------------------------------------
bool CBaseShader::IsAlphaModulating()
{
	return (s_nModulationFlags & SHADER_USING_ALPHA_MODULATION) != 0;
}

void CBaseShader::ComputeModulationColor( float* color )
{
	IMaterialVar *pMaterialVar = s_ppParams[COLOR];
	if( pMaterialVar->GetType() == MATERIAL_VAR_TYPE_VECTOR )
	{
		pMaterialVar->GetVecValue( color, 3 );
	}
	else
	{
		float flValue = pMaterialVar->GetFloatValue();
		color[0] = flValue;
		color[1] = flValue;
		color[2] = flValue;
	}

	ApplyColor2Factor( color );
	color[3] = GetAlpha();
}

// FIXME: Figure out a better way to do this?
//-----------------------------------------------------------------------------
int CBaseShader::ComputeModulationFlags( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI )
{
	s_pShaderAPI = pShaderAPI;

	int mod = 0;

	if ( GetAlpha( params ) != 1.0f )
	{
		mod |= SHADER_USING_ALPHA_MODULATION;
	}

	if( UsingFlashlight(params) )
	{
		mod |= SHADER_USING_FLASHLIGHT;
	}

	if ( UsingEditor(params) )
	{
		mod |= SHADER_USING_EDITOR;
	}

	if( IS_FLAG2_SET( MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING ) )
	{
		AssertOnce( IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) );
		if( IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) )
		{
			mod |= SHADER_USING_FIXED_FUNCTION_BAKED_LIGHTING;
		}
	}

	s_pShaderAPI = NULL;
	return mod;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
{ 
	return CShader_IsFlag2Set( params, MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE ); 
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsFullFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
{ 
	return CShader_IsFlag2Set( params, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE ); 
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CBaseShader::IsTranslucent( IMaterialVar **params ) const
{
	return IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT );
}

//-----------------------------------------------------------------------------
// Returns the translucency...
//-----------------------------------------------------------------------------
void CBaseShader::ApplyColor2Factor( float *pColorOut ) const // (*pColorOut) *= COLOR2
{
	if ( !g_pConfig->bShowDiffuse )
	{
		pColorOut[0] = pColorOut[1] = pColorOut[2] = 0.0f;
		return;
	}

	IMaterialVar* pColor2Var = s_ppParams[COLOR2];
	if ( pColor2Var->GetType() == MATERIAL_VAR_TYPE_VECTOR )
	{
		float flColor2[3];
		pColor2Var->GetVecValue( flColor2, 3 );
		
		pColorOut[0] *= flColor2[0];
		pColorOut[1] *= flColor2[1];
		pColorOut[2] *= flColor2[2];
	}
	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() )
	{
		IMaterialVar* pSRGBVar = s_ppParams[SRGBTINT];
		if (pSRGBVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		{
			float flSRGB[3];
			pSRGBVar->GetVecValue( flSRGB, 3 );
			
			pColorOut[0] *= flSRGB[0];
			pColorOut[1] *= flSRGB[1];
			pColorOut[2] *= flSRGB[2];
		}
	}
}


//-----------------------------------------------------------------------------
//
// Helper methods for alpha blending....
//
//-----------------------------------------------------------------------------
void CBaseShader::EnableAlphaBlending( ShaderBlendFactor_t src, ShaderBlendFactor_t dst )
{
	Assert( IsSnapshotting() );
	s_pShaderShadow->EnableBlending( true );
	s_pShaderShadow->BlendFunc( src, dst );
	s_pShaderShadow->EnableDepthWrites(false);
}

void CBaseShader::DisableAlphaBlending()
{
	Assert( IsSnapshotting() );
	s_pShaderShadow->EnableBlending( false );
}

void CBaseShader::SetNormalBlendingShadowState( int textureVar, bool isBaseTexture )
{
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if (isTranslucent)
	{
		EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		DisableAlphaBlending();
	}
}

//ConVar mat_debug_flashlight_only( "mat_debug_flashlight_only", "0" );
void CBaseShader::SetAdditiveBlendingShadowState( int textureVar, bool isBaseTexture )
{
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	/*
	if ( mat_debug_flashlight_only.GetBool() )
	{
		if (isTranslucent)
		{
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			//s_pShaderShadow->EnableAlphaTest( true );
			//s_pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.99f );
		}
		else
		{
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ZERO);
		}
	}
	else
	*/
	{
		if (isTranslucent)
		{
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
		}
		else
		{
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
		}
	}
}

void CBaseShader::SetDefaultBlendingShadowState( int textureVar, bool isBaseTexture ) 
{
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{
		SetAdditiveBlendingShadowState( textureVar, isBaseTexture );
	}
	else
	{
		SetNormalBlendingShadowState( textureVar, isBaseTexture );
	}
}

void CBaseShader::SetBlendingShadowState( BlendType_t nMode )
{
	switch ( nMode )
	{
		case BT_NONE:
			DisableAlphaBlending();
			break;

		case BT_BLEND:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			break;

		case BT_ADD:
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			break;

		case BT_BLENDADD:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
			break;
	}
}



//-----------------------------------------------------------------------------
// Sets lightmap blending mode for single texturing
//-----------------------------------------------------------------------------
void CBaseShader::SingleTextureLightmapBlendMode( )
{
	Assert( IsSnapshotting() );

	s_pShaderShadow->EnableBlending( true );
	s_pShaderShadow->BlendFunc( SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR );
}

FORCEINLINE void CBaseShader::SetFogMode( ShaderFogMode_t fogMode )
{
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		s_pShaderShadow->FogMode( fogMode );
	}
	else
	{
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

//-----------------------------------------------------------------------------
//
// Helper methods for fog
//
//-----------------------------------------------------------------------------
void CBaseShader::FogToOOOverbright( void )
{
	Assert( IsSnapshotting() );
	SetFogMode( SHADER_FOGMODE_OO_OVERBRIGHT );
}

void CBaseShader::FogToWhite( void )
{
	Assert( IsSnapshotting() );
	SetFogMode( SHADER_FOGMODE_WHITE );
}
void CBaseShader::FogToBlack( void )
{
	Assert( IsSnapshotting() );
	SetFogMode( SHADER_FOGMODE_BLACK );
}

void CBaseShader::FogToGrey( void )
{
	Assert( IsSnapshotting() );
	SetFogMode( SHADER_FOGMODE_GREY );
}

void CBaseShader::FogToFogColor( void )
{
	Assert( IsSnapshotting() );
	SetFogMode( SHADER_FOGMODE_FOGCOLOR );
}

void CBaseShader::DisableFog( void )
{
	Assert( IsSnapshotting() );
	s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
}

void CBaseShader::DefaultFog( void )
{
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{
		FogToBlack();
	}
	else
	{
		FogToFogColor();
	}
}

bool CBaseShader::UsingFlashlight( IMaterialVar **params ) const
{
	if( IsSnapshotting() )
	{
		return CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_FLASHLIGHT );
	}
	else
	{
		return s_pShaderAPI->InFlashlightMode();
	}
}

bool CBaseShader::UsingEditor( IMaterialVar **params ) const
{
	if( IsSnapshotting() )
	{
		return CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_EDITOR );
	}
	else
	{
		return s_pShaderAPI->InEditorMode();
	}
}

bool CBaseShader::IsHDREnabled( void )
{
	// HDRFIXME!  Need to fix this for vgui materials
	HDRType_t hdr_mode = g_pHardwareConfig->GetHDRType();
	return ( hdr_mode == HDR_TYPE_INTEGER ) || ( hdr_mode == HDR_TYPE_FLOAT );
}
