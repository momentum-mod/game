#pragma once

enum RunSafeguardType_t
{
    RUN_SAFEGUARD_PRACTICEMODE = 0,
    RUN_SAFEGUARD_RESTART,
    RUN_SAFEGUARD_RESTART_STAGE,
    RUN_SAFEGUARD_SAVELOC_TELE,
    RUN_SAFEGUARD_CHAT_OPEN,

    RUN_SAFEGUARD_COUNT,

    RUN_SAFEGUARD_INVALID = -1
};

enum RunSafeguardMode_t
{
    RUN_SAFEGUARD_MODE_NONE = 0,
    RUN_SAFEGUARD_MODE_MOVEMENTKEYS,
    RUN_SAFEGUARD_MODE_DOUBLEPRESS,

    RUN_SAFEGUARD_MODE_COUNT,

    RUN_SAFEGUARD_MODE_INVALID = -1
};

class CRunSafeguard
{
  public:
    CRunSafeguard(const char *szAction);

    bool IsSafeguarded(RunSafeguardMode_t mode);

  private:
    bool IsMovementKeysSafeguarded(int nButtons);
    bool IsDoublePressSafeguarded();

    char m_szAction[64];

    float m_flLastTimePressed;

    bool m_bDoublePressSafeguard;
};

class MomRunSafeguards
{
  public:
    MomRunSafeguards();

    bool IsSafeguarded(RunSafeguardType_t type);
    bool IsSafeguarded(RunSafeguardType_t type, RunSafeguardMode_t mode);

    RunSafeguardMode_t GetModeFromType(int type);
    RunSafeguardMode_t GetModeFromType(RunSafeguardType_t type);

    void SetMode(int type, int mode);
    void SetMode(RunSafeguardType_t type, RunSafeguardMode_t mode);

  private:
    CRunSafeguard *m_pSafeguards[RUN_SAFEGUARD_COUNT];
};

extern MomRunSafeguards *g_pRunSafeguards;
