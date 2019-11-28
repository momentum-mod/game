//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific explosion effects
//
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"
#include "dlight.h"
#include "engine/IEngineSound.h"
#include "iefx.h"
#include "mom_shareddefs.h"
#include "tempent.h"
#include "tier0/vprof.h"
#include "weapon/mom_weapon_parse.h"
#include "weapon/weapon_mom_rocketlauncher.h"

#include "tier0/memdbgon.h"

CWeaponInfo *GetMomWeaponInfo(int iWeapon)
{
    const char *pszWeaponAlias = WeaponIDToAlias(iWeapon);
    if (!pszWeaponAlias)
    {
        return nullptr;
    }

    WEAPON_FILE_INFO_HANDLE hWpnInfo = LookupWeaponInfoSlot(pszWeaponAlias);
    if (hWpnInfo == GetInvalidWeaponInfoHandle())
    {
        return nullptr;
    }

    return static_cast<CWeaponInfo *>(GetFileWeaponInfoFromHandle(hWpnInfo));
}

void TFExplosionCallback(const Vector &vecOrigin, const Vector &vecNormal, CWeaponID iWeaponID,
                         ClientEntityHandle_t hEntity)
{
    const auto pWeaponInfo = GetWeaponInfo(iWeaponID);

    bool bIsPlayer = false;
    if (hEntity.Get())
    {
        C_BaseEntity *pEntity = C_BaseEntity::Instance(hEntity);
        if (pEntity && pEntity->IsPlayer())
        {
            bIsPlayer = true;
        }
    }

    // Calculate the angles, given the normal.
    bool bIsWater = (UTIL_PointContents(vecOrigin) & CONTENTS_WATER);
    bool bInAir = false;
    QAngle angExplosion(0.0f, 0.0f, 0.0f);

    // Cannot use zeros here because we are sending the normal at a smaller bit size.
    if (fabs(vecNormal.x) < 0.05f && fabs(vecNormal.y) < 0.05f && fabs(vecNormal.z) < 0.05f)
    {
        bInAir = true;
        angExplosion.Init();
    }
    else
    {
        VectorAngles(vecNormal, angExplosion);
        bInAir = false;
    }

    static ConVarRef mom_rj_particles("mom_rj_particles");
    static ConVarRef mom_rj_sounds("mom_rj_sounds");

    // Base explosion effect and sound.
    const char *pszEffect = "ExplosionCore_wall";
    const char *pszSound = "BaseExplosionEffect.Sound";

    if (pWeaponInfo)
    {
        // Explosions.
        if (bIsWater)
        {
            if (Q_strlen(pWeaponInfo->m_szExplosionWaterEffect) > 0)
                pszEffect = pWeaponInfo->m_szExplosionWaterEffect;
        }
        else
        {
            if (bIsPlayer || bInAir)
            {
                if (Q_strlen(pWeaponInfo->m_szExplosionPlayerEffect) > 0)
                    pszEffect = pWeaponInfo->m_szExplosionPlayerEffect;
            }
            else
            {
                if (Q_strlen(pWeaponInfo->m_szExplosionEffect) > 0)
                    pszEffect = pWeaponInfo->m_szExplosionEffect;
            }
        }

        // Sound.
        if (Q_strlen(pWeaponInfo->m_szExplosionSound) > 0)
        {
            pszSound = pWeaponInfo->m_szExplosionSound;
        }
    }

    if (mom_rj_sounds.GetInt() > 0)
    {
        CLocalPlayerFilter filter;
        if (mom_rj_sounds.GetInt() == 2)
            pszSound = "BaseExplosionEffect.SoundTF2";
        C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, pszSound, &vecOrigin);
    }
    
    if (mom_rj_particles.GetInt() == 2)
    {
        DispatchParticleEffect(pszEffect, vecOrigin, angExplosion);
    }
}

class C_TETFExplosion : public C_BaseTempEntity
{
  public:
    DECLARE_CLASS(C_TETFExplosion, C_BaseTempEntity);
    DECLARE_CLIENTCLASS();

    C_TETFExplosion();

    ClientEntityHandle_t m_hEntity;

  private:
    virtual void PostDataUpdate(DataUpdateType_t updateType);

    Vector m_vecOrigin;
    Vector m_vecNormal;
    CWeaponID m_iWeaponID;
};

C_TETFExplosion::C_TETFExplosion()
{
    m_vecOrigin.Init();
    m_vecNormal.Init();
    m_iWeaponID = WEAPON_NONE;
    m_hEntity = INVALID_EHANDLE_INDEX;
}

void C_TETFExplosion::PostDataUpdate(DataUpdateType_t updateType)
{
    VPROF("C_TETFExplosion::PostDataUpdate");

    TFExplosionCallback(m_vecOrigin, m_vecNormal, m_iWeaponID, m_hEntity);
}

static void RecvProxy_ExplosionEntIndex(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
    int nEntIndex = pData->m_Value.m_Int;
    ((C_TETFExplosion *)pStruct)->m_hEntity =
        (nEntIndex < 0) ? INVALID_EHANDLE_INDEX : ClientEntityList().EntIndexToHandle(nEntIndex);
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFExplosion, DT_TETFExplosion, CTETFExplosion)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecNormal)),
    RecvPropInt(RECVINFO(m_iWeaponID)),
    RecvPropInt("entindex", 0, SIZEOF_IGNORE, 0, RecvProxy_ExplosionEntIndex),
END_RECV_TABLE()