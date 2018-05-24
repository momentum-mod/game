//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws the normal TF2 or HL2 HUD.
//
//=============================================================================
#include "cbase.h"

#include "ClientTimesDisplay.h"
#include "clientmode_mom_normal.h"
#include "hud.h"
#include "ienginevgui.h"
#include "momSpectatorGUI.h"
#include "momentum/mom_shareddefs.h"
#include "IGameUIFuncs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool g_bRollingCredits;

using namespace vgui;

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
HScheme g_hVGuiCombineScheme = 0;

// Instance the singleton and expose the interface to it.
IClientMode *GetClientModeNormal()
{
    static ClientModeMOMNormal g_ClientModeNormal;
    return &g_ClientModeNormal;
}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
  private:
    DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

  protected:
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        BaseClass::ApplySchemeSettings(pScheme);

        gHUD.InitColors(pScheme);
        
        SetPaintBackgroundEnabled(false);
    }

    IViewPortPanel *CreatePanelByName(const char *pzName) OVERRIDE
    {

        if (!Q_strcmp(PANEL_TIMES, pzName))
        {
            return new CClientTimesDisplay(this);
        }
        if (!Q_strcmp(PANEL_SPECGUI, pzName))
        {
            return new CMOMSpectatorGUI(this);
        }
        if (!Q_strcmp(PANEL_REPLAY, pzName))
        {
            return new C_MOMReplayUI(this);
        }

        return BaseClass::CreatePanelByName(pzName);
    }

    void CreateDefaultPanels(void) OVERRIDE
    {
        AddNewPanel(CreatePanelByName(PANEL_REPLAY), "PANEL_REPLAY");
        AddNewPanel(CreatePanelByName(PANEL_TIMES), "PANEL_TIMES");
        AddNewPanel(CreatePanelByName(PANEL_SPECGUI), "PANEL_SPECGUI");
        //BaseClass::CreateDefaultPanels(); // MOM_TODO: do we want the other panels?
    }
};

//-----------------------------------------------------------------------------
// ClientModeHLNormal implementation
//-----------------------------------------------------------------------------
ClientModeMOMNormal::ClientModeMOMNormal()
{
    m_pHudMenuStatic = nullptr;
    m_pHudMapFinished = nullptr;
    m_pLeaderboards = nullptr;
    m_pSpectatorGUI = nullptr;
    m_pViewport = new CHudViewport();
    m_pViewport->Start(gameuifuncs, gameeventmanager);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
ClientModeMOMNormal::~ClientModeMOMNormal()
{
    //MOM_TODO: delete pointers (m_pViewport) here?
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ClientModeMOMNormal::Init()
{
    BaseClass::Init();
    SetupPointers();
    // Load up the combine control panel scheme
    g_hVGuiCombineScheme = scheme()->LoadSchemeFromFileEx(
        enginevgui->GetPanel(PANEL_CLIENTDLL),
        IsXbox() ? "resource/ClientScheme.res" : "resource/CombinePanelScheme.res", "CombineScheme");
    if (!g_hVGuiCombineScheme)
    {
        Warning("Couldn't load combine panel scheme!\n");
    }
}

bool ClientModeMOMNormal::ShouldDrawCrosshair(void) { return (g_bRollingCredits == false); }

int ClientModeMOMNormal::HudElementKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding)
{
    //Swallow the key input if a hud menu is open
    if (m_pHudMenuStatic && m_pHudMenuStatic->IsMenuDisplayed())
    {
        if (down >= 1 && keynum >= KEY_0 && keynum <= KEY_9)
        {
            m_pHudMenuStatic->SelectMenuItem(keynum - KEY_0);
            return 0; // The hud menu static swallowed the key input
        }
    }

    //Detach the mouse if the user right-clicked while the leaderboards are open
    if (m_pLeaderboards && m_pLeaderboards->IsVisible())
    {
        if (keynum == MOUSE_RIGHT)
        {
            m_pLeaderboards->SetMouseInputEnabled(true);
            //MOM_TODO: Consider toggling the leaderboards open with this
            //m_pLeaderboards->SetKeyBoardInputEnabled(!prior);
            //ButtonCode_t close = gameuifuncs->GetButtonCodeForBind("showtimes");
            //gViewPortInterface->PostMessageToPanel(PANEL_TIMES, new KeyValues("PollHideCode", "code", close));
            return 0;
        }
    }

    //Detach the mouse if the user right-clicked while the map finished dialog is open
    if (m_pHudMapFinished && m_pHudMapFinished->IsVisible())
    {
        if (keynum == MOUSE_RIGHT)
        {
            m_pHudMapFinished->SetMouseInputEnabled(true);
            return 0;
        }
    }

    return BaseClass::HudElementKeyInput(down, keynum, pszCurrentBinding);
}

int ClientModeMOMNormal::HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding)
{
    if (m_pSpectatorGUI)
    {
        // we are in spectator mode, open spectator menu
        if (down && pszCurrentBinding && !Q_strcmp(pszCurrentBinding, "+duck"))
        {
            m_pSpectatorGUI->SetMouseInputEnabled(!m_pSpectatorGUI->IsMouseInputEnabled());
            // MOM_TODO: re-enable this in alpha+ when we add movie-style controls to the spectator menu!
            //m_pViewport->ShowPanel(PANEL_SPECMENU, true);

            return 0; // we handled it, don't handle twice or send to server
        }

        if (down && pszCurrentBinding && !Q_strcmp(pszCurrentBinding, "+attack") && !m_pSpectatorGUI->IsMouseInputEnabled())
        {
            engine->ClientCmd("spec_next");
            return 0;
        }
        else if (down && pszCurrentBinding && !Q_strcmp(pszCurrentBinding, "+attack2") && !m_pSpectatorGUI->IsMouseInputEnabled())
        {
            engine->ClientCmd("spec_prev");
            return 0;
        }
        else if (down && pszCurrentBinding && !Q_strcmp(pszCurrentBinding, "+jump"))
        {
            engine->ClientCmd("spec_mode");
            return 0;
        }
    }

    return 1;
}

void ClientModeMOMNormal::SetupPointers()
{
    m_pHudMenuStatic = GET_HUDELEMENT(CHudMenuStatic);
    m_pHudMapFinished = GET_HUDELEMENT(CHudMapFinishedDialog);
    m_pLeaderboards = dynamic_cast<CClientTimesDisplay*>(m_pViewport->FindPanelByName(PANEL_TIMES));
    m_pSpectatorGUI = dynamic_cast<CMOMSpectatorGUI*>(m_pViewport->FindPanelByName(PANEL_SPECGUI));
}
