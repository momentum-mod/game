#include "cbase.h"
#include "TimerTriggers.h"
#include "tier0/memdbgon.h"

void CTriggerStart::EndTouch(CBaseEntity* other) {
	Msg("dbg: CTriggerStart::EndTouch");
	BaseClass::EndTouch(other);
	CTimer::timer()->Start(gpGlobals->tickcount);
	TriggerCommands::SetStartTrigger(this);
}

void CTriggerEnd::StartTouch(CBaseEntity* ent) {
	BaseClass::EndTouch(ent);
	CTimer::timer()->Stop(); 
}


void CTriggerCheckpoint::StartTouch(CBaseEntity* ent) {
	BaseClass::StartTouch(ent);
	((CHL2_Player*)ent)->SetCurrentCheckpoint(checkpointNumber);
	TriggerCommands::SetCheckpointTrigger(this);
}

void TriggerCommands::ResetToStart()
{
	if (startTrigger)
	{
		UTIL_SetOrigin(UTIL_GetLocalPlayer(), startTrigger->WorldSpaceCenter(), true);
	}
}

void TriggerCommands::ResetToCheckpoint()
{
	if (startTrigger)
	{
		UTIL_SetOrigin(UTIL_GetLocalPlayer(), lastCheckpointTrigger->WorldSpaceCenter(), true);
	}
}

void TriggerCommands::SetStartTrigger(CTriggerStart* trigger)
{
	startTrigger = trigger;
}

void TriggerCommands::SetCheckpointTrigger(CTriggerCheckpoint* trigger)
{
	lastCheckpointTrigger = trigger;
}

static void TestCreateTriggerStart(void)
{
	CTriggerStart *pTrigger = (CTriggerStart *)CreateEntityByName("trigger_start");
	if (pTrigger)
	{
		pTrigger->Spawn();
		pTrigger->SetAbsOrigin( UTIL_GetLocalPlayer()->GetAbsOrigin() );
		pTrigger->SetSize( Vector(-256, -256, -256), Vector(256, 256, 256) );
		pTrigger->SetSolid( SOLID_BBOX );
		// now use mom_reset_to_start
	}
}

static ConCommand mom_createstart("mom_createstart", TestCreateTriggerStart, "test");
