#pragma once

#include "clientmode_shared.h"

class CHudViewport;
class CHudMenuStatic;
class CHudMapFinishedDialog;
class CMOMSpectatorGUI;
class CClientTimesDisplay;
class LobbyMembersPanel;
class TrickList;

namespace vgui
{
typedef unsigned long HScheme;
}

enum
{
    MD_NONE = 0,
    MD_Forwards,
    MD_Sideways,
    MD_Sideways2,
    MD_Backwards,
};

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
    // NOTE: This includes mouse inputs!!!
    int HudElementKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;
    int HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;

    // Stub
    void ComputeVguiResConditions(KeyValues *pkvConditions) override {}

    void SetupPointers();
    int MovementDirection(const QAngle viewangles, const Vector velocity);
    bool CreateMove(float flInputSampleTime, CUserCmd *cmd);

  public:
    CHudMenuStatic *m_pHudMenuStatic;
    CHudMapFinishedDialog *m_pHudMapFinished;
    CClientTimesDisplay *m_pLeaderboards;
    CMOMSpectatorGUI *m_pSpectatorGUI;
    LobbyMembersPanel *m_pLobbyMembers;
    TrickList *m_pTrickList;
};

extern IClientMode *GetClientModeNormal();
extern vgui::HScheme g_hVGuiCombineScheme;