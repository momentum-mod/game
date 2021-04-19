#pragma once

#include "mom_shareddefs.h"

#ifdef CLIENT_DLL
#define CMomentumPlayer C_MomentumPlayer
#endif

class CMomentumPlayer;

enum class GameModeHUDCapability_t
{
    CAP_HUD_SYNC = 0,
    CAP_HUD_SYNC_BAR,
    CAP_HUD_KEYPRESS_STRAFES,
    CAP_HUD_KEYPRESS_JUMPS,
    CAP_HUD_KEYPRESS_ATTACK,
    CAP_HUD_KEYPRESS_WALK,
    CAP_HUD_KEYPRESS_SPRINT,
};

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
    virtual bool        WeaponIsAllowed(WeaponID_t weapon) = 0;
    virtual bool        HasCapability(GameModeHUDCapability_t capability) = 0;

    // Movement vars
    virtual float       GetViewScale() = 0;
    virtual bool        CanBhop() = 0;
    virtual float       GetJumpFactor() = 0;

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
    bool CanBhop() override { return true; }
    float GetJumpFactor() override;

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return true; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    void ExecGameModeCfg() override;
    bool WeaponIsAllowed(WeaponID_t weapon) override { return true; } // Unknown allows all weapons
    bool HasCapability(GameModeHUDCapability_t capability) override;
};

class CGameMode_Surf : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_SURF; }
    const char* GetStatusString() override { return "Surfing"; }
    const char* GetDiscordIcon() override { return "mom_icon_surf"; }
    const char* GetMapPrefix() override { return "surf_"; }
    const char* GetGameModeCfg() override { return "surf.cfg"; }
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;
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
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;
};

class CGameMode_KZ : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_KZ; }
    const char* GetStatusString() override { return "Climbing"; }
    const char* GetDiscordIcon() override { return "mom_icon_kz"; }
    const char* GetMapPrefix() override { return "kz_"; }
    const char* GetGameModeCfg() override { return "kz.cfg"; }
    float GetIntervalPerTick() override { return 0.0078125f; }
    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;
};

class CGameMode_RJ : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_RJ; }
    const char* GetStatusString() override { return "Rocket Jumping"; }
    const char* GetDiscordIcon() override { return "mom_icon_rj"; }
    const char* GetMapPrefix() override { return "rj_"; }
    const char* GetGameModeCfg() override { return "rj.cfg"; }
    float GetViewScale() override { return 1.0f; }
    float GetJumpFactor() override;
    bool CanBhop() override { return false; }

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;
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
    float GetJumpFactor() override;
    bool CanBhop() override { return false; }
    bool HasCapability(GameModeHUDCapability_t capability) override;

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return false; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
};

class CGameMode_Tricksurf : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_TRICKSURF; }
    const char* GetStatusString() override { return "Tricksurfing"; }
    const char* GetDiscordIcon() override { return "mom_icon_tricksurf"; }
    const char* GetMapPrefix() override { return "tsurf_"; }
    const char* GetGameModeCfg() override { return "tsurf.cfg"; }
    float GetIntervalPerTick() override { return 0.01f; }
    void SetGameModeVars() override;

    bool HasCapability(GameModeHUDCapability_t capability) override;
};

// Ahop-specific defines
#define AHOP_WALK_SPEED   150.0f
#define AHOP_NORM_SPEED   190.0f
#define AHOP_SPRINT_SPEED 320.0f

class CGameMode_Ahop : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_AHOP; }
    const char *GetStatusString() override { return "Accelerated hopping"; }
    const char *GetDiscordIcon() override { return "mom_icon_ahop"; }
    const char *GetMapPrefix() override { return "ahop_"; }
    const char *GetGameModeCfg() override { return "ahop.cfg"; }
    void SetGameModeVars() override;
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;

    float GetViewScale() override { return 1.0f; }
    float GetJumpFactor() override;
};

// Parkour-specific defines
#define PK_NORM_SPEED       162.5f
#define PK_SPRINT_SPEED     242.5f

#define PK_POWERSLIDE_MIN_SPEED (PK_NORM_SPEED + 5.0f) // must be going faster than this to powerslide
#define PK_WALLRUN_MAX_Z        20.0f
#define PK_WALLRUN_MIN_Z        -50.0f

#define PK_SLIDE_TIME           2000.0f // in ms
#define PK_SLIDE_SPEED_BOOST    75.0f
#define PK_WALLRUN_TIME         2000.0f
#define PK_WALLRUN_SPEED        300.0f
#define PK_WALLRUN_BOOST        60.0f

#define PK_CORNER_ESC_SPEED 80.0f
#define PK_WALLRUN_OUT_TIME 500.0f // start easing out of the wallrun for last 500 ms

#define PK_WALLRUN_PLANE_MAX_Z 0.5

class CGameMode_Parkour : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_PARKOUR; }
    const char *GetStatusString() override { return "Parkouring"; }
    const char *GetDiscordIcon() override { return "mom_icon_parkour"; }
    const char *GetMapPrefix() override { return "pk_"; }
    const char *GetGameModeCfg() override { return "pk.cfg"; }
    void SetGameModeVars() override;
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;

    float GetViewScale() override { return 1.0f; }
    float GetJumpFactor() override { return 300.0f; } // sqrt( 60 * 2 * 750 )
};

class CGameMode_Conc : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_CONC; }
    const char *GetStatusString() override { return "Concussion Grenade Jumping"; }
    const char *GetDiscordIcon() override { return "mom_icon_conc"; }
    const char *GetMapPrefix() override { return "conc_"; }
    const char *GetGameModeCfg() override { return "conc.cfg"; }
    float GetViewScale() override { return 1.0f; }
    bool CanBhop() override { return true; }
    float GetIntervalPerTick() override { return 0.01f; }

    float GetJumpFactor() override { return 268.6261f; } // sqrt(2 * 800.0f * 45.1f)

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return true; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
};

class CGameMode_Defrag : public CGameModeBase
{
public:
    GameMode_t GetType() override { return GAMEMODE_DEFRAG; }
    const char *GetStatusString() override { return "Defragging"; }
    const char *GetDiscordIcon() override { return "mom_icon_dfrag"; }
    const char *GetMapPrefix() override { return "df_"; }
    const char *GetGameModeCfg() override { return "df.cfg"; }
    float GetViewScale() override { return 1.0f; }
    bool CanBhop() override { return true; }

    float GetIntervalPerTick() override { return 0.01f; }

    void SetGameModeVars() override;
    bool PlayerHasAutoBhop() override { return true; }
    void OnPlayerSpawn(CMomentumPlayer *pPlayer) override;
    bool WeaponIsAllowed(WeaponID_t weapon) override;
    bool HasCapability(GameModeHUDCapability_t capability) override;
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
    /// Gets a game mode based off of the map name
    IGameMode *GetGameModeFromMapName(const char *pMapName);
    /// Checks if the game mode is the given one.
    /// (convenience method; functionally equivalent to `GetGameMode()->GetType() == eCheck`)
    bool GameModeIs(GameMode_t eCheck) const { return m_pCurrentGameMode->GetType() == eCheck; }
    /// Another convenience method to check if the current game mode is a TF2-based one (RJ || SJ)
    bool IsTF2BasedMode() const { return GameModeIs(GAMEMODE_RJ) || GameModeIs(GAMEMODE_SJ); }
    /// Another convenience method to check if the current game mode is a CS-based one (Surf || Bhop || KZ || Unknown)
    bool IsCSBasedMode() const { return GameModeIs(GAMEMODE_SURF) || GameModeIs(GAMEMODE_BHOP) ||
                                        GameModeIs(GAMEMODE_KZ) || GameModeIs(GAMEMODE_UNKNOWN); }
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