#include "cbase.h"

#include "GhostEntityPanel.h"
#include "clientmode.h"
#include "view.h"
#include "mom_shareddefs.h"
#include "vgui/IVGui.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui_controls/Label.h>
#include "vgui_avatarimage.h"
#include "c_mom_online_ghost.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_entpanels_enable, "1", FLAG_HUD_CVAR, "Shows all entity panels. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_entpanels_enable_avatars, "1", FLAG_HUD_CVAR, "Enables drawing the entity Steam avatars on entity panels. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_entpanels_enable_names, "1", FLAG_HUD_CVAR, "Enables drawing the entity's name on entity panels. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_entpanels_fade_enable, "1", FLAG_HUD_CVAR, "Fades entity panels after a certain distance, over a certain distance. 0 = OFF, 1 = ON\n");
static MAKE_CONVAR(mom_entpanels_fade_dist, "100", FLAG_HUD_CVAR, "The amount of units to linearly fade the entity panel over.\n", 1, MAX_TRACE_LENGTH);
static MAKE_CONVAR(mom_entpanels_fade_start, "4096", FLAG_HUD_CVAR, "The distance (in units) at which entity panels start to fade.\n", 1, MAX_TRACE_LENGTH);

CGhostEntityPanel::CGhostEntityPanel() : BaseClass(g_pClientMode->GetViewport(), "GhostEntityPanel"), m_pEntity(nullptr), m_pAvatarImage(nullptr)
{
    SetProportional(true);
    SetSize(2, 2);

    m_pAvatarImagePanel = new ImagePanel(this, "GhostEntityPanelAvatar");
    m_pAvatarImage = new CAvatarImage();
    m_pNameLabel = new Label(this, "GhostEntityPanelName", "");

    LoadControlSettings("resource/ui/GhostEntityPanel.res");

    SetVisible(false);
    SetPaintBackgroundEnabled(false);

    m_pAvatarImage->SetDrawFriend(false);
    m_pAvatarImagePanel->SetImage(m_pAvatarImage);
}

void CGhostEntityPanel::Init(C_MomentumOnlineGhostEntity* pEnt)
{
    m_pEntity = pEnt;

    m_iPosX = m_iPosY = -1;
    
    SetVisible(true);
    ivgui()->AddTickSignal(GetVPanel());
}

void CGhostEntityPanel::OnThink()
{
    SetPos(static_cast<int>(m_iPosX - GetWide() / 2), static_cast<int>(m_iPosY + m_iOffsetY));

    if (m_pEntity)
    {
        if (!m_pAvatarImage->IsValid() && m_pEntity->GetSteamID())
        {
            m_pAvatarImage->SetAvatarSteamID(CSteamID(m_pEntity->GetSteamID()), k_EAvatarSize64x64);
        }

        if (m_bPaintName)
        {
            char check[MAX_PLAYER_NAME_LENGTH];
            m_pNameLabel->GetText(check, MAX_PLAYER_NAME_LENGTH);
            if (!FStrEq(check, m_pEntity->m_szGhostName.Get()))
                m_pNameLabel->SetText(m_pEntity->m_szGhostName.Get());
        }
        m_pNameLabel->SetVisible(m_bPaintName);
    }

    BaseClass::OnThink();
}

void CGhostEntityPanel::OnTick()
{
    // Visibility checks
    // Check fade status
    bool bFadeShow = true;
    int iFadeAlpha = 255;
    if (mom_entpanels_fade_enable.GetBool() && m_pEntity)
    {
        // Get distance to entity
        float flDistance = (m_pEntity->GetRenderOrigin() - MainViewOrigin()).Length();

        // Get the starting units that this panel should start to fade
        float flFadeStart = mom_entpanels_fade_start.GetFloat();
        // Get the distance over which it should fade
        float flFadeDist = mom_entpanels_fade_dist.GetFloat();

        if (flDistance > flFadeStart)
        {
            float flDiff = flDistance - flFadeStart;

            // The entity being over the fade distance away makes the panel hide
            if (flDiff > flFadeDist)
                bFadeShow = false;
            else // We're starting to fade, calculate it here
                iFadeAlpha = static_cast<int>(255.0f * (1.0f - flDiff / flFadeDist));
        }
        // Else this distance works, fade show should be true
    }

    bool bEntOnScreen = GetEntityPosition(m_iPosX, m_iPosY); // Also doubles as getting the position for render
    bool shouldDraw = ShouldDraw() && bEntOnScreen && bFadeShow;

    SetAlpha(shouldDraw * iFadeAlpha);

    if (shouldDraw != IsVisible())
    {
        SetVisible(shouldDraw);

        if (!shouldDraw)
            return;
    }

    bool isCursorOver = IsCursorOver();
    bool areNamesEnabled = mom_entpanels_enable_names.GetBool();
    
    m_bPaintName = areNamesEnabled && isCursorOver;
    SetPaintBackgroundEnabled(m_bPaintName);
    
    m_pAvatarImagePanel->SetVisible(mom_entpanels_enable_avatars.GetBool());
}

void CGhostEntityPanel::PerformLayout()
{
    BaseClass::PerformLayout();
    SetTall(m_pAvatarImagePanel->GetTall() + m_pNameLabel->GetTall() + m_pAvatarImagePanel->GetYPos() + m_pNameLabel->GetYPos());
    m_pAvatarImage->SetAvatarSize(m_pAvatarImagePanel->GetWide(), m_pAvatarImagePanel->GetTall());
}

bool CGhostEntityPanel::ShouldDraw()
{
    return mom_entpanels_enable.GetBool() && (mom_entpanels_enable_avatars.GetBool() || mom_entpanels_enable_names.GetBool());
}

bool CGhostEntityPanel::GetEntityPosition(int& sx, int& sy)
{
    if (!m_pEntity || !m_pEntity->IsVisible())
    {
        sx = sy = -1.0f;
        return false;
    }
    
    return GetVectorInScreenSpace(m_pEntity->GetAbsOrigin() + 
        (m_pEntity->GetModelPtr() ? m_pEntity->GetModelPtr()->eyeposition() : Vector(0, 0, 62)), 
        sx, sy, nullptr);
}

CGhostEntityPanel::~CGhostEntityPanel()
{
    if (m_pAvatarImage)
    {
        m_pAvatarImage->ClearAvatarSteamID();
        delete m_pAvatarImage;
    }

    m_pAvatarImage = nullptr;

    ivgui()->RemoveTickSignal(GetVPanel());
}
