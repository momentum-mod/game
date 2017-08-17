#include "cbase.h"

#include "GhostEntityPanel.h"
#include "clientmode.h"

#include "tier0/memdbgon.h"
#include "view.h"
#include "mom_shareddefs.h"
#include "vgui/IVGui.h"
#include "vgui/ILocalize.h"


static MAKE_TOGGLE_CONVAR(mom_entpanels_enable, "1", FLAG_HUD_CVAR, "Shows all entity panels. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_entpanels_enable_avatars, "1", FLAG_HUD_CVAR, "Enables drawing the entity Steam avatars on entity panels. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_entpanels_enable_names, "1", FLAG_HUD_CVAR, "Enables drawing the entity's name on entity panels. 0 = OFF, 1 = ON\n");


static MAKE_TOGGLE_CONVAR(mom_entpanels_fade_enable, "1", FLAG_HUD_CVAR, "Fades entity panels after a certain distance, over a certain distance. 0 = OFF, 1 = ON\n");
static MAKE_CONVAR(mom_entpanels_fade_dist, "100", FLAG_HUD_CVAR, "The amount of units to linearly fade the entity panel over.\n", 1, MAX_TRACE_LENGTH);
static MAKE_CONVAR(mom_entpanels_fade_start, "4096", FLAG_HUD_CVAR, "The distance (in units) at which entity panels start to fade.\n", 1, MAX_TRACE_LENGTH);

CGhostEntityPanel::CGhostEntityPanel() : Panel(g_pClientMode->GetViewport()), m_pEntity(nullptr), m_pAvatarImage(nullptr)
{
    SetPaintBackgroundEnabled(false);

    m_pAvatarImagePanel = new vgui::ImagePanel(this, "GhostEntityPanelAvatar");
    m_pAvatarImage = new CAvatarImage();
    m_pNameLabel = new vgui::Label(this, "GhostEntityPanelName", "");
    m_pNameLabel->SetAutoWide(true);
    m_pAvatarImage->SetDrawFriend(true);
    m_pAvatarImage->SetAvatarSize(32, 32);
    m_pAvatarImagePanel->SetImage(m_pAvatarImage);
    m_pAvatarImagePanel->SetSize(32, 32);
    SetVisible(false);
    SetSize(150, 60);
}

void CGhostEntityPanel::Init(C_MomentumOnlineGhostEntity* pEnt)
{
    m_pEntity = pEnt;
    m_OffsetX = -75;
    m_OffsetY = -40;
    m_iOrgWidth = 128;
    m_iOrgHeight = 128;
    m_iOrgOffsetX = m_OffsetX;
    m_iOrgOffsetY = m_OffsetY;
    
    SetVisible(true);
    vgui::ivgui()->AddTickSignal(GetVPanel());
}

void CGhostEntityPanel::OnThink()
{
    // Update our current position
    int sx, sy;
    GetEntityPosition(sx, sy);

    // Recalculate our size
    ComputeAndSetSize();

    // Set the position
    SetPos(static_cast<int>(sx + m_OffsetX + 0.5f), static_cast<int>(sy + m_OffsetY + 0.5f));


    if (m_pEntity)
    {
        if (!m_pAvatarImage->IsValid() && m_pEntity->m_SteamID.IsValid())
        {
            m_pAvatarImage->SetAvatarSteamID(m_pEntity->m_SteamID, k_EAvatarSize32x32);
        }

        // MOM_TODO: Blink the panel if they're typing? Maybe an icon or something? Idk

        if (m_bPaintName)
        {
            char check[MAX_PLAYER_NAME_LENGTH];
            m_pNameLabel->GetText(check, MAX_PLAYER_NAME_LENGTH);
            if (!FStrEq(check, m_pEntity->m_pszGhostName))
                m_pNameLabel->SetText(m_pEntity->m_pszGhostName);
        }
        m_pNameLabel->SetVisible(m_bPaintName);
    }

    BaseClass::OnThink();
}

void CGhostEntityPanel::OnTick()
{
    bool shouldDraw = ShouldDraw();
    if (shouldDraw != IsVisible())
    {
        SetVisible(shouldDraw);

        if (!shouldDraw)
            return;
    }

    bool isCursorOver = IsCursorOver();
    bool areNamesEnabled = mom_entpanels_enable_names.GetBool();
    
    if (areNamesEnabled)
    {
        SetPaintBackgroundEnabled(isCursorOver);
        m_bPaintName = isCursorOver;
    }
    
    m_pAvatarImagePanel->SetVisible(mom_entpanels_enable_avatars.GetBool());


    if (mom_entpanels_fade_enable.GetBool() && m_pEntity)
    {
        // Get distance to entity
        float flDistance = (m_pEntity->GetRenderOrigin() - MainViewOrigin()).Length();

        float flFadeStart = mom_entpanels_fade_start.GetFloat();
        float flFadeDist = mom_entpanels_fade_dist.GetFloat();

        if (flDistance > flFadeStart)
        {
            float flDiff = flDistance - flFadeStart;

            if (flDiff > flFadeDist)
                SetVisible(false);
            else
            {
                SetVisible(true);
                SetAlpha(static_cast<int>(255.0f * (1.0f - flDiff / flFadeDist)));
            }  
        }
        else
        {
            SetVisible(true);
            SetAlpha(255);
        }
    }
}

bool CGhostEntityPanel::ShouldDraw()
{
    return mom_entpanels_enable.GetBool() && (mom_entpanels_enable_avatars.GetBool() || mom_entpanels_enable_names.GetBool());
}


void CGhostEntityPanel::GetEntityPosition(int& sx, int& sy)
{
    if (!m_pEntity)
    {
        sx = sy = -1.0f;
        return;
    }
    
    GetVectorInScreenSpace(m_pEntity->GetAbsOrigin() + 
        (m_pEntity->GetModelPtr() ? m_pEntity->GetModelPtr()->eyeposition() : Vector(0, 0, 62)), 
        sx, sy, nullptr);
}

void CGhostEntityPanel::ComputeAndSetSize()
{
    m_flScale = 1.0;


    if (mom_entpanels_enable_names.GetBool())
    {
        int panelWide, panelHigh;
        GetSize(panelWide, panelHigh);

        vgui::HFont font = m_pNameLabel->GetFont();
        wchar_t playerName[MAX_PLAYER_NAME_LENGTH];
        ANSI_TO_UNICODE(m_pEntity->m_pszGhostName, playerName);
        int nameWide = UTIL_ComputeStringWidth(font, playerName) + 6;
        int fontHeight = vgui::surface()->GetFontTall(font);

        m_pAvatarImagePanel->SetPos((panelWide / 2) - 16, 0);

        m_pNameLabel->SetPos(panelWide / 2 - (nameWide / 2), mom_entpanels_enable_avatars.GetBool() ? 33 : (panelHigh / 2 - fontHeight / 2) - 4);

    }
}

CGhostEntityPanel::~CGhostEntityPanel()
{
    if (m_pAvatarImage)
    {
        m_pAvatarImage->ClearAvatarSteamID();
        delete m_pAvatarImage;
    }

    if (m_pAvatarImagePanel)
    {
        m_pAvatarImagePanel->DeletePanel();
    }

    if (m_pNameLabel)
    {
        m_pNameLabel->DeletePanel();
    }
        
    m_pAvatarImage = nullptr;
    m_pAvatarImagePanel = nullptr;
    m_pNameLabel = nullptr;
}
