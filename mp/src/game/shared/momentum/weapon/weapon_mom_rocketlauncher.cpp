#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_rocketlauncher.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

#define TF_ROCKETLAUNCHER_MODEL "models/weapons/v_models/v_rocketlauncher_soldier.mdl"
#define TF_ROCKETLAUNCHER_WMODEL "models/weapons/w_models/w_rocketlauncher.mdl"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumRocketLauncher, DT_MomentumRocketLauncher)

BEGIN_NETWORK_TABLE(CMomentumRocketLauncher, DT_MomentumRocketLauncher)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumRocketLauncher)
#ifdef CLIENT_DLL
    DEFINE_PRED_FIELD(m_iTFViewIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
    DEFINE_PRED_FIELD(m_iTFWorldIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
    DEFINE_PRED_FIELD(m_iMomViewIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
    DEFINE_PRED_FIELD(m_iMomWorldIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_rocketlauncher, CMomentumRocketLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_rocketlauncher);

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR_CV(mom_rj_center_fire, "0", FCVAR_ARCHIVE, "If enabled, all rockets will be fired from the center of the screen. 0 = OFF, 1 = ON\n", nullptr,
    [](IConVar *pVar, const char *pNewVal)
    {
        if (g_pMomentumTimer->IsRunning())
        {
            Warning("Cannot change rocket firing mode while in a run! Stop your timer to be able to change it.\n");
            return false;
        }

        return true;
    }
);
static MAKE_TOGGLE_CONVAR(mom_rj_use_tf_viewmodel, "0", FCVAR_ARCHIVE,
                          "Toggles between the TF2 Rocket Launcher model and the Momentum one. 0 = Momentum, 1 = TF2\n");
static MAKE_CONVAR(mom_rj_sounds, "1", FCVAR_ARCHIVE,
                   "Toggles between the TF2 rocket and weapon sounds and the Momentum ones. 0 = None, 1 = Momentum, 2 = TF2\n", 0, 2);
#endif

CMomentumRocketLauncher::CMomentumRocketLauncher()
{
    m_flTimeToIdleAfterFire = 0.8f;
    m_flIdleInterval = 20.0f;
}

void CMomentumRocketLauncher::Precache()
{
    BaseClass::Precache();

    const auto hWeaponData = GetWpnData();

    m_iMomViewIndex = PrecacheModel(hWeaponData.szViewModel);
    m_iMomWorldIndex = PrecacheModel(hWeaponData.szWorldModel);
    m_iTFViewIndex = PrecacheModel(TF_ROCKETLAUNCHER_MODEL);
    m_iTFWorldIndex = PrecacheModel(TF_ROCKETLAUNCHER_WMODEL);

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}

bool CMomentumRocketLauncher::Deploy()
{
    CMomentumPlayer *pOwner = GetPlayerOwner();
    if (pOwner)
    {
#ifdef CLIENT_DLL
        static ConVarRef mom_rj_use_tf_viewmodel("mom_rj_use_tf_viewmodel");
#endif
        SetModelType(mom_rj_use_tf_viewmodel.GetBool());
    }

    return BaseClass::Deploy();
}

const char *CMomentumRocketLauncher::GetViewModel(int) const
{
    return m_iViewModelIndex == m_iTFViewIndex ? TF_ROCKETLAUNCHER_MODEL : GetWpnData().szViewModel;
}

const char *CMomentumRocketLauncher::GetWorldModel() const
{
    return m_iWorldModelIndex == m_iTFWorldIndex ? TF_ROCKETLAUNCHER_WMODEL : GetWpnData().szWorldModel;
}

void CMomentumRocketLauncher::SetModelType(bool bTF2Model)
{
    m_iViewModelIndex = bTF2Model ? m_iTFViewIndex.Get() : m_iMomViewIndex.Get();
    m_iWorldModelIndex = bTF2Model ? m_iTFWorldIndex.Get() : m_iMomWorldIndex.Get();
    SetModel(GetViewModel());
}

//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CMomentumRocketLauncher::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward)
{
#ifdef GAME_DLL
    static ConVarRef cl_righthand("cl_righthand");
#else
    extern ConVar_Validated cl_righthand;
    static ConVarRef mom_rj_center_fire("mom_rj_center_fire");
#endif

    if (mom_rj_center_fire.GetBool())
    {
        vecOffset.y = 0.0f;
    }
    else if (!cl_righthand.GetBool())
    {
        vecOffset.y *= -1.0f;
    }

    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecShootPos = pPlayer->Weapon_ShootPosition();

    // Estimate end point
    Vector endPos = vecShootPos + vecForward * 2000.0f;

    // Trace forward and find what's in front of us, and aim at that
    trace_t tr;

    CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
    UTIL_TraceLine(vecShootPos, endPos, MASK_SOLID_BRUSHONLY, &filter, &tr);

    // Offset actual start point
    *vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

    // Find angles that will get us to our desired end point
    // Only use the trace end if it wasn't too close, which results
    // in visually bizarre forward angles
    if (tr.fraction > 0.1f)
    {
        VectorAngles(tr.endpos - *vecSrc, *angForward);
    }
    else
    {
        VectorAngles(endPos - *vecSrc, *angForward);
    }
}

void CMomentumRocketLauncher::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;
    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();

#ifdef CLIENT_DLL
    static ConVarRef mom_rj_sounds("mom_rj_sounds");
#endif

    if (mom_rj_sounds.GetInt() == 1)
    {
        WeaponSound(GetWeaponSound("single_shot"));
    }
    else if (mom_rj_sounds.GetInt() == 2)
    {
        WeaponSound(GetWeaponSound("special1"));
    }

    // MOM_FIXME:
    // Should no longer Assert, unsure about BaseGunFire() though
    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
    Vector vForward, vRight, vUp;

    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    // Offset values from
    // https://github.com/NicknineTheEagle/TF2-Base/blob/master/src/game/shared/tf/tf_weaponbase_gun.cpp#L334
    Vector vecOffset(23.5f, 12.0f, -3.0f);
    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        vecOffset.z = 8.0f;
    }
    Vector vecSrc;
    QAngle angForward;
    GetProjectileFireSetup(pPlayer, vecOffset, &vecSrc, &angForward);

    trace_t trace;
    CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
    UTIL_TraceLine(pPlayer->EyePosition(), vecSrc, MASK_SOLID_BRUSHONLY, &traceFilter, &trace);

    CMomRocket::EmitRocket(trace.endpos, angForward, pPlayer);

    DecalPacket rocket = DecalPacket::Rocket(trace.endpos, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocket);
#endif
}

bool CMomentumRocketLauncher::CanDeploy()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        return false;

    return BaseClass::CanDeploy();
}