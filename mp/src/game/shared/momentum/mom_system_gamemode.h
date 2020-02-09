#pragma once

#include "mom_shareddefs.h"

#ifdef CLIENT_DLL
#define CMomentumPlayer C_MomentumPlayer
#endif

class CMomentumPlayer;

abstract_class IGameMode
{
public:
    virtual GameMode_t  GetType() = 0;
    virtual const char* GetStatusString() = 0;
    virtual const char* GetDiscordIcon() = 0;
    virtual const char* GetMapPrefix() = 0;
    virtual const char* GetGameModeCfg() = 0;

    virtual void        SetGameModeVars() = 0;
    virtual bool        PlayerHasAutoBhop() = 0;
    virtual void        OnPlayerSpawn(CMomentumPlayer *pPlayer) = 0;
    virtual void        ExecGameModeCfg() = 0;
    virtual float       GetIntervalPerTick() = 0;

    // Movement vars
    virtual float       GetViewScale() = 0;

    virtual ~IGameMode() {}
};

// Unknown ("default") game mode
class CGameModeBase : public IGameMode
{
public:
    GameMode_t GetType() override { return GAMEMODE_UNKNOWN; }
    const char* GetStatusString() override { return "Playing"; }
    const char* GetDiscordIcon() override { return "mom"; }
    const char* GetMapPrefix() override { return ""; }
    const char* GetGameModeCfg() override { return nullptr; }
    float GetIntervalPerTick() override { return 0.015f; }
    float GetViewScale() override { return 0.5f; }

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return true; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    void ExecGameModeCfg() override;
};

class CGameMode_Surf : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_SURF; }
    const char* GetStatusString() override { return "Surfing"; }
    const char* GetDiscordIcon() override { return "mom_icon_surf"; }
    const char* GetMapPrefix() override { return "surf_"; }
    const char* GetGameModeCfg() override { return "surf.cfg"; }
};

class CGameMode_Bhop : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_BHOP; }
    const char* GetStatusString() override { return "Bhopping"; }
    const char* GetDiscordIcon() override { return "mom_icon_bhop"; }
    const char* GetMapPrefix() override { return "bhop_"; }
    const char* GetGameModeCfg() override { return "bhop.cfg"; }
    float GetIntervalPerTick() override { return 0.01f; }
    void SetGameModeVars() override;
};

class CGameMode_KZ : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_KZ; }
    const char* GetStatusString() override { return "Climbing"; }
    const char* GetDiscordIcon() override { return "mom_icon_kz"; }
    const char* GetMapPrefix() override { return "kz_"; }
    const char* GetGameModeCfg() override { return "kz.cfg"; }
    float GetIntervalPerTick() override { return 0.01f; }
    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
};

class CGameMode_RJ : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_RJ; }
    const char* GetStatusString() override { return "Rocket Jumping"; }
    const char* GetDiscordIcon() override { return "mom_icon_rj"; }
    const char* GetMapPrefix() override { return "jump_"; }
    const char* GetGameModeCfg() override { return "rj.cfg"; }
    float GetViewScale() override { return 1.0f; }

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
};

class CGameMode_SJ : public CGameModeBase
{
  public:
    GameMode_t GetType() override { return GAMEMODE_SJ; }
    const char *GetStatusString() override { return "Sticky Jumping"; }
    const char *GetDiscordIcon() override { return "mom_icon_sj"; }
    const char *GetMapPrefix() override { return "sj_"; }
    const char *GetGameModeCfg() override { return "sj.cfg"; }
    float GetViewScale() override { return 1.0f; }

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
};

class CGameMode_Tricksurf : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_TRICKSURF; }
    const char* GetStatusString() override { return "Surfing"; }
    const char* GetDiscordIcon() override { return "mom_icon_tricksurf"; }
    const char* GetMapPrefix() override { return "tricksurf_"; }
    const char* GetGameModeCfg() override { return "tricksurf.cfg"; }
    float GetIntervalPerTick() override { return 0.01f; }
    void SetGameModeVars() override;
};

// MOM_TODO
class CGameMode_Trikz : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_TRIKZ; }
    const char* GetDiscordIcon() override { return "mom_icon_trikz"; }
    const char* GetMapPrefix() override { return "trikz_"; }
    const char* GetGameModeCfg() override { return "trikz.cfg"; }
};

class CGameModeSystem : public CAutoGameSystem
{
public:
    CGameModeSystem();
    ~CGameModeSystem();

    // IGameSystem overrides
    void LevelInitPostEntity() override;

    // Extra methods
    /// Gets current game mode
    IGameMode *GetGameMode() const { return m_pCurrentGameMode; }
    /// Gets a specific game mode instance by type
    IGameMode *GetGameMode(int eMode) const;
    /// Checks if the game mode is the given one.
    /// (convenience method; functionally equivalent to `GetGameMode()->GetType() == eCheck`)
    bool GameModeIs(GameMode_t eCheck) const { return m_pCurrentGameMode->GetType() == eCheck; }
    /// Another convenience method to check if the current game mode is a TF2-based one (RJ || SJ)
    bool IsTF2BasedMode() const { return GameModeIs(GAMEMODE_RJ) || GameModeIs(GAMEMODE_SJ); }
    /// Sets the game mode directly
    void SetGameMode(GameMode_t eMode);
    /// Sets the game mode from a map name (backup method)
    void SetGameModeFromMapName(const char *pMapName);
    /// Prints out the game mode's vars
    void PrintGameModeVars();

private:
    IGameMode *m_pCurrentGameMode;
    CUtlVector<IGameMode*> m_vecGameModes;
};

extern CGameModeSystem *g_pGameModeSystem;