#include "cbase.h"
#include "c_basetempentity.h"
#include "engine/IEngineSound.h"
#include "weapon/weapon_base.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

void TFExplosionCallback(const Vector &vecOrigin, const Vector &vecNormal, WeaponID_t iWeaponID)
{
    const auto pWeaponInfo = g_pWeaponDef->GetWeaponScript(iWeaponID);

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
            pWeaponInfo->pKVWeaponParticles->GetString("ExplosionWaterEffect");
        }
        else
        {
            if (bInAir)
            {
                pWeaponInfo->pKVWeaponParticles->GetString("ExplosionPlayerEffect");
            }
            else
            {
                pWeaponInfo->pKVWeaponParticles->GetString("ExplosionEffect");
            }
        }

        // Sound.
        pszSound = pWeaponInfo->pKVWeaponSounds->GetString("explosion");
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

protected:
    void PostDataUpdate(DataUpdateType_t updateType) OVERRIDE;

private:
    Vector m_vecOrigin;
    Vector m_vecNormal;
    WeaponID_t m_iWeaponID;
};

C_TETFExplosion::C_TETFExplosion()
{
    m_vecOrigin.Init();
    m_vecNormal.Init();
    m_iWeaponID = WEAPON_NONE;
}

void C_TETFExplosion::PostDataUpdate(DataUpdateType_t updateType)
{
    VPROF("C_TETFExplosion::PostDataUpdate");

    TFExplosionCallback(m_vecOrigin, m_vecNormal, m_iWeaponID);
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFExplosion, DT_TETFExplosion, CTETFExplosion)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecNormal)),
    RecvPropInt(RECVINFO(m_iWeaponID)),
END_RECV_TABLE();