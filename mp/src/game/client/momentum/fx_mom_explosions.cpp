#include "cbase.h"
#include "c_basetempentity.h"
#include "engine/IEngineSound.h"
#include "weapon/weapon_def.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

static MAKE_CONVAR(mom_rj_particles, "1", FCVAR_ARCHIVE, "Switches between the particles for rocket explosions.\n"
    "0 = None\n1 = Use weapon script (momentum)\n2 = Force TF2 particles\n3 through 6 = alternative momentum particles", 0, 6);

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
    if (updateType == DATA_UPDATE_CREATED)
    {
        const auto pWeaponInfo = g_pWeaponDef->GetWeaponScript(m_iWeaponID);
        AssertMsg(pWeaponInfo, "Invalid pWeaponInfo for weaponID %i\n", m_iWeaponID);
        if (!pWeaponInfo)
            return;

        const bool bIsWater = (UTIL_PointContents(m_vecOrigin) & CONTENTS_WATER);
        // Cannot use zeros here because we are sending the normal at a smaller bit size.
        const bool bInAir = fabs(m_vecNormal.x) < 0.05f && fabs(m_vecNormal.y) < 0.05f && fabs(m_vecNormal.z) < 0.05f;
        QAngle angExplosion(0.0f, 0.0f, 0.0f);

        if (!bInAir)
            VectorAngles(m_vecNormal, angExplosion);

        static ConVarRef mom_rj_sounds("mom_rj_sounds");

        if (mom_rj_sounds.GetInt() > 0)
        {
            const bool bIsTF2Sound = mom_rj_sounds.GetInt() == 2;
            const char *pszSound = pWeaponInfo->pKVWeaponSounds->GetString(bIsTF2Sound ? "explosion_TF2" : "explosion");

            CLocalPlayerFilter filter;
            C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, pszSound, &m_vecOrigin);
        }

        if (mom_rj_particles.GetInt() > 0)
        {
            const char *pszEffect;
            const bool bIsTF2Particle = mom_rj_particles.GetInt() == 2;

            if (bIsWater)
            {
                pszEffect = pWeaponInfo->pKVWeaponParticles->GetString(bIsTF2Particle ? "ExplosionWaterEffect_TF2" : "ExplosionWaterEffect");
            }
            else
            {
                if (bInAir)
                {
                    pszEffect = pWeaponInfo->pKVWeaponParticles->GetString(bIsTF2Particle ? "ExplosionMidAirEffect_TF2" : "ExplosionMidAirEffect");
                }
                else
                {
                    pszEffect = pWeaponInfo->pKVWeaponParticles->GetString(bIsTF2Particle ? "ExplosionEffect_TF2" : "ExplosionEffect");
                }
            }

            // MOM_TODO REMOVEME
            if (mom_rj_particles.GetInt() == 3)
                pszEffect = "mom_rocket_explosion_b";
            else if (mom_rj_particles.GetInt() == 4)
                pszEffect = "mom_rocket_explosion_b_";
            else if (mom_rj_particles.GetInt() == 5)
                pszEffect = "mom_rocket_explosion_c";
            else if (mom_rj_particles.GetInt() == 6)
                pszEffect = "mom_rocket_explosion_d";

            DispatchParticleEffect(pszEffect, m_vecOrigin, angExplosion);
        }
    }
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFExplosion, DT_TETFExplosion, CTETFExplosion)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecNormal)),
    RecvPropInt(RECVINFO(m_iWeaponID)),
END_RECV_TABLE();