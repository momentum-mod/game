#ifndef MOM_GAMERULES_H
#define MOM_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "gamerules.h"
#include "mom_player_shared.h"
#include "singleplay_gamerules.h"

#ifdef CLIENT_DLL
#define CMomentumGameRules C_MomentumGameRules
#else
#include "momentum/tickset.h"
#endif

class CMomentumGameRules : public CSingleplayRules
{
  public:
    DECLARE_CLASS(CMomentumGameRules, CSingleplayRules);

    CMomentumGameRules();
    ~CMomentumGameRules();

    const CViewVectors* GetViewVectors() const OVERRIDE;

    bool ShouldCollide(int collGroup1, int collGroup2) OVERRIDE;

#ifdef CLIENT_DLL

    DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else
    DECLARE_SERVERCLASS_NOBASE();

    // virtual void			Think(void);

    bool ClientCommand(CBaseEntity *pEdict, const CCommand &args) OVERRIDE;
    void PlayerSpawn(CBasePlayer *pPlayer) OVERRIDE;
    bool IsSpawnPointValid(CBaseEntity *pSpot, CBasePlayer *pPlayer) OVERRIDE;
    CBaseEntity *GetPlayerSpawnSpot(CBasePlayer *pPlayer) OVERRIDE;

    const char *GetGameDescription(void) OVERRIDE { return "Momentum"; }
    const char *GetChatPrefix(bool bTeamOnly, CBasePlayer *pPlayer) OVERRIDE;

    // Ammo
    void PlayerThink(CBasePlayer *pPlayer) OVERRIDE {}
    // virtual float			GetAmmoDamage(CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType);

    // Players take no damage
    float FlPlayerFallDamage(CBasePlayer *pPlayer) OVERRIDE { return 0.0f; }

    bool AllowDamage(CBaseEntity *pVictim, const CTakeDamageInfo &info) OVERRIDE { return !pVictim->IsPlayer(); }

    void ClientSettingsChanged(CBasePlayer *) OVERRIDE;

    bool FAllowNPCs() OVERRIDE { return false; }

  private:
    // void AdjustPlayerDamageTaken(CTakeDamageInfo *pInfo);
    // float AdjustPlayerDamageInflicted(float damage);
    Vector DropToGround(CBaseEntity *pMainEnt, const Vector &vPos, const Vector &vMins, const Vector &vMaxs);

    int DefaultFOV(void) OVERRIDE;// { return 90; }
#endif
};

inline CMomentumGameRules *GetMomentumGamerules() { return static_cast<CMomentumGameRules *>(g_pGameRules); }

#endif // MOM_GAMERULES_H
