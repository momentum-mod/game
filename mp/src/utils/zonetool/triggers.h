#pragma once
#include "string_t.h"

#define DO_ONCE_SCOPE(Code)                                                                                            \
    \
{                                                                                                                 \
        \
static bool bCodeRun = false;                                                                                          \
        \
if(!bCodeRun)                                                                                                          \
        \
{                                                                                                             \
            Code;                                                                                                      \
            bCodeRun = true;                                                                                           \
        \
}                                                                                                             \
    \
}

#define DO_ONCE(Code)                                                                                                  \
    \
static bool bCodeRun = false;                                                                                          \
    \
if(!bCodeRun)                                                                                                          \
    \
{                                                                                                                 \
        Code;                                                                                                          \
        bCodeRun = true;                                                                                               \
    \
}

enum
{
    SF_LIMIT_LEAVE_SPEED = 0x0001,             // Limit speed if player bhopped in start zone?
    SF_USE_LOOKANGLES = 0x0002,                // Use look angles?
                                               // CTriggerOneHop
    SF_TELEPORT_RESET_ONEHOP = 0x0010,         // Reset hop state if player hops onto another different onehop
                                               // CTriggerLimitMove
    LIMIT_JUMP = 0x0020,                       // prevent player from jumping
    LIMIT_CROUCH = 0x0040,                     // prevent player from croching
    LIMIT_BHOP = 0x0080,                       // prevent player from bhopping
                                               // CFuncShootBost and CTriggerMomentumPush
    SF_PUSH_DIRECTION_AS_FINAL_FORCE = 0x0100, // Use the direction vector as final force instead of calculating it by
                                               // force amount CTriggerMomentumPush
    SF_PUSH_ONETOUCH = 0x0200,                 // Only allow for one touch
    SF_PUSH_ONSTART = 0x0400,                  // Modify player velocity on StartTouch
    SF_PUSH_ONEND = 0x0800,                    // Modify player velocity on EndTouch
                                               // CTriggerTeleport
    SF_TELE_ONEXIT = 0x1000,                   // Teleport the player on EndTouch instead of StartTouch
};

// All triggers used in the zon file.
enum e_Triggers
{
    trigger_momentum_timer_start,
    trigger_momentum_timer_stop,
    trigger_momentum_onehop,
    trigger_momentum_resetonehop,
    trigger_momentum_checkpoint,
    trigger_momentum_teleport_checkpoint,
    trigger_momentum_multihop,
    trigger_momentum_timer_stage,
    triggers_max
};

// All triggers have a position, mins and maxs.
class CBaseMomentumTrigger
{
  public:
    Vector m_vecPos = Vector(0, 0, 0), m_vecMins = Vector(0, 0, 0), m_vecMaxs = Vector(0, 0, 0);
    int m_spawnflags = 0;
};

// All those triggers should be updated in case we changed them later.
// For reference see mapzones.cpp and mom_triggers.cpp/h in server project.
class CTriggerMomentumTimerStart
{
  public:
    QAngle m_lookangles = QAngle(0, 0, 0);
    float m_bhopleavespeed = 0.0f;
    bool m_StartOnJump = false;
    int m_LimitSpeedType = 0;
    int m_ZoneNumber = 0;
    CBaseMomentumTrigger Base;
};

//
class CTriggerMomentumTimerStop
{
  public:
    int m_ZoneNumber = 0;
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumResetOneHop
{
  public:
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumOneHop
{
  public:
    float m_hold = 0.0f;
    bool m_resetang = false;
    bool m_stop = false;
    string_t m_target;
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumTeleportCheckPoint
{
  public:
    bool m_resetang = false;
    bool m_stop = false;
    string_t m_target;
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumMultiHop
{
  public:
    float m_hold = 0.0f;
    bool m_resetang = false;
    bool m_stop = false;
    string_t m_target;
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumTimerStage
{
  public:
    int m_stage = 0;
    CBaseMomentumTrigger Base;
};

class CTriggerMomentumCheckPoint
{
  public:
    int m_checkpoint = 0;
    CBaseMomentumTrigger Base;
};

class CBSPTriggers
{
  public:
    CCopyableUtlVector<CTriggerMomentumTimerStart> m_TriggersMomentumTimerStart;
    CCopyableUtlVector<CTriggerMomentumTimerStop> m_TriggersMomentumTimerStop;
    CCopyableUtlVector<CTriggerMomentumOneHop> m_TriggersMomentumOneHop;
    CCopyableUtlVector<CTriggerMomentumResetOneHop> m_TriggersMomentumResetOneHop;
    CCopyableUtlVector<CTriggerMomentumTeleportCheckPoint> m_TriggersMomentumTeleportCheckPoint;
    CCopyableUtlVector<CTriggerMomentumMultiHop> m_TriggersMomentumMultiHop;
    CCopyableUtlVector<CTriggerMomentumTimerStage> m_TriggersMomentumTimerStage;
    CCopyableUtlVector<CTriggerMomentumCheckPoint> m_TriggersMomentumCheckPoint;
};

extern int GetTriggerType(const char *classname);
extern char m_TriggersName[triggers_max][64];