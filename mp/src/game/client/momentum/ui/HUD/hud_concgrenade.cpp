#include "cbase.h"

#include "hud_concgrenade.h"

#include <vgui/IVGui.h>
#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"

#include "c_mom_player.h"
#include "view.h"
#include "iclientmode.h"

#include "mom_concgrenade.h"
#include "mom_system_gamemode.h"
#include "weapon/weapon_mom_concgrenade.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_conc_timer_enable, "1", FLAG_HUD_CVAR, "Toggles the conc timer on or off.\n");
static MAKE_TOGGLE_CONVAR(mom_hud_conc_timer_countdown, "0", FLAG_HUD_CVAR, "Toggles whether the countdown for the conc timer is shown.\n");

static MAKE_TOGGLE_CONVAR(mom_hud_conc_entpanels_enable, "1", FLAG_HUD_CVAR, "Toggles whether concs have their fuse timer displayed above them as an ent panel.\n");
static MAKE_TOGGLE_CONVAR(mom_hud_conc_entpanels_fade_enable, "1", FLAG_HUD_CVAR, "Toggles whether the conc ent panels fade after a certain distance.\n");
static MAKE_TOGGLE_CONVAR(mom_hud_conc_entpanels_panel_enable, "1", FLAG_HUD_CVAR, "Toggles whether conc ent panels display the panel with the remaining fuse timer.\n");
static MAKE_TOGGLE_CONVAR(mom_hud_conc_entpanels_label_enable, "1", FLAG_HUD_CVAR, "Toggles whether conc ent panels display a label with the remaining fuse timer in seconds.\n");

static MAKE_CONVAR(mom_hud_conc_entpanels_fade_dist, "100", FLAG_HUD_CVAR, "The amount of units to linearly fade the conc ent panel over.\n", 1, MAX_TRACE_LENGTH);
static MAKE_CONVAR(mom_hud_conc_entpanels_fade_start, "4096", FLAG_HUD_CVAR, "The distance (in units) at which conc ent panels start to fade.\n", 1, MAX_TRACE_LENGTH);

CHudConcTimer::CHudConcTimer(const char *pElementName) : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudConcTimer")
{
    m_pGrenade = nullptr;
    m_pTimer = new ContinuousProgressBar(this, "ConcTimer");
    m_pTimerLabel = new Label(this, "ConcTimerLabel", "Timer");

    LoadControlSettings("resource/ui/HudConcTimer.res");
}

void CHudConcTimer::Reset()
{
    m_pGrenade = nullptr;
}

bool CHudConcTimer::ShouldDraw()
{
    if (!mom_hud_conc_timer_enable.GetBool() || !g_pGameModeSystem->GameModeIs(GAMEMODE_CONC))
        return false;

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer || !pPlayer->IsAlive())
        return false;

    const auto pWeapon = pPlayer->GetActiveWeapon();

    if (!pWeapon || pWeapon->GetWeaponID() != WEAPON_CONCGRENADE)
        return false;

    m_pGrenade = static_cast<CMomentumConcGrenade*>(pWeapon);

    return CHudElement::ShouldDraw();
}

void CHudConcTimer::OnThink()
{
    if (!m_pGrenade)
        return;

    // Reset the timer label when the conc explodes
    if (mom_hud_conc_timer_countdown.GetBool() && m_pGrenade->GetGrenadeTimer() <= 0.0f)
    {
        m_pTimerLabel->SetText("TIMER");
    }

    m_pTimer->SetFgColor(Color(235, 235, 235, 255));
    float flMaxTimer = m_pGrenade->GetMaxTimer();

    if (!CloseEnough(flMaxTimer, 0.0f, FLT_EPSILON))
    {
        float flGrenadeTimer = m_pGrenade->GetGrenadeTimer();

        if (flGrenadeTimer > 0)
        {
            float flPrimedTime = max(0, gpGlobals->curtime - flGrenadeTimer);
            float flPrimedPercent = min(1.0f, flPrimedTime / flMaxTimer);

            m_pTimer->SetProgress(flPrimedPercent);

            if (mom_hud_conc_timer_countdown.GetBool())
            {
                char buf[64];
                Q_snprintf(buf, sizeof(buf), "%.2f s", flPrimedTime);

                 m_pTimerLabel->SetText(buf);
            }
        }
        else
        {
            m_pTimer->SetProgress(0.0f);
        }
    }
}

CHudConcEntPanel::CHudConcEntPanel() : Panel(g_pClientMode->GetViewport()), m_pGrenade(nullptr)
{
    SetPaintBackgroundEnabled(false);

    m_pHudTimer = new ContinuousProgressBar(this, "ConcEntPanelTimer");
    m_pHudTimer->SetSize(80, 15);
    m_pHudTimer->SetVisible(mom_hud_conc_entpanels_panel_enable.GetBool() ? true : false);
    m_pTimerLabel = new Label(this, "ConcEntPanelLabel", "");
    m_pTimerLabel->SetCenterWrap(true);
    m_pTimerLabel->SetVisible(mom_hud_conc_entpanels_label_enable.GetBool() ? true : false);

    SetVisible(false);
    SetSize(160, 80);
}

CHudConcEntPanel::~CHudConcEntPanel()
{
    if (m_pHudTimer)
    {
        m_pHudTimer->DeletePanel();
    }

    m_pHudTimer = nullptr;
}

void CHudConcEntPanel::Init(CMomConcProjectile *pEntity)
{
    m_pGrenade = pEntity;
    m_OffsetX = -75;
    m_OffsetY = -40;
    m_iOrgWidth = 128;
    m_iOrgHeight = 128;
    m_iOrgOffsetX = m_OffsetX;
    m_iOrgOffsetY = m_OffsetY;
    m_iPosX = m_iPosY = -1;

    SetVisible(true);
    ivgui()->AddTickSignal(GetVPanel());
}

void CHudConcEntPanel::OnThink()
{
    // Recalculate our size
    ComputeAndSetSize();

    // Set the position
    SetPos(static_cast<int>(m_iPosX + m_OffsetX + 0.5f), static_cast<int>(m_iPosY + m_OffsetY + 0.5f));

    if (m_pGrenade)
    {
        m_pHudTimer->SetFgColor(Color(235, 235, 235, 255));
        float flMaxTimer = m_pGrenade->GetMaxTimer();

        if (!CloseEnough(flMaxTimer, 0.0f, FLT_EPSILON))
        {
            float flGrenadeTimer = m_pGrenade->m_flDetonateTime.Get();

            if (flGrenadeTimer > 0)
            {
                float flPrimedTime = max(0, flGrenadeTimer - gpGlobals->curtime);
                float flPrimedPercent = min(1.0, flPrimedTime / flMaxTimer);

                m_pHudTimer->SetProgress(flPrimedPercent);

                char buf[64];
                Q_snprintf(buf, sizeof(buf), "%.2f s", flPrimedTime);

                m_pTimerLabel->SetText(buf);
            }
            else
            {
                m_pHudTimer->SetProgress(0.0f);
            }
        }
    }

    BaseClass::OnThink();
}

void CHudConcEntPanel::OnTick()
{
    bool bFadeShow = true;
    int iFadeAlpha = 255;

    if (m_pGrenade && mom_hud_conc_entpanels_fade_enable.GetBool())
    {
        float flDistance = (m_pGrenade->GetRenderOrigin() - MainViewOrigin()).Length();

        float flFadeStart = mom_hud_conc_entpanels_fade_start.GetFloat();
        float flFadeDist = mom_hud_conc_entpanels_fade_dist.GetFloat();

        if (flDistance > flFadeStart)
        {
            float flDiff = flDistance - flFadeStart;

            if (flDiff > flFadeDist)
                bFadeShow = false;
            else
                iFadeAlpha = static_cast<int>(255.0f * (1.0f - flDiff / flFadeDist));
        }
    }

    bool bEntOnScreen = GetEntityPosition(m_iPosX, m_iPosY);
    bool shouldDraw = ShouldDraw() && bEntOnScreen && bFadeShow;

    SetAlpha(shouldDraw * iFadeAlpha);

    if (shouldDraw != IsVisible())
    {
        SetVisible(shouldDraw);
    }
}

bool CHudConcEntPanel::ShouldDraw()
{
    return mom_hud_conc_entpanels_enable.GetBool();
}


bool CHudConcEntPanel::GetEntityPosition(int &sx, int &sy)
{
    if (!m_pGrenade || !m_pGrenade->IsVisible())
    {
        sx = sy = -1.0f;
        return false;
    }

    return GetVectorInScreenSpace(m_pGrenade->GetAbsOrigin() +
        (m_pGrenade->GetModelPtr() ? m_pGrenade->GetModelPtr()->eyeposition() : Vector(0, 0, 62)),
        sx, sy, nullptr);
}

void CHudConcEntPanel::ComputeAndSetSize()
{
    int panelWide, panelHigh;
    GetSize(panelWide, panelHigh);

    m_pHudTimer->SetPos(panelWide / 4, panelHigh / 2);
    m_pTimerLabel->SetPos((panelWide / 4) + 4, panelHigh / 6);
}