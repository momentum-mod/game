#include "cbase.h"

#include "weapon_mom_paintgun.h"
#ifdef CLIENT_DLL
#include "PaintGunPanel.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumPaintGun, DT_MomentumPaintGun)

BEGIN_NETWORK_TABLE(CMomentumPaintGun, DT_MomentumPaintGun)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumPaintGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_paintgun, CMomentumPaintGun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_paintgun);


CMomentumPaintGun::CMomentumPaintGun()
#ifdef CLIENT_DLL
: m_cvarDrawPaintgun("mom_paintgun_drawmodel")
#endif
{
    m_flTimeToIdleAfterFire = 0.0f;
    m_flIdleInterval = 0.0f;
#ifdef CLIENT_DLL
    m_pSettingsPanel = new vgui::PaintGunPanel();
#endif
}

CMomentumPaintGun::~CMomentumPaintGun()
{
#ifdef CLIENT_DLL
    m_pSettingsPanel->DeletePanel();
    m_pSettingsPanel = nullptr;
#endif
}

void CMomentumPaintGun::RifleFire()
{
    // Hardcoded here so people don't change the text files for easy spam
    if (!CSBaseGunFire(0.0f, 0.1f, true))
        return;
}

void CMomentumPaintGun::GetControlPanelInfo(int nPanelIndex, const char*& pPanelName)
{
    pPanelName = "paintgun_screen";
}

#ifdef GAME_DLL
void PaintGunDraw(IConVar *var, const char *pOldValue, float flOldValue)
{
    CBasePlayer *pBasePlayer = UTIL_GetLocalPlayer();
    if (pBasePlayer)
    {
        CBaseCombatWeapon *pWeapon = pBasePlayer->GetActiveWeapon();
        if (pWeapon)
        {
            ConVarRef varRef(var);
            pWeapon->SetWeaponVisible(varRef.GetBool());
        }
    }
}

static MAKE_TOGGLE_CONVAR_C(mom_paintgun_drawmodel, "1", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, "Draws the viewmodel of the paintgun. 0 = OFF, 1 = ON\n", PaintGunDraw);
static MAKE_TOGGLE_CONVAR(mom_paintgun_shoot_sound, "1", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, "Toggles the paintgun's primary fire noise. 0 = OFF, 1 = ON\n");

bool CMomentumPaintGun::ShouldShowControlPanels()
{
    return mom_paintgun_drawmodel.GetBool();
}

#endif


#ifdef CLIENT_DLL
bool C_MomentumPaintGun::IsOverridingViewmodel()
{
    return !m_cvarDrawPaintgun.GetBool();
}

// Our overridden viewmodel is not drawn, due to being hidden
int C_MomentumPaintGun::DrawOverriddenViewmodel(C_BaseViewModel* pViewmodel, int flags)
{
    return 0;
}
#endif

void CMomentumPaintGun::PrimaryAttack() { RifleFire(); }

void CMomentumPaintGun::SecondaryAttack()
{
    IGameEvent *pCloseEvent = gameeventmanager->CreateEvent("paintgun_panel");
    if (pCloseEvent)
    {
        // Open our paintgun panel
        gameeventmanager->FireEvent(pCloseEvent);
    }

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}
