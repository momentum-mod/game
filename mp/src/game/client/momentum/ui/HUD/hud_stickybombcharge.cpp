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
#include "weapon/weapon_mom_stickybomblauncher.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_sj_chargemeter_enable, "1", FCVAR_ARCHIVE, "Toggles the charge meter on or off.\n");

class CHudStickyCharge : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickyCharge, EditablePanel);

  public:
    CHudStickyCharge(const char *pElementName);

    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Reset() OVERRIDE;

  private:
    CMomentumStickybombLauncher *pLauncher;
    ContinuousProgressBar *m_pChargeMeter;
    Label *m_pChargeLabel;
};

DECLARE_HUDELEMENT(CHudStickyCharge);

CHudStickyCharge::CHudStickyCharge(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudStickyCharge")
{
    pLauncher = nullptr;
    m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");
    m_pChargeLabel = new Label(this, "ChargeMeterLabel", "CHARGE");

    LoadControlSettings("resource/ui/HudStickyCharge.res");
}

void CHudStickyCharge::Reset()
{
    pLauncher = nullptr;
}

bool CHudStickyCharge::ShouldDraw()
{
    static ConVarRef mom_sj_firing_mode("mom_sj_firing_mode");

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!mom_hud_sj_chargemeter_enable.GetBool())
        return false;

    if (!pPlayer || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ) || !pPlayer->IsAlive())
        return false;

    CWeaponBase *pGun = dynamic_cast<CWeaponBase *>(pPlayer->GetActiveWeapon());

    if (!pGun)
        return false;

    int iWeaponID = pGun->GetWeaponID();

    if (iWeaponID != WEAPON_STICKYLAUNCHER)
        return false;

    if (!mom_sj_firing_mode.GetBool())
        return false;

    return CHudElement::ShouldDraw();
}

void CHudStickyCharge::OnThink()
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pPlayer->GetActiveWeapon());

    if (!pPlayer || !pLauncher)
        return;

    if (m_pChargeMeter)
    {
        // Turn charge meter red while inside start zone to indicate that stickies can't be charged
        if (!pLauncher->IsChargeEnabled())
        {
            m_pChargeMeter->SetFgColor(Color(192, 28, 0, 140));
            m_pChargeMeter->SetProgress(1.0f);
        }
        else
        {
            m_pChargeMeter->SetFgColor(Color(235, 235, 235, 255));
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
}