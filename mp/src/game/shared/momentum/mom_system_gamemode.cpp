#include "cbase.h"

#include "mom_system_gamemode.h"
#include "movevars_shared.h"
#include "mom_player_shared.h"
#include "fmtstr.h"

#ifdef GAME_DLL
#include "momentum/tickset.h"
#endif

#include "tier0/memdbgon.h"

#ifdef GAME_DLL
CON_COMMAND(mom_print_gamemode_vars, "Prints out the currently set values for commands like sv_maxvelocity, airaccel, etc")
{
    g_pGameModeSystem->PrintGameModeVars();
}

static void OnGamemodeChanged(IConVar* var, const char* pOldValue, float fOldValue)
{
    ConVarRef gm(var);
    const auto gamemode = gm.GetInt();

    TickSet::SetTickrate(gamemode);
    // set the value of sv_interval_per_tick so it updates when gamemode changes the tickrate.
    ConVarRef tr("sv_interval_per_tick");
    tr.SetValue(TickSet::GetTickrate());

    g_pGameModeSystem->SetGameMode((GameMode_t)gamemode);
}

static ConVar mom_gamemode("mom_gamemode", "0", FCVAR_REPLICATED | FCVAR_NOT_CONNECTED | FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE,
                       "", true, 0, false, 0, OnGamemodeChanged);
#endif

void CGameModeBase::SetGameModeVars()
{
    // Default game mode vars
    sv_maxvelocity.SetValue(3500);
    sv_airaccelerate.SetValue(150);
    sv_accelerate.SetValue(5);
    sv_maxspeed.SetValue(260);
    sv_stopspeed.SetValue(75);
    sv_considered_on_ground.SetValue(1);
    sv_duck_collision_fix.SetValue(true);
    sv_ground_trigger_fix.SetValue(true);
}

void CGameModeBase::OnPlayerSpawn(CMomentumPlayer *pPlayer)
{
#ifdef GAME_DLL
    pPlayer->SetAutoBhopEnabled(PlayerHasAutoBhop());
#endif
}

void CGameModeBase::ExecGameModeCfg()
{
#ifdef CLIENT_DLL // Without this ifdef, the game fails to build
    if (GetGameModeCfg())
    {
        engine->ClientCmd_Unrestricted(CFmtStr("exec %s", GetGameModeCfg()));
    }
#endif
}

void CGameMode_Bhop::SetGameModeVars()
{
    CGameModeBase::SetGameModeVars();

    // Bhop-specific
    sv_maxvelocity.SetValue(100000);
    sv_airaccelerate.SetValue(1000);
}

void CGameMode_KZ::SetGameModeVars()
{
    CGameModeBase::SetGameModeVars();

    // KZ-specific
    sv_airaccelerate.SetValue(100);
    sv_maxspeed.SetValue(250);
}

void CGameMode_RJ::SetGameModeVars()
{
    CGameModeBase::SetGameModeVars();

    // RJ-specific
    sv_airaccelerate.SetValue(10);
    sv_accelerate.SetValue(10);
    sv_maxspeed.SetValue(240);
    sv_stopspeed.SetValue(100);
    sv_considered_on_ground.SetValue(2);
    sv_duck_collision_fix.SetValue(false);
    sv_ground_trigger_fix.SetValue(false); // MOM_TODO Remove when bounce triggers have been implemented
}

void CGameMode_RJ::OnPlayerSpawn(CMomentumPlayer *pPlayer)
{
#ifdef GAME_DLL
    pPlayer->GiveNamedItem("weapon_momentum_rocketlauncher");
    pPlayer->GiveNamedItem("weapon_momentum_shotgun");
#endif
}

void CGameMode_Tricksurf::SetGameModeVars()
{
    CGameModeBase::SetGameModeVars();

    // Tricksurf-specific
    sv_airaccelerate.SetValue(1000);
    sv_accelerate.SetValue(10);
}

CGameModeSystem::CGameModeSystem() : CAutoGameSystem("CGameModeSystem")
{
    m_pCurrentGameMode = new CGameModeBase; // Unknown game mode
    m_vecGameModes.AddToTail(m_pCurrentGameMode);
    m_vecGameModes.AddToTail(new CGameMode_Surf);
    m_vecGameModes.AddToTail(new CGameMode_Bhop);
    m_vecGameModes.AddToTail(new CGameMode_KZ);
    m_vecGameModes.AddToTail(new CGameMode_RJ);
    m_vecGameModes.AddToTail(new CGameMode_Tricksurf);
    m_vecGameModes.AddToTail(new CGameMode_Trikz);
}

CGameModeSystem::~CGameModeSystem()
{
    m_vecGameModes.PurgeAndDeleteElements();
}

void CGameModeSystem::LevelInitPostEntity()
{
#ifdef GAME_DLL
    m_pCurrentGameMode->SetGameModeVars();
    PrintGameModeVars();
#else // CLIENT_DLL
    m_pCurrentGameMode->ExecGameModeCfg();
#endif
}

void CGameModeSystem::SetGameMode(GameMode_t eMode)
{
    m_pCurrentGameMode = m_vecGameModes[eMode];
#ifdef CLIENT_DLL
    static ConVarRef mom_gamemode("mom_gamemode");
    // Throw the change to the server too
    mom_gamemode.SetValue(m_pCurrentGameMode->GetType());
#endif
}

void CGameModeSystem::SetGameModeFromMapName(const char *pMapName)
{
    // Set to unknown for now
    m_pCurrentGameMode = m_vecGameModes[0];

    if (pMapName)
    {
        // Skip over unknown in the loop
        for (auto i = 1; i < m_vecGameModes.Count(); ++i)
        {
            const auto pPrefix = m_vecGameModes[i]->GetMapPrefix();
            const auto strLen = Q_strlen(pPrefix);
            if (!Q_strnicmp(pPrefix, pMapName, strLen))
            {
                m_pCurrentGameMode = m_vecGameModes[i];
                break;
            }
        }
    }

#ifdef CLIENT_DLL
    static ConVarRef mom_gamemode("mom_gamemode");
    // Throw the change to the server too
    mom_gamemode.SetValue(m_pCurrentGameMode->GetType());
#endif
}

void CGameModeSystem::PrintGameModeVars()
{
    Msg("Set game mode ConVars:\n\n"
        "sv_maxvelocity: %i\n"
        "sv_airaccelerate: %i\n"
        "sv_accelerate: %i\n"
        "sv_maxspeed: %i\n"
        "sv_gravity: %i\n"
        "sv_friction: %i\n",
        sv_maxvelocity.GetInt(),
        sv_airaccelerate.GetInt(),
        sv_accelerate.GetInt(),
        sv_maxspeed.GetInt(),
        sv_gravity.GetInt(),
        sv_friction.GetInt());
}

// Expose to DLL
CGameModeSystem s_GameModeSys;
CGameModeSystem *g_pGameModeSystem = &s_GameModeSys;