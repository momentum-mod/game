#pragma once

enum RunSafeguardType_t
{
    RUN_SAFEGUARD_PRACTICEMODE = 0,
    RUN_SAFEGUARD_RESTART,
    RUN_SAFEGUARD_RESTART_STAGE,
    RUN_SAFEGUARD_SAVELOC_TELE,
    RUN_SAFEGUARD_CHAT_OPEN,
    RUN_SAFEGUARD_MAP_CHANGE,
    RUN_SAFEGUARD_QUIT_TO_MENU,
    RUN_SAFEGUARD_QUIT_GAME,

    RUN_SAFEGUARD_COUNT,

    RUN_SAFEGUARD_INVALID = -1
};

enum RunSafeguardMode_t
{
    RUN_SAFEGUARD_MODE_NONE = 0,
    RUN_SAFEGUARD_MODE_MOVEMENTKEYS,
    RUN_SAFEGUARD_MODE_DOUBLEPRESS,
    RUN_SAFEGUARD_MODE_POPUP_CONFIRM,

    RUN_SAFEGUARD_MODE_COUNT,

    RUN_SAFEGUARD_MODE_INVALID = -1
};

class CRunSafeguard
{
  public:
    CRunSafeguard(const char *szAction);

    void Reset();

    bool IsSafeguarded(RunSafeguardMode_t mode);

    void SetRelevantCVar(ConVar *pVarRef) { m_pRelatedVar = pVarRef; }
    void SetIgnoredInMenu(bool bIgnoredInMenu) { m_bIgnoredInMenu = bIgnoredInMenu; }
    bool IsIgnoredInMenu() const { return m_bIgnoredInMenu; }

  private:
    bool IsMovementKeysSafeguarded(int nButtons, CBasePlayer *pPlayer);
    bool IsDoublePressSafeguarded(CBasePlayer *pPlayer);

    char m_szAction[64];

    float m_flLastTimePressed;
    float m_flLastTimeWarned;

    bool m_bIgnoredInMenu;

    ConVar *m_pRelatedVar;
};

class MomRunSafeguards
{
  public:
    MomRunSafeguards();

    void ResetAllSafeguards();
    void ResetSafeguard(int type);
    void ResetSafeguard(RunSafeguardType_t type);

    bool IsSafeguarded(RunSafeguardType_t type);
    bool IsSafeguarded(RunSafeguardType_t type, RunSafeguardMode_t mode);

    RunSafeguardMode_t GetModeFromType(int type);
    RunSafeguardMode_t GetModeFromType(RunSafeguardType_t type);

    void SetMode(int type, int mode);
    void SetMode(RunSafeguardType_t type, RunSafeguardMode_t mode);

#ifdef GAME_DLL
    void OnGameUIToggled(KeyValues *pKv);
    bool IsGameUIActive() const { return m_bGameUIActive; }
#endif

  private:
    CRunSafeguard *m_pSafeguards[RUN_SAFEGUARD_COUNT];

#ifdef GAME_DLL
    bool m_bGameUIActive;
#endif
};

extern MomRunSafeguards *g_pRunSafeguards;