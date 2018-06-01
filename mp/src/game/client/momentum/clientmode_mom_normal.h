//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined(CLIENTMODE_MOM_NORM_H)
#define CLIENTMODE_MOM_NORM_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Cursor.h>
#include <vgui_controls/EditablePanel.h>
#include "ClientTimesDisplay.h"
#include "clientmode_shared.h"
#include "hud_mapfinished.h"
#include "hud_menu_static.h"
#include "momSpectatorGUI.h"

class CHudViewport;

namespace vgui
{
typedef unsigned long HScheme;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class ClientModeMOMNormal : public ClientModeShared
{
  public:
    DECLARE_CLASS(ClientModeMOMNormal, ClientModeShared);

    ClientModeMOMNormal();
    ~ClientModeMOMNormal();

    void Init() OVERRIDE;
    bool ShouldDrawCrosshair(void) OVERRIDE;
    // NOTE: This includes mouse inputs!!!
    int HudElementKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;
    int HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;

    void SetupPointers();

  public:
    CHudMenuStatic *m_pHudMenuStatic;
    CHudMapFinishedDialog *m_pHudMapFinished;
    CClientTimesDisplay *m_pLeaderboards;
    CMOMSpectatorGUI *m_pSpectatorGUI;
};

extern IClientMode *GetClientModeNormal();
extern vgui::HScheme g_hVGuiCombineScheme;

#endif // CLIENTMODE_MOM_NORMAL
