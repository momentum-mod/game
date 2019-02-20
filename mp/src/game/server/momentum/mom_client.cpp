#include "cbase.h"
#include "engine/IEngineSound.h"
#include "entitylist.h"
#include "game.h"
#include "gamerules.h"
#include "mom_player.h"
#include "momentum/mom_gamerules.h"
#include "physics.h"
#include "player_resource.h"
#include "teamplay_gamerules.h"
#include "util/os_utils.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!

#include "tier0/memdbgon.h"

void Host_Say(edict_t *pEdict, bool teamonly);

extern CBaseEntity *FindPickerEntityClass(CBasePlayer *pPlayer, char *classname);
extern bool g_fGameOver;

/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer(edict_t *pEdict, const char *playername)
{
    // Allocate a CBasePlayer for pev, and call spawn
    CMomentumPlayer *pPlayer = CMomentumPlayer::CreatePlayer("player", pEdict);
    pPlayer->SetPlayerName(playername);

    // Acquire client module's data recieve function
    pPlayer->StdDataToPlayer = (DataToPlayerFn)(GetProcAddress(GetModuleHandle(CLIENT_DLL_NAME), "StdDataToPlayer"));
}

void ClientActive(edict_t *pEdict, bool bLoadGame)
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(CBaseEntity::Instance(pEdict));
    Assert(pPlayer);

    if (!pPlayer)
    {
        return;
    }

    pPlayer->InitialSpawn();

    if (!bLoadGame)
    {
        pPlayer->Spawn();
    }
}

/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
    if (g_pGameRules) // this function may be called before the world has spawned, and the game rules initialized
        return g_pGameRules->GetGameDescription();
    else
        return "Momentum";
}

//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that
//			classname that the player is nearest facing
//
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity *FindEntity(edict_t *pEdict, char *classname)
{
    // If no name was given set bits based on the picked
    if (FStrEq(classname, ""))
    {
        return (FindPickerEntityClass(static_cast<CBasePlayer *>(GetContainingEntity(pEdict)), classname));
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void ClientGamePrecache(void)
{
    // MOM_TODO: Precache all of the mod-related sounds here

    /*CBaseEntity::PrecacheModel("models/player.mdl");
    CBaseEntity::PrecacheModel("models/gibs/agibs.mdl");
    CBaseEntity::PrecacheModel("models/weapons/v_hands.mdl");

    CBaseEntity::PrecacheScriptSound("HUDQuickInfo.LowAmmo");
    CBaseEntity::PrecacheScriptSound("HUDQuickInfo.LowHealth");

    CBaseEntity::PrecacheScriptSound("FX_AntlionImpact.ShellImpact");
    CBaseEntity::PrecacheScriptSound("Missile.ShotDown");
    CBaseEntity::PrecacheScriptSound("Bullets.DefaultNearmiss");
    CBaseEntity::PrecacheScriptSound("Bullets.GunshipNearmiss");
    CBaseEntity::PrecacheScriptSound("Bullets.StriderNearmiss");

    CBaseEntity::PrecacheScriptSound("Geiger.BeepHigh");
    CBaseEntity::PrecacheScriptSound("Geiger.BeepLow");*/
}

// called by ClientKill and DeadThink
void respawn(CBaseEntity *pEdict, bool fCopyCorpse) { pEdict->Spawn(); }

void GameStartFrame(void)
{
    VPROF("GameStartFrame()");
    if (g_fGameOver)
        return;

    gpGlobals->teamplay = (teamplay.GetInt() != 0);
}

//=========================================================
// instantiate the proper game rules object
//=========================================================
void InstallGameRules() { CreateGameRulesObject("CMomentumGameRules"); }

class CPointServerSettings : public CPointEntity
{
  public:
    DECLARE_CLASS(CPointServerSettings, CPointEntity);
    DECLARE_DATADESC();

    CPointServerSettings()
        : Gravity("sv_gravity"), Accelerate("sv_accelerate"), WaterAccelerate("sv_wateraccelerate"),
          AirAccelerate("sv_airaccelerate"), MaxVelocity("sv_maxvelocity"), FrictionType("sv_friction"),
          BackSpeed("sv_backspeed"), StopSpeed("sv_stopspeed"), StepSize("sv_stepsize"), WaterDist("sv_waterdist"),
          FootSteps("sv_footsteps"), SwimSound("sv_swimsound")
    {
        m_flGravity = atof(Gravity.GetDefault());
        m_flMaxVelocity = atof(MaxVelocity.GetDefault());
        m_flWaterAccelerate = atof(WaterAccelerate.GetDefault());
        m_flAirAccelerate = atof(AirAccelerate.GetDefault());
        m_flAccelerate = atof(Accelerate.GetDefault());
        m_iFrictionType = atoi(FrictionType.GetDefault());
        m_flBackSpeed = atof(BackSpeed.GetDefault());
        m_flStopSpeed = atof(StopSpeed.GetDefault());
        m_flStepSize = atof(StepSize.GetDefault());
        m_flWaterDist = atof(WaterDist.GetDefault());
        m_iFootSteps = atoi(FootSteps.GetDefault());
        m_iSwimSound = atoi(SwimSound.GetDefault());
    }

    void Spawn() OVERRIDE
    {
        BaseClass::Spawn();

        Gravity.SetValue(m_flGravity);
        MaxVelocity.SetValue(m_flMaxVelocity);
        Accelerate.SetValue(m_flAccelerate);
        WaterAccelerate.SetValue(m_flWaterAccelerate);
        AirAccelerate.SetValue(m_flAirAccelerate);
        FrictionType.SetValue(m_iFrictionType);
        BackSpeed.SetValue(m_flBackSpeed);
        StopSpeed.SetValue(m_flStopSpeed);
        StepSize.SetValue(m_flStepSize);
        WaterDist.SetValue(m_flWaterDist);
        FootSteps.SetValue(m_iFootSteps);
        SwimSound.SetValue(m_iSwimSound);
    }

  private:
    float m_flGravity;
    float m_flMaxVelocity;
    float m_flWaterAccelerate;
    float m_flAirAccelerate;
    float m_flAccelerate;
    int m_iFrictionType;
    float m_flBackSpeed;
    float m_flStopSpeed;
    float m_flStepSize;
    float m_flWaterDist;
    int m_iFootSteps;
    int m_iSwimSound;

    ConVarRef Gravity;
    ConVarRef MaxVelocity;
    ConVarRef Accelerate;
    ConVarRef WaterAccelerate;
    ConVarRef AirAccelerate;
    ConVarRef FrictionType;
    ConVarRef BackSpeed;
    ConVarRef StopSpeed;
    ConVarRef StepSize;
    ConVarRef WaterDist;
    ConVarRef FootSteps;
    ConVarRef SwimSound;
};

LINK_ENTITY_TO_CLASS(point_momentum_serversettings, CPointServerSettings);

BEGIN_DATADESC(CPointServerSettings)
    DEFINE_KEYFIELD(m_flGravity, FIELD_FLOAT, "Gravity"),
    DEFINE_KEYFIELD(m_flMaxVelocity, FIELD_FLOAT, "MaxVelocity"),
    DEFINE_KEYFIELD(m_flWaterAccelerate, FIELD_FLOAT, "WaterAccelerate"),
    DEFINE_KEYFIELD(m_flAirAccelerate, FIELD_FLOAT, "AirAccelerate"),
    DEFINE_KEYFIELD(m_flAccelerate, FIELD_FLOAT, "Accelerate"),
    DEFINE_KEYFIELD(m_iFrictionType, FIELD_INTEGER, "Friction"),
    DEFINE_KEYFIELD(m_flBackSpeed, FIELD_FLOAT, "BackSpeed"),
    DEFINE_KEYFIELD(m_flStopSpeed, FIELD_FLOAT, "StopSpeed"),
    DEFINE_KEYFIELD(m_flStepSize, FIELD_FLOAT, "StepSize"),
    DEFINE_KEYFIELD(m_flWaterDist, FIELD_FLOAT, "WaterDist"),
    DEFINE_KEYFIELD(m_iFootSteps, FIELD_INTEGER, "FootSteps"),
    DEFINE_KEYFIELD(m_iSwimSound, FIELD_INTEGER, "SwimSound"),
END_DATADESC()