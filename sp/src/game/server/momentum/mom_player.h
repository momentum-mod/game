
#ifndef MOMPLAYER_H
#define MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "player.h"


class CMomentumPlayer : public CBasePlayer
{
public:

    DECLARE_CLASS(CMomentumPlayer, CBasePlayer);

    CMomentumPlayer();
    ~CMomentumPlayer(void);

    static CMomentumPlayer *CreatePlayer(const char *className, edict_t *ed)
    {
        CMomentumPlayer::s_PlayerEdict = ed;
        return (CMomentumPlayer*) CreateEntityByName(className);
    }

    DECLARE_SERVERCLASS();
    DECLARE_DATADESC();

    int FlashlightIsOn() { return IsEffectActive(EF_DIMLIGHT); }

    void FlashlightTurnOn()
    { 
        AddEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOn");//MOM_TODO: change this?
    }

    void FlashlightTurnOff()
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOff");//MOM_TODO: change this?
    }


    // LADDERS
    void SurpressLadderChecks(const Vector& pos, const Vector& normal);
    bool CanGrabLadder(const Vector& pos, const Vector& normal);
    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    //SPAWNING
    CBaseEntity* EntSelectSpawnPoint();

private:
    CountdownTimer m_ladderSurpressionTimer;
    Vector m_lastLadderNormal;
    Vector m_lastLadderPos;
    EHANDLE g_pLastSpawn;
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity* &pSpot);

    friend class CMomentumGameMovement;

};
#endif //MOMPLAYER_H