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
static MAKE_CONVAR(mom_hud_sj_chargemeter_units, "0", FCVAR_ARCHIVE,
                   "If above 0, shows the speed at which a sticky will be launched when charged.\n"
                   " 0 = OFF\n"
                   " 1 = Speed in Hammer units (900-2400u/s)\n"
                   " 2 = Speed in percent (0% = 900u/s, 100% = 2400u/s)",
                   0, 2);

class CHudStickyCharge : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickyCharge, EditablePanel);

  public:
    CHudStickyCharge(const char *pElementName);

    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Reset() OVERRIDE;

  private:
    CMomentumStickybombLauncher *m_pLauncher;
    ContinuousProgressBar *m_pChargeMeter;
    Label *m_pChargeLabel;
};

DECLARE_HUDELEMENT(CHudStickyCharge);

CHudStickyCharge::CHudStickyCharge(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudStickyCharge")
{
    m_pLauncher = nullptr;
    m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");
    m_pChargeLabel = new Label(this, "ChargeMeterLabel", "CHARGE");

    LoadControlSettings("resource/ui/HudStickyCharge.res");
}

void CHudStickyCharge::Reset()
{
    m_pLauncher = nullptr;
}

bool CHudStickyCharge::ShouldDraw()
{
    if (!mom_hud_sj_chargemeter_enable.GetBool())
        return false;

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ) || !pPlayer->IsAlive())
        return false;

    const auto pWeapon = pPlayer->GetActiveWeapon();

    if (!pWeapon || pWeapon->GetWeaponID() != WEAPON_STICKYLAUNCHER)
        return false;

    m_pLauncher = static_cast<CMomentumStickybombLauncher *>(pWeapon);

    return CHudElement::ShouldDraw();
}

void CHudStickyCharge::OnThink()
{
    if (!m_pLauncher)
        return;

    // Reset the charge label when player stops charging
    if (m_pLauncher->GetChargeBeginTime() <= 0.0f)
    {
        m_pChargeLabel->SetText("CHARGE");
    }

    // Turn charge meter red while inside start zone to indicate that stickies can't be charged
    if (!m_pLauncher->IsChargeEnabled())
    {
        m_pChargeMeter->SetFgColor(Color(192, 28, 0, 140));
        m_pChargeMeter->SetProgress(1.0f);

        m_pChargeLabel->SetVisible(false);
    }
    else
    {
        // If charge label was disabled by start zone, enable it again
        if (!m_pChargeLabel->IsVisible())
            m_pChargeLabel->SetVisible(true);

        m_pChargeMeter->SetFgColor(Color(235, 235, 235, 255));
        float flChargeMaxTime = m_pLauncher->GetChargeMaxTime();

        if (!CloseEnough(flChargeMaxTime, 0.0f, FLT_EPSILON))
        {
            float flChargeBeginTime = m_pLauncher->GetChargeBeginTime();

            if (flChargeBeginTime > 0)
            {
                float flTimeCharged = max(0, gpGlobals->curtime - flChargeBeginTime);
                float flPercentCharged = min(1.0, flTimeCharged / flChargeMaxTime);

                m_pChargeMeter->SetProgress(flPercentCharged);

                if (mom_hud_sj_chargemeter_units.GetInt() == 1)
                {
                    char buf[64];
                    Q_snprintf(buf, sizeof(buf), "%du/s", (int) m_pLauncher->CalculateProjectileSpeed(flTimeCharged));

                    m_pChargeLabel->SetText(buf);
                }
                else if (mom_hud_sj_chargemeter_units.GetInt() == 2)
                {
                    char buf[64];
                    Q_snprintf(buf, sizeof(buf), "%d%%", (int) (flPercentCharged * 100));

                    m_pChargeLabel->SetText(buf);
                }
            }
            else
            {
                m_pChargeMeter->SetProgress(0.0f);
            }
        }
    }
}