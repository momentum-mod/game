#include "cbase.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/Label.h>
#include "c_mom_player.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "mom_system_gamemode.h"
#include "weapon/weapon_base.h"
#include "weapon/weapon_mom_stickybomblauncher.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_sj_chargemeter_enable, "1", FCVAR_ARCHIVE, "Toggles the charge meter on or off.\n");

class CHudStickyCharge : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickyCharge, EditablePanel);

  public:
    CHudStickyCharge(const char *pElementName);

    void ApplySchemeSettings(IScheme *scheme) OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void OnTick() OVERRIDE;

  private:
    vgui::ContinuousProgressBar *m_pChargeMeter;
    vgui::Label *m_pChargeLabel;
};

DECLARE_HUDELEMENT(CHudStickyCharge);

CHudStickyCharge::CHudStickyCharge(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(NULL, "HudStickyCharge")
{
    Panel *pParent = g_pClientMode->GetViewport();
    SetParent(pParent);

    m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");
    m_pChargeLabel = new Label(this, "ChargeMeterLabel", "CHARGE");

    LoadControlSettings("resource/ui/HudStickyCharge.res");

    vgui::ivgui()->AddTickSignal(GetVPanel());
}

void CHudStickyCharge::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
}

bool CHudStickyCharge::ShouldDraw()
{
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!mom_sj_chargemeter_enable.GetBool())
        return false;

    if (!pPlayer || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ) || !pPlayer->IsAlive())
        return false;

    CWeaponBase *pGun = dynamic_cast<CWeaponBase*>(pPlayer->GetActiveWeapon());

    if (!pGun)
        return false;

    int iWeaponID = pGun->GetWeaponID();

    if (iWeaponID != WEAPON_STICKYLAUNCHER)
        return false;

    if (!mom_sj_charge_enable.GetBool())
        return false;

    return CHudElement::ShouldDraw();
}

void CHudStickyCharge::OnTick()
{
    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer)
        return;

    const auto pGun = dynamic_cast<CWeaponBase*>(pPlayer->GetActiveWeapon());

    if (!pGun)
        return;

    const auto pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pGun);

    if (!pLauncher)
        return;

    if (m_pChargeMeter)
    {
        float flChargeMaxTime = pLauncher->GetChargeMaxTime();

        if (flChargeMaxTime != 0)
        {
            float flChargeBeginTime = pLauncher->GetChargeBeginTime();

            if (flChargeBeginTime > 0)
            {
                float flTimeCharged = max(0, gpGlobals->curtime - flChargeBeginTime);
                float flPercentCharged = min(1.0, flTimeCharged / flChargeMaxTime);

                m_pChargeMeter->SetProgress(flPercentCharged);
            }
            else
            {
                m_pChargeMeter->SetProgress(0.0f);
            }
        }
    }
}