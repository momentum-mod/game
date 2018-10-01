#ifndef _C_MOM_TRIGGERS_H_
#define _C_MOM_TRIGGERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_mom_basetoggle.h"
#include "prediction.h"

//
// Spawnflags
//

class C_BaseMomentumTrigger : public C_BaseToggle
{
    DECLARE_CLASS(C_BaseMomentumTrigger, C_BaseToggle);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

  public:
    C_BaseMomentumTrigger(){};

	void UpdatePartitionListEntry() OVERRIDE;
	void Spawn() OVERRIDE;

	/*void Activate( void );
	virtual void PostClientActive( void );
	void InitTrigger( void );

	void Enable( void );
	void Disable( void );
	void UpdateOnRemove( void );
	void TouchTest(  void );*/

	// Input handlers
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggle( inputdata_t &inputdata );
	virtual void InputTouchTest ( inputdata_t &inputdata );
	virtual void InputStartTouch( inputdata_t &inputdata );
	virtual void InputEndTouch( inputdata_t &inputdata );

	virtual bool UsesFilter( void ){ return ( m_hFilter.Get() != NULL ); }
	virtual bool PassesTriggerFilters(CBaseEntity *pOther);

	// Touch handlers
	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	virtual void OnStartTouch(CBaseEntity *pOther) {}
	virtual void OnEndTouch(CBaseEntity *pOther) {}
	virtual void StartTouchAll() {}
	virtual void EndTouchAll() {}

	bool IsTouching( CBaseEntity *pOther );

	CBaseEntity *GetTouchedEntityOfType( const char *sClassName );

	bool PointIsWithin( const Vector &vecPoint );

	CNetworkString(m_iszTarget, MAX_POINT_NAME);
	CNetworkString(m_iszModel, MAX_TRIGGER_NAME);
	CNetworkString(m_iszFilter, MAX_FILTER_NAME);

private:
	bool m_bDisabled;
	string_t m_iFilterName;
	CHandle<class CBaseFilter> m_hFilter;
};

class C_TriggerTimerStart : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerTimerStop : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerSlide : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();

    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
    CNetworkVar(bool, m_bFixUpsideSlope);
};

class C_TriggerTeleport : public C_BaseMomentumTrigger
{
public:
	DECLARE_CLASS(C_TriggerTeleport, C_BaseMomentumTrigger);
	DECLARE_CLIENTCLASS();

	void StartTouch(CBaseEntity *pOther) OVERRIDE;

	CNetworkString(m_iszLandmark, MAX_LANDMARK_NAME);
};

class C_TriggerPush : public C_BaseMomentumTrigger
{
public:
	DECLARE_CLASS(C_TriggerPush, C_BaseMomentumTrigger);
	DECLARE_CLIENTCLASS();

	void Spawn(void) OVERRIDE;
	void Activate( void ) OVERRIDE;
	void Touch( CBaseEntity *pOther ) OVERRIDE;
	
	CNetworkVar(float, m_flAlternateTicksFix); // Scale factor to apply to the push speed when running with alternate ticks
	CNetworkVar(float, m_flPushSpeed);
	CNetworkVector(m_vecPushDir);

	int m_nTickBasePush;
	int m_iUserID;
};

class C_TriggerMultiple : public C_BaseMomentumTrigger
{
public:
	DECLARE_CLASS(C_TriggerMultiple, C_BaseMomentumTrigger);
	DECLARE_CLIENTCLASS();

	void Spawn( void );
	void MultiTouch( CBaseEntity *pOther );
	void MultiWaitOver( void );
	void ActivateMultiTrigger(CBaseEntity *pActivator);

	// Outputs
	//COutputEvent m_OnTrigger;
};

class C_PointEntity : public C_BaseEntity
{
  public:
    DECLARE_CLASS(C_PointEntity, C_BaseEntity);
    DECLARE_CLIENTCLASS();

    void Spawn() OVERRIDE;
};

#endif