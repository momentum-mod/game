#include "cbase.h"

#include "weapon_mom_machinegun.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumMachinegun, DT_MomentumMachinegun);

BEGIN_NETWORK_TABLE(CMomentumMachinegun, DT_MomentumMachinegun)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumMachinegun)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_machinegun, CMomentumMachinegun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_machinegun);

static MAKE_TOGGLE_CONVAR(sv_smg_recoil, "0", FCVAR_ARCHIVE, "Toggles the SMG recoil. 0 = OFF, 1 = ON\n");

int CMomentumMachinegun::GetSlot(void) const
{
    if (g_pGameModeSystem->GameModeIs(GAMEMODE_DEFRAG))
    {
        return 1;
    }
    else
    {
        return BaseClass::GetSlot();
    }
}

CMomentumMachinegun::CMomentumMachinegun()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 2.0f;

    m_iPrimaryAmmoType = AMMO_TYPE_MACHINEGUN;
}

void CMomentumMachinegun::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!BaseGunFire(0.0f, 0.07f, true))
        return;

    if (sv_smg_recoil.GetBool())
    {
        // Kick the gun based on the state of the player.
        if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
            pPlayer->KickBack(.33, 0.23, 0.18, 0.020, 3.7, 2.1, 4.5);
        else if (pPlayer->GetAbsVelocity().Length2D() > 5)
            pPlayer->KickBack(0.40, 0.25, 0.2, 0.0260, 4, 2.17, 4.9);
        else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
            pPlayer->KickBack(0.275, 0.2, 0.125, 0.02, 3, 1, 9);
        else
            pPlayer->KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
    }
}
