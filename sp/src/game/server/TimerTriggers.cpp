#include "cbase.h"
#include "triggers.h"
#include "Timer.h"
#include "hl2_player.h"



#include "memdbgon.h"


class CTriggerStart : public CTriggerMultiple {

	DECLARE_CLASS(CTriggerStart, CTriggerMultiple);
public:
	void EndTouch(CBaseEntity*);
};

LINK_ENTITY_TO_CLASS(trigger_start, CTriggerStart);


void CTriggerStart::EndTouch(CBaseEntity* other) {
	BaseClass::EndTouch(other);
	CTimer::timer()->Start(gpGlobals->tickcount);
}



class CTriggerEnd : public CTriggerMultiple {

	DECLARE_CLASS(CTriggerEnd, CTriggerMultiple);

public:
	void StartTouch(CBaseEntity*);


};

LINK_ENTITY_TO_CLASS(trigger_end, CTriggerEnd);

void CTriggerEnd::StartTouch(CBaseEntity* ent) {
	BaseClass::EndTouch(ent);
	CTimer::timer()->Stop(); 
}


class CTriggerCheckpoint : public CTriggerMultiple {

	DECLARE_CLASS(CTriggerCheckpoint, CTriggerMultiple);
	DECLARE_DATADESC();
public:
	void StartTouch(CBaseEntity*);
	int getCheckpointNumber();

private:
	int checkpointNumber = 0;




};

BEGIN_DATADESC(CTriggerCheckpoint)

	DEFINE_KEYFIELD(checkpointNumber, FIELD_INTEGER, "number")

END_DATADESC()



LINK_ENTITY_TO_CLASS(trigger_checkpoint, CTriggerCheckpoint);

void CTriggerCheckpoint::StartTouch(CBaseEntity* ent) {
	BaseClass::StartTouch(ent);
	((CHL2_Player*)ent)->SetCurrentCheckpoint(checkpointNumber);
}