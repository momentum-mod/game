//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include "GameConsole.h"
#include "GameConsoleDialog.h"
#include "vgui/ISurface.h"

#include "KeyValues.h"
#include "convar.h"
#include "vgui/VGUI.h"
#include "vgui_controls/Panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CGameConsole g_GameConsole;
//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
CGameConsole &GameConsole() { return g_GameConsole; }
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameConsole, IGameConsole, GAMECONSOLE_INTERFACE_VERSION, g_GameConsole);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGameConsole::CGameConsole(): m_pConsole(nullptr) { m_bInitialized = false; }

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGameConsole::~CGameConsole() { m_bInitialized = false; }

//-----------------------------------------------------------------------------
// Purpose: sets up the console for use
//-----------------------------------------------------------------------------
void CGameConsole::Initialize()
{
    m_pConsole = vgui::SETUP_PANEL(new CGameConsoleDialog()); // we add text before displaying this so set it up now!

    // set the console to taking up most of the right-half of the screen
    int swide, stall;
    vgui::surface()->GetScreenSize(swide, stall);
    int offset = vgui::scheme()->GetProportionalScaledValue(16);

    m_pConsole->SetBounds(swide / 2 - (offset * 4), offset, (swide / 2) + (offset * 3), stall - (offset * 8));

    m_bInitialized = true;
}

//-----------------------------------------------------------------------------
// Purpose: activates the console, makes it visible and brings it to the foreground
//-----------------------------------------------------------------------------
void CGameConsole::Activate()
{
    if (!m_bInitialized)
        return;

    vgui::surface()->RestrictPaintToSinglePanel(NULL);
    m_pConsole->Activate();
}

//-----------------------------------------------------------------------------
// Purpose: hides the console
//-----------------------------------------------------------------------------
void CGameConsole::Hide()
{
    if (!m_bInitialized)
        return;

    m_pConsole->Hide();
}

//-----------------------------------------------------------------------------
// Purpose: clears the console
//-----------------------------------------------------------------------------
void CGameConsole::Clear()
{
    if (!m_bInitialized)
        return;

    m_pConsole->Clear();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the console is currently in focus
//-----------------------------------------------------------------------------
bool CGameConsole::IsConsoleVisible()
{
    return false;
}

//-----------------------------------------------------------------------------
// Purpose: activates the console after a delay
//-----------------------------------------------------------------------------
void CGameConsole::ActivateDelayed(float time)
{
    if (!m_bInitialized)
        return;

    m_pConsole->PostMessage(m_pConsole, new KeyValues("Activate"), time);
}

void CGameConsole::SetParent(int parent)
{
    if (!m_bInitialized)
        return;

    m_pConsole->SetParent(static_cast<vgui::VPANEL>(parent));
}

//-----------------------------------------------------------------------------
// Purpose: static command handler
//-----------------------------------------------------------------------------
void CGameConsole::OnCmdCondump()
{
    if (m_pConsole)
        m_pConsole->DumpConsoleTextToFile();
}

CON_COMMAND(condump, "dump the text currently in the console to condumpXX.log")
{
    g_GameConsole.OnCmdCondump();
}