#include "bsplib.h"
#include "cmdlib.h"
#include "filesystem.h"
#include "keyvalues.h"
#include "utlbuffer.h"
#include "triggers.h"

char m_TriggersName[triggers_max][64] = {"trigger_momentum_timer_start", "trigger_momentum_timer_stop",
                                         "trigger_momentum_onehop",      "trigger_momentum_resetonehop",
                                         "trigger_momentum_checkpoint",  "trigger_momentum_teleport_checkpoint",
                                         "trigger_momentum_multihop",    "trigger_momentum_timer_stage"};

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