
#if !defined ( ENV_DOF_CONTROLLER_H )
#define ENV_DOF_CONTROLLER_H
#ifdef _WIN32
#pragma once
#endif

struct DOFControlSettings_t
{
	// Near plane
	float	flNearBlurDepth;
	float	flNearBlurRadius;
	float	flNearFocusDistance;
	// Far plane
	float	flFarBlurDepth;
	float	flFarBlurRadius;
	float	flFarFocusDistance;
};

//-----------------------------------------------------------------------------
// Purpose: Entity that controls depth of field postprocessing
//-----------------------------------------------------------------------------
class CEnvDOFController : public CPointEntity
{
	DECLARE_CLASS( CEnvDOFController, CPointEntity );
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual int		UpdateTransmitState( void );
	void			SetControllerState( DOFControlSettings_t setting );

	void	UpdateParamBlend( void );

	// Inputs
	void	InputSetNearBlurDepth( inputdata_t &inputdata );
	void	InputSetNearFocusDepth( inputdata_t &inputdata );
	void	InputSetFarFocusDepth( inputdata_t &inputdata );
	void	InputSetFarBlurDepth( inputdata_t &inputdata );
	void	InputSetNearBlurRadius( inputdata_t &inputdata );
	void	InputSetFarBlurRadius( inputdata_t &inputdata );
	void	InputBlendDOFScale( inputdata_t &inputdata );
	
	void	InputSetFocusTarget( inputdata_t &inputdata );
	void	InputSetFocusTargetRange( inputdata_t &inputdata );

private:
	float	m_flFocusTargetRange;

	string_t	m_strFocusTargetName;	// Name of the entity to focus on
	EHANDLE		m_hFocusTarget;

	CNetworkVar( bool, m_bDOFEnabled );
	CNetworkVar( float, m_flNearBlurDepth );
	CNetworkVar( float, m_flNearFocusDepth );
	CNetworkVar( float, m_flFarFocusDepth );
	CNetworkVar( float, m_flFarBlurDepth );
	CNetworkVar( float, m_flNearBlurRadius );
	CNetworkVar( float, m_flFarBlurRadius );
};

#endif//  ENV_DOF_CONTROLLER_H
