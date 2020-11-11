//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_EnvProjectedTexture : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvProjectedTexture, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	void SetMaterial( IMaterial *pMaterial );
	void SetLightColor( byte r, byte g, byte b, byte a );
	void SetSize( float flSize );
	void SetRotation( float flRotation );

	virtual void OnDataChanged( DataUpdateType_t updateType );
	void	ShutDownLightHandle( void );

	virtual void Simulate();

	void	UpdateLight( void );

	C_EnvProjectedTexture();
	~C_EnvProjectedTexture();

	static void SetVisibleBBoxMinHeight( float flVisibleBBoxMinHeight ) { m_flVisibleBBoxMinHeight = flVisibleBBoxMinHeight; }
	static float GetVisibleBBoxMinHeight( void ) { return m_flVisibleBBoxMinHeight; }
	static C_EnvProjectedTexture *Create( );

private:

	inline bool IsBBoxVisible( void );
	bool IsBBoxVisible( Vector vecExtentsMin,
						Vector vecExtentsMax );

	ClientShadowHandle_t m_LightHandle;
	bool m_bForceUpdate;

	EHANDLE	m_hTargetEntity;
	bool m_bDontFollowTarget;

	bool		m_bState;
	bool		m_bAlwaysUpdate;
	float		m_flLightFOV;
	float		m_flLightHorFOV;
	bool		m_bEnableShadows;
	bool		m_bLightOnlyTarget;
	bool		m_bLightWorld;
	bool		m_bCameraSpace;
	float		m_flBrightnessScale;
	color32		m_LightColor;
	Vector		m_CurrentLinearFloatLightColor;
	float		m_flCurrentLinearFloatLightAlpha;
	float		m_flCurrentBrightnessScale;
	float		m_flColorTransitionTime;
	float		m_flAmbient;
	float		m_flNearZ;
	float		m_flFarZ;
	char		m_SpotlightTextureName[ MAX_PATH ];
	CTextureReference m_SpotlightTexture;
	int			m_nSpotlightTextureFrame;
	int			m_nShadowQuality;
	float		m_flConstantAtten;
	float		m_flLinearAtten;
	float		m_flQuadraticAtten;
	float		m_flShadowAtten;

	bool		m_bAlwaysDraw;
	//bool		m_bProjectedTextureVersion;

	Vector	m_vecExtentsMin;
	Vector	m_vecExtentsMax;

	static float m_flVisibleBBoxMinHeight;
};



bool C_EnvProjectedTexture::IsBBoxVisible( void )
{
	return IsBBoxVisible( GetAbsOrigin() + m_vecExtentsMin, GetAbsOrigin() + m_vecExtentsMax );
}
