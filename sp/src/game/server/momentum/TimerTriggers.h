#include "cbase.h"
#include "triggers.h"
#include "Timer.h"
#include "hl2_player.h"


class CTriggerStart : public CTriggerMultiple {

	DECLARE_CLASS(CTriggerStart, CTriggerMultiple);
public:
	void EndTouch(CBaseEntity*);
};

LINK_ENTITY_TO_CLASS(trigger_start, CTriggerStart);


class CTriggerEnd : public CTriggerMultiple {

	DECLARE_CLASS(CTriggerEnd, CTriggerMultiple);

public:
	void StartTouch(CBaseEntity*);


};

LINK_ENTITY_TO_CLASS(trigger_end, CTriggerEnd);


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


class TriggerCommands
{
public:
	static void ResetToStart();
	static void ResetToCheckpoint();
	static void SetStartTrigger(CTriggerStart* trigger);
	static void SetCheckpointTrigger(CTriggerCheckpoint* trigger);
};
CTriggerStart* startTrigger;
CTriggerCheckpoint* lastCheckpointTrigger;

static ConCommand mom_reset_to_start("mom_reset_to_start", TriggerCommands::ResetToStart);
static ConCommand mom_reset_to_checkpoint("mom_reset_to_checkpoint", TriggerCommands::ResetToCheckpoint);