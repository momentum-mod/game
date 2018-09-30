#ifndef _C_MOM_TRIGGERS_H_
#define _C_MOM_TRIGGERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "prediction.h"

#define MAX_TRIGGER_NAME 128
//
// Spawnflags
//

enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger EXCEPT DEBRIS!
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects
	SF_TRIGGER_ONLY_NPCS_IN_VEHICLES		= 0X800,	// *if* NPCs can fire this trigger, only NPCs in vehicles do so (respects player ally flag too)
	SF_TRIGGER_DISALLOW_BOTS                = 0x1000,   // Bots are not allowed to fire this trigger
};

class C_BaseMomentumTrigger : public C_BaseEntity
{
    DECLARE_CLASS(C_BaseMomentumTrigger, C_BaseEntity);
	DECLARE_CLIENTCLASS();

  public:
    C_BaseMomentumTrigger(){};

    int GetSpawnFlags(void) const;
    void AddSpawnFlags(int nFlags);
    void RemoveSpawnFlags(int nFlags);
    void ClearSpawnFlags(void);
    bool HasSpawnFlags(int nFlags) const;

    bool PointIsWithin(const Vector &vecPoint);
    bool PassesTriggerFilters(CBaseEntity *pOther);

    CNetworkVar(int, m_iSpawnFlags);
    CNetworkString(m_iszTarget, MAX_POINT_NAME);
    CNetworkString(m_iszModel, MAX_TRIGGER_NAME);

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
};

class C_PointEntity : public C_BaseEntity
{
  public:
    DECLARE_CLASS(C_PointEntity, C_BaseEntity);
    DECLARE_CLIENTCLASS();

    void Spawn() OVERRIDE;
};

#endif