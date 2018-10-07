#ifndef _MOM_TRIGGERS_H_
#define _MOM_TRIGGERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basetrigger.h"

#ifdef CLIENT_DLL
#define CTriggerMultiple C_TriggerMultiple
#define CTriggerOnce C_TriggerOnce
#define CTriggerLook C_TriggerLook
#define CTriggerPush C_TriggerPush
#define CTriggerTeleport C_TriggerTeleport
#else
#endif

// ##################################################################################
//	>> TriggerMultiple
// ##################################################################################
class CTriggerMultiple : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerMultiple, CBaseTrigger);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);
	void MultiTouch(CBaseEntity *pOther);
	void MultiWaitOver(void);
	void ActivateMultiTrigger(CBaseEntity *pActivator);

	// Outputs
	COutputEvent m_OnTrigger;

#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// Global list of triggers that care about weapon fire
extern CUtlVector< CHandle<CTriggerMultiple> >	g_hWeaponFireTriggers;

// ##################################################################################
//	>> TriggerOnce
// ##################################################################################
class CTriggerOnce : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CTriggerOnce, CTriggerMultiple);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);

#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ##################################################################################
//	>> TriggerLook
//
//  Triggers once when player is looking at m_target
//
// ##################################################################################
#define SF_TRIGGERLOOK_FIREONCE		128
#define SF_TRIGGERLOOK_USEVELOCITY	256

class CTriggerLook : public CTriggerOnce
{
public:
	DECLARE_CLASS(CTriggerLook, CTriggerOnce);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);
	virtual void Touch(CBaseEntity *pOther);
	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

private:
	void Trigger(CBaseEntity *pActivator, bool bTimeout);
	void TimeoutThink();

	// Vars
public:
	EHANDLE m_hLookTarget;
	float m_flFieldOfView;
	float m_flLookTime;			// How long must I look for
	float m_flLookTimeTotal;	// How long have I looked
	float m_flLookTimeLast;		// When did I last look
	float m_flTimeoutDuration;	// Number of seconds after start touch to fire anyway
	bool m_bTimeoutFired;		// True if the OnTimeout output fired since the last StartTouch.
	EHANDLE m_hActivator;		// The entity that triggered us.

	// Outputs 
	COutputEvent m_OnTimeout;
#ifdef GAME_DLL // Server specific things
public:
	int	 DrawDebugTextOverlays(void);
	DECLARE_DATADESC();
#endif
};

// ##################################################################################
//	>> TriggerPush
//
//  Purpose: A trigger that pushes the player, NPCs, or objects.
//
// ##################################################################################
class CTriggerPush : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerPush, CBaseTrigger);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();

	int m_nTickBasePush;
	int m_iUserID;
#endif

	// Shared things
	virtual void Spawn(void);
	virtual void Activate(void);
	virtual void Touch(CBaseEntity *pOther);

	// Vars
	CNetworkVar(float, m_flAlternateTicksFix); // Scale factor to apply to the push speed when running with alternate ticks
	CNetworkVar(float, m_flPushSpeed);
	CNetworkVector(m_vecPushDir);
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ##################################################################################
//	>> TriggerTeleport
//
//  Purpose: A trigger that teleports things
//
// ##################################################################################
class CTriggerTeleport : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerTeleport, CBaseTrigger);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Touch(CBaseEntity *pOther);

	// Vars
	CNetworkVar(unsigned int, m_iLandmarkCRC);
#ifdef GAME_DLL // Server specific things
public:
	virtual void Spawn(void);

	string_t m_iLandmark;
	DECLARE_DATADESC();
#endif
};

#endif