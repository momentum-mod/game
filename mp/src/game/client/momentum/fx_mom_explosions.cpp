#include "cbase.h"
#include "c_basetempentity.h"
#include "engine/IEngineSound.h"
#include "weapon/weapon_def.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_sj_particle_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the particles for sticky explosions. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_sj_sound_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the sticky explosion sound. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_rj_particle_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the particles for rocket explosions. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_rj_sound_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the rocket explosion sound. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_conc_particle_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the particles for conc explosions. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_conc_sound_explosion_enable, "1", FCVAR_ARCHIVE, "Toggles the conc explosion sound. 0 = OFF, 1 = ON\n");

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
    bool m_bHandHeld;
};

C_TETFExplosion::C_TETFExplosion()
{
    m_vecOrigin.Init();
    m_vecNormal.Init();
    m_iWeaponID = WEAPON_NONE;
    m_bHandHeld = false;
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

        bool bPlaySound = false;
        bool bDispatchParticles = false;

        if (m_iWeaponID == WEAPON_ROCKETLAUNCHER)
        {
            bPlaySound = mom_rj_sound_explosion_enable.GetBool();
            bDispatchParticles = mom_rj_particle_explosion_enable.GetBool();
        }
        else if (m_iWeaponID == WEAPON_STICKYLAUNCHER)
        {
            bPlaySound = mom_sj_sound_explosion_enable.GetBool();
            bDispatchParticles = mom_sj_particle_explosion_enable.GetBool();
        }
        else if (m_iWeaponID == WEAPON_CONCGRENADE)
        {
            bPlaySound = mom_conc_sound_explosion_enable.GetBool();
            bDispatchParticles = mom_conc_particle_explosion_enable.GetBool();
        }

        if (bPlaySound)
        {
            CLocalPlayerFilter filter;
            C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, pWeaponInfo->pKVWeaponSounds->GetString("explosion"), &m_vecOrigin);
        }

        if (bDispatchParticles)
        {
            const char *pszEffect;
            if (m_bHandHeld)
            {
                pszEffect = "ExplosionHandHeld";
            }
            else if (bIsWater)
            {
                pszEffect = "ExplosionWaterEffect";
            }
            else if (bInAir)
            {
                pszEffect = "ExplosionMidAirEffect";
            }
            else
            {
                pszEffect = "ExplosionEffect";
            }

            DispatchParticleEffect(pWeaponInfo->pKVWeaponParticles->GetString(pszEffect), m_vecOrigin, angExplosion);
        }
    }
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFExplosion, DT_TETFExplosion, CTETFExplosion)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecNormal)),
    RecvPropInt(RECVINFO(m_iWeaponID)),
    RecvPropBool(RECVINFO(m_bHandHeld)),
END_RECV_TABLE();