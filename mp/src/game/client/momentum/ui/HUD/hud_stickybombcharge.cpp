#include "cbase.h"

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/Label.h>

#include "c_mom_player.h"
#include "hudelement.h"
#include "iclientmode.h"

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

enum ChargeMeterUnits_t
{
    CHARGEMETER_UNITS_NONE = 0,
    CHARGEMETER_UNITS_UPS,
    CHARGEMETER_UNITS_PERCENT
};

class CHudStickyCharge : public CHudElement, public EditablePanel
{
  public:
    DECLARE_CLASS_SIMPLE(CHudStickyCharge, EditablePanel);

    CHudStickyCharge(const char *pElementName);

    bool ShouldDraw() override;
    void OnThink() override;
    void Reset() override;
    void FireGameEvent(IGameEvent *pEvent) override;

    CPanelAnimationVar(Color, m_cChargeColor, "ChargeColor", "MomGreydientStep8");
    CPanelAnimationVar(Color, m_cChargeDisabled, "ChargeDisabledColor", "MomentumRed");

  private:
    CMomentumStickybombLauncher *m_pLauncher;
    ContinuousProgressBar *m_pChargeMeter;
    Label *m_pChargeLabel;
};

DECLARE_HUDELEMENT(CHudStickyCharge);

CHudStickyCharge::CHudStickyCharge(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudStickyCharge")
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");

    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pLauncher = nullptr;
    m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");
    m_pChargeLabel = new Label(this, "ChargeMeterLabel", "CHARGE");

    LoadControlSettings("resource/ui/HudStickyCharge.res");
}

void CHudStickyCharge::FireGameEvent(IGameEvent* pEvent)
{
    const auto pLocal = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pLocal)
        return; 

    if (pEvent->GetInt("ent") != pLocal->GetCurrentUIEntity()->GetEntIndex())
        return;

    if (pEvent->GetInt("num") != 1)
        return;

    if (FStrEq(pEvent->GetName(), "zone_enter"))
    {
        // Turn charge meter red while inside start zone to indicate that stickies can't be charged
        m_pChargeLabel->SetVisible(false);
        m_pChargeMeter->SetSubdivMarksVisible(false);
        m_pChargeMeter->SetFgColor(m_cChargeDisabled);
        m_pChargeMeter->SetProgress(1.0f);
    }
    else
    {
        m_pChargeLabel->SetVisible(true);
        m_pChargeMeter->SetSubdivMarksVisible(true);
        m_pChargeMeter->SetFgColor(m_cChargeColor);
    }
}

void CHudStickyCharge::Reset()
{
    m_pLauncher = nullptr;

    m_pChargeLabel->SetVisible(true);
    m_pChargeMeter->SetSubdivMarksVisible(true);
    m_pChargeMeter->SetFgColor(m_cChargeColor);
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
    if (!m_pLauncher || !m_pLauncher->IsChargeEnabled())
        return;

    // Reset the charge label when player stops charging
    if (m_pLauncher->GetChargeBeginTime() <= 0.0f)
    {
        m_pChargeLabel->SetText("CHARGE");
    }

    // Set progress of charge meter
    float flChargeMaxTime = m_pLauncher->GetChargeMaxTime();

    if (!CloseEnough(flChargeMaxTime, 0.0f, FLT_EPSILON))
    {
        float flChargeBeginTime = m_pLauncher->GetChargeBeginTime();

        if (flChargeBeginTime > 0)
        {
            float flTimeCharged = max(0, gpGlobals->curtime - flChargeBeginTime);
            float flPercentCharged = min(1.0, flTimeCharged / flChargeMaxTime);

            m_pChargeMeter->SetProgress(flPercentCharged);

            if (!mom_hud_sj_chargemeter_units.GetInt())
                return;

            char buf[64];
            switch (mom_hud_sj_chargemeter_units.GetInt())
            {
            case CHARGEMETER_UNITS_UPS:
                Q_snprintf(buf, sizeof(buf), "%du/s", static_cast<int>(m_pLauncher->CalculateProjectileSpeed(flTimeCharged)));
                break;
            case CHARGEMETER_UNITS_PERCENT:
            default:
                Q_snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(flPercentCharged * 100.0f));
                break;
            }
            m_pChargeLabel->SetText(buf);
        }
        else
        {
            m_pChargeMeter->SetProgress(0.0f);
        }
    }
}
