#include "triggers.h"

char m_TriggersName[triggers_max][64] = {"trigger_momentum_timer_start", "trigger_momentum_timer_stop",
                                         "trigger_momentum_onehop",      "trigger_momentum_resetonehop",
                                         "trigger_momentum_checkpoint",  "trigger_momentum_teleport_checkpoint",
                                         "trigger_momentum_multihop",    "trigger_momentum_timer_stage"};

char m_ZoneFileTriggersName[triggers_max][64] = {
    "start", "end", "onehop", "resetonehop", "checkpoint", "checkpoint_teleport", "multihop", "stage"};

// Get the trigger type depending on the trigger name.
int GetTriggerType(const char *classname)
{
    for (auto i = 0; i != triggers_max; i++)
    {
        if (!strcmp(classname, m_TriggersName[i]))
            return i;
    }

    return -1;
}

// Get the trigger type depending on the zone name.
int GetZoneFileTriggerType(const char *zonename)
{
    for (auto i = 0; i != triggers_max; i++)
    {
        if (!strcmp(zonename, m_ZoneFileTriggersName[i]))
            return i;
    }

    return -1;
}

// This is used for field the basis of the triggers, its position, angles and mins maxs collisions data.
void ProcessBaseTriggerSubKey(KeyValues *subKey, CBaseMomentumTrigger *BaseTrigger)
{
    subKey->SetFloat("xPos", BaseTrigger->m_vecPos.x);
    subKey->SetFloat("yPos", BaseTrigger->m_vecPos.y);
    subKey->SetFloat("zPos", BaseTrigger->m_vecPos.z);
    // Angles are not even supported in collision for triggers I think, we might remove it.
    subKey->SetFloat("xRot", 0.0f);
    subKey->SetFloat("yRot", 0.0f);
    subKey->SetFloat("zRot", 0.0f);
    subKey->SetFloat("xScaleMins", BaseTrigger->m_vecMins.x);
    subKey->SetFloat("yScaleMins", BaseTrigger->m_vecMins.y);
    subKey->SetFloat("zScaleMins", BaseTrigger->m_vecMins.z);
    subKey->SetFloat("xScaleMaxs", BaseTrigger->m_vecMaxs.x);
    subKey->SetFloat("yScaleMaxs", BaseTrigger->m_vecMaxs.y);
    subKey->SetFloat("zScaleMaxs", BaseTrigger->m_vecMaxs.z);
}

// This is used for field the basis of the triggers, its position, angles and mins maxs collisions data.
void ProcessSubKeyBaseTrigger(KeyValues *subKey, CBaseMomentumTrigger *BaseTrigger)
{
    BaseTrigger->m_vecPos.x = subKey->GetFloat("xPos");
    BaseTrigger->m_vecPos.y = subKey->GetFloat("yPos");
    BaseTrigger->m_vecPos.z = subKey->GetFloat("zPos");
    BaseTrigger->m_vecMins.x = subKey->GetFloat("xScaleMins");
    BaseTrigger->m_vecMins.y = subKey->GetFloat("yScaleMins");
    BaseTrigger->m_vecMins.z = subKey->GetFloat("zScaleMins");
    BaseTrigger->m_vecMaxs.x = subKey->GetFloat("xScaleMaxs");
    BaseTrigger->m_vecMaxs.y = subKey->GetFloat("yScaleMaxs");
    BaseTrigger->m_vecMaxs.z = subKey->GetFloat("zScaleMaxs");
}