//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity to control screen overlays on a player
//
//=============================================================================

#include "cbase.h"
#include "shareddefs.h"
#include "env_projectedtexture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ENV_PROJECTEDTEXTURE_STARTON			(1<<0)

LINK_ENTITY_TO_CLASS( env_projectedtexture, CEnvProjectedTexture );

BEGIN_DATADESC( CEnvProjectedTexture )
	DEFINE_FIELD( m_hTargetEntity, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_bDontFollowTarget, FIELD_BOOLEAN, "dontfollowtarget" ),
	DEFINE_FIELD( m_bAlwaysUpdate, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bState, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_flLightFOV, FIELD_FLOAT, "lightfov" ),
	DEFINE_KEYFIELD( m_flLightHorFOV, FIELD_FLOAT, "lighthorfov" ),
	DEFINE_KEYFIELD( m_bEnableShadows, FIELD_BOOLEAN, "enableshadows" ),
	DEFINE_KEYFIELD( m_bLightOnlyTarget, FIELD_BOOLEAN, "lightonlytarget" ),
	DEFINE_KEYFIELD( m_bLightWorld, FIELD_BOOLEAN, "lightworld" ),
	DEFINE_KEYFIELD( m_bCameraSpace, FIELD_BOOLEAN, "cameraspace" ),
	DEFINE_KEYFIELD( m_flAmbient, FIELD_FLOAT, "ambient" ),
	DEFINE_AUTO_ARRAY( m_SpotlightTextureName, FIELD_CHARACTER ),
	DEFINE_KEYFIELD( m_nSpotlightTextureFrame, FIELD_INTEGER, "textureframe" ),
	DEFINE_KEYFIELD( m_flNearZ, FIELD_FLOAT, "nearz" ),
	DEFINE_KEYFIELD( m_flFarZ, FIELD_FLOAT, "farz" ),
	DEFINE_KEYFIELD( m_nShadowQuality, FIELD_INTEGER, "shadowquality" ),
	DEFINE_KEYFIELD( m_bAlwaysDraw, FIELD_BOOLEAN, "alwaysdraw" ),
	DEFINE_KEYFIELD( m_bProjectedTextureVersion, FIELD_BOOLEAN, "ProjectedTextureVersion" ),
	DEFINE_KEYFIELD( m_flBrightnessScale, FIELD_FLOAT, "brightnessscale" ),
	DEFINE_FIELD( m_LightColor, FIELD_COLOR32 ), 
	DEFINE_KEYFIELD( m_flColorTransitionTime, FIELD_FLOAT, "colortransitiontime" ),
	DEFINE_FIELD( m_flConstantAtten, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLinearAtten, FIELD_FLOAT ),
	DEFINE_FIELD( m_flQuadraticAtten, FIELD_FLOAT ),
	DEFINE_KEYFIELD( m_flShadowAtten, FIELD_FLOAT, "shadowatten" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AlwaysUpdateOn", InputAlwaysUpdateOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AlwaysUpdateOff", InputAlwaysUpdateOff ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "FOV", InputSetFOV ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "VerFOV", InputSetVerFOV ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "HorFOV", InputSetHorFOV ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "Target", InputSetTarget ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "CameraSpace", InputSetCameraSpace ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "LightOnlyTarget", InputSetLightOnlyTarget ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "LightWorld", InputSetLightWorld ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "EnableShadows", InputSetEnableShadows ),
	DEFINE_INPUTFUNC( FIELD_COLOR32, "LightColor", InputSetLightColor ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "Ambient", InputSetAmbient ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SpotlightTexture", InputSetSpotlightTexture ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetSpotlightFrame", InputSetSpotlightFrame ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBrightness", InputSetBrightness ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetColorTransitionTime", InputSetColorTransitionTime ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetQuadratic", InputSetQuadratic ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetLinear", InputSetLinear ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetConstant", InputSetConstant ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetShadowAtten", InputSetShadowAtten ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetNearZ", InputSetNearZ ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetFarZ", InputSetFarZ ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AlwaysDrawOn", InputAlwaysDrawOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AlwaysDrawOff", InputAlwaysDrawOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StopFollowingTarget", InputStopFollowingTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StartFollowingTarget", InputStartFollowingTarget ),
	DEFINE_THINKFUNC( InitialThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CEnvProjectedTexture, DT_EnvProjectedTexture )
	SendPropEHandle( SENDINFO( m_hTargetEntity ) ),
	SendPropBool( SENDINFO( m_bDontFollowTarget ) ),
	SendPropBool( SENDINFO( m_bState ) ),
	SendPropBool( SENDINFO( m_bAlwaysUpdate ) ),
	SendPropFloat( SENDINFO( m_flLightFOV ) ),
	SendPropFloat( SENDINFO( m_flLightHorFOV ) ),
	SendPropBool( SENDINFO( m_bEnableShadows ) ),
	SendPropBool( SENDINFO( m_bLightOnlyTarget ) ),
	SendPropBool( SENDINFO( m_bLightWorld ) ),
	SendPropBool( SENDINFO( m_bCameraSpace ) ),
	SendPropFloat( SENDINFO( m_flBrightnessScale ) ),
	SendPropInt( SENDINFO ( m_LightColor ),	32, SPROP_UNSIGNED, SendProxy_Color32ToInt ),
	SendPropFloat( SENDINFO( m_flColorTransitionTime ) ),
	SendPropFloat( SENDINFO( m_flAmbient ) ),
	SendPropString( SENDINFO( m_SpotlightTextureName ) ),
	SendPropInt( SENDINFO( m_nSpotlightTextureFrame ) ),
	SendPropFloat( SENDINFO( m_flNearZ ), 16, SPROP_ROUNDDOWN, 0.0f,  500.0f ),
	SendPropFloat( SENDINFO( m_flFarZ ),  18, SPROP_ROUNDDOWN, 0.0f, 1500.0f ),
	SendPropInt( SENDINFO( m_nShadowQuality ), 1, SPROP_UNSIGNED ),  // Just one bit for now
	SendPropFloat( SENDINFO( m_flConstantAtten ) ),
	SendPropFloat( SENDINFO( m_flLinearAtten ) ),
	SendPropFloat( SENDINFO( m_flQuadraticAtten ) ),
	SendPropFloat( SENDINFO( m_flShadowAtten ) ),
	SendPropBool( SENDINFO( m_bAlwaysDraw ) ),

	// Not needed on the client right now, change when it actually is needed
	//SendPropBool( SENDINFO( m_bProjectedTextureVersion ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvProjectedTexture::CEnvProjectedTexture( void )
{
	m_bState = true;
	m_bAlwaysUpdate = false;
	m_flLightFOV = 45.0f;
	m_bEnableShadows = false;
	m_bLightOnlyTarget = false;
	m_bLightWorld = true;
	m_bCameraSpace = false;

	Q_strcpy( m_SpotlightTextureName.GetForModify(), "effects/flashlight001" );

	m_nSpotlightTextureFrame = 0;
	m_flBrightnessScale = 1.0f;
	m_LightColor.Init( 255, 255, 255, 255 );
	m_flColorTransitionTime = 0.0f;
	m_flAmbient = 0.0f;
	m_flNearZ = 4.0f;
	m_flFarZ = 750.0f;
	m_nShadowQuality = 0;
	m_flQuadraticAtten = 0.0f;
	m_flLinearAtten = 100.0f;
	m_flConstantAtten = 0.0f;
	m_flShadowAtten = 0.0f;
}

void UTIL_ColorStringToLinearFloatColor( Vector &color, const char *pString )
{
	float tmp[4];
	UTIL_StringToFloatArray( tmp, 4, pString );
	if( tmp[3] <= 0.0f )
	{
		tmp[3] = 255.0f;
	}
	tmp[3] *= ( 1.0f / 255.0f );
	color.x = tmp[0] * ( 1.0f / 255.0f ) * tmp[3];
	color.y = tmp[1] * ( 1.0f / 255.0f ) * tmp[3];
	color.z = tmp[2] * ( 1.0f / 255.0f ) * tmp[3];
}

bool CEnvProjectedTexture::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "lightcolor" ) )
	{
		// 
		// Most existing projected textures don't have intensity implemented.
		// This can give them 0% intensity, which makes them invisible.
		// If intensity is not detected, assume 255.
		// 
		// Also, get rid of the floats, for god's sake. Have you ever seen a color32 with decimals?
		// 
		int tmp[4];
		tmp[3] = 255;
		UTIL_StringToIntArray_PreserveArray( tmp, 4, szValue );

		m_LightColor.SetR( tmp[0] );
		m_LightColor.SetG( tmp[1] );
		m_LightColor.SetB( tmp[2] );
		m_LightColor.SetA( tmp[3] );
	}
	else if ( FStrEq( szKeyName, "texturename" ) )
	{
		Q_strcpy( m_SpotlightTextureName.GetForModify(), szValue );
	}
	else if ( FStrEq( szKeyName, "constant_attn" ) )
	{
		m_flConstantAtten = CorrectConstantAtten( atof( szValue ) );
	}
	else if ( FStrEq( szKeyName, "linear_attn" ) )
	{
		m_flLinearAtten = CorrectLinearAtten( atof( szValue ) );
	}
	else if ( FStrEq( szKeyName, "quadratic_attn" ) )
	{
		m_flQuadraticAtten = CorrectQuadraticAtten( atof( szValue ) );
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}

bool CEnvProjectedTexture::GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen )
{
	if ( FStrEq( szKeyName, "lightcolor" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%d %d %d %d", m_LightColor.GetR(), m_LightColor.GetG(), m_LightColor.GetB(), m_LightColor.GetA() );
		return true;
	}
	else if ( FStrEq( szKeyName, "texturename" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%s", m_SpotlightTextureName.Get() );
		return true;
	}
	else if ( FStrEq( szKeyName, "constant_attn" ) )
	{
		// Undo correction
		Q_snprintf( szValue, iMaxLen, "%f", m_flConstantAtten *= 2.0f );
		return true;
	}
	else if ( FStrEq( szKeyName, "linear_attn" ) )
	{
		// Undo correction
		Q_snprintf( szValue, iMaxLen, "%f", m_flLinearAtten *= 0.01f );
		return true;
	}
	else if ( FStrEq( szKeyName, "quadratic_attn" ) )
	{
		// Undo correction
		Q_snprintf( szValue, iMaxLen, "%f", m_flQuadraticAtten *= 0.0001f );
		return true;
	}
	return BaseClass::GetKeyValue( szKeyName, szValue, iMaxLen );
}

void CEnvProjectedTexture::InputTurnOn( inputdata_t &inputdata )
{
	m_bState = true;
}

void CEnvProjectedTexture::InputTurnOff( inputdata_t &inputdata )
{
	m_bState = false;
}

void CEnvProjectedTexture::InputAlwaysUpdateOn( inputdata_t &inputdata )
{
	m_bAlwaysUpdate = true;
}

void CEnvProjectedTexture::InputAlwaysUpdateOff( inputdata_t &inputdata )
{
	m_bAlwaysUpdate = false;
}

void CEnvProjectedTexture::InputSetFOV( inputdata_t &inputdata )
{
	m_flLightFOV = inputdata.value.Float();
	m_flLightHorFOV = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetVerFOV( inputdata_t &inputdata )
{
	m_flLightFOV = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetHorFOV( inputdata_t &inputdata )
{
	m_flLightHorFOV = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetTarget( inputdata_t &inputdata )
{
	// env_projectedtexture's "Target" uses FIELD_EHANDLE while Mapbase's base entity "SetTarget" uses FIELD_STRING
	if (inputdata.value.FieldType() != FIELD_EHANDLE)
		inputdata.value.Convert(FIELD_EHANDLE, this, inputdata.pActivator, inputdata.pCaller);
	m_hTargetEntity = inputdata.value.Entity();
}

void CEnvProjectedTexture::InputSetCameraSpace( inputdata_t &inputdata )
{
	m_bCameraSpace = inputdata.value.Bool();
}

void CEnvProjectedTexture::InputSetLightOnlyTarget( inputdata_t &inputdata )
{
	m_bLightOnlyTarget = inputdata.value.Bool();
}

void CEnvProjectedTexture::InputSetLightWorld( inputdata_t &inputdata )
{
	m_bLightWorld = inputdata.value.Bool();
}

void CEnvProjectedTexture::InputSetEnableShadows( inputdata_t &inputdata )
{
	m_bEnableShadows = inputdata.value.Bool();
}

void CEnvProjectedTexture::InputSetLightColor( inputdata_t &inputdata )
{
	m_LightColor = inputdata.value.Color32();
}

void CEnvProjectedTexture::InputSetAmbient( inputdata_t &inputdata )
{
	m_flAmbient = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetSpotlightTexture( inputdata_t &inputdata )
{
	Q_strcpy( m_SpotlightTextureName.GetForModify(), inputdata.value.String() );
}

void CEnvProjectedTexture::InputSetSpotlightFrame( inputdata_t &inputdata )
{
	m_nSpotlightTextureFrame = inputdata.value.Int();
}

void CEnvProjectedTexture::InputSetBrightness( inputdata_t &inputdata )
{
	m_flBrightnessScale = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetColorTransitionTime( inputdata_t &inputdata )
{
	m_flColorTransitionTime = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetNearZ( inputdata_t &inputdata )
{
	m_flNearZ = inputdata.value.Float();
}

void CEnvProjectedTexture::InputSetFarZ( inputdata_t &inputdata )
{
	m_flFarZ = inputdata.value.Float();
}

void CEnvProjectedTexture::Spawn( void )
{
	// Set m_flLightHorFOV to m_flLightFOV if it's still 0, indicating either legacy support or desire to do this
	if (m_flLightHorFOV == 0.0f)
	{
		m_flLightHorFOV = m_flLightFOV;
	}

	m_bState = ( ( GetSpawnFlags() & ENV_PROJECTEDTEXTURE_STARTON ) != 0 );
	m_bAlwaysUpdate = ( ( GetSpawnFlags() & ENV_PROJECTEDTEXTURE_ALWAYSUPDATE ) != 0 );

	BaseClass::Spawn();
}

void CEnvProjectedTexture::Activate( void )
{
	SetThink( &CEnvProjectedTexture::InitialThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	if (m_bProjectedTextureVersion == 0 && GetMoveParent())
	{
		// Pre-Mapbase projected textures should use the VDC parenting fix.
		m_bAlwaysUpdate = true;
	}

	BaseClass::Activate();
}

void CEnvProjectedTexture::SetParent( CBaseEntity* pNewParent, int iAttachment )
{
	BaseClass::SetParent( pNewParent, iAttachment );

	if (m_bProjectedTextureVersion == 0)
	{
		// Pre-Mapbase projected textures should use the VDC parenting fix.
		// Since the ASW changes structure projected textures differently,
		// we have to set it here on the server when our parent changes.
		// Don't run this code if we already have the spawnflag though.
		if ( ( GetSpawnFlags() & ENV_PROJECTEDTEXTURE_ALWAYSUPDATE ) == 0 )
		{
			m_bAlwaysUpdate = GetMoveParent() != NULL;
		}
	}
}

void CEnvProjectedTexture::InitialThink( void )
{
	m_hTargetEntity = gEntList.FindEntityByName( NULL, m_target );
}

int CEnvProjectedTexture::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

// Console command for creating env_projectedtexture entities
void CC_CreateFlashlight( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if( !pPlayer )
		return;

	QAngle angles = pPlayer->EyeAngles();
	Vector origin = pPlayer->EyePosition();		

	CEnvProjectedTexture *pFlashlight = dynamic_cast< CEnvProjectedTexture * >( CreateEntityByName("env_projectedtexture") );
	if( args.ArgC() > 1 )
	{
		pFlashlight->SetName( AllocPooledString( args[1] ) );
	}

	pFlashlight->Teleport( &origin, &angles, NULL );

}
static ConCommand create_flashlight("create_flashlight", CC_CreateFlashlight, 0, FCVAR_CHEAT);
