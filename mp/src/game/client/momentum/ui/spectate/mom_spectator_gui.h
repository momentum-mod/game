#pragma once

#include "GameEventListener.h"
#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>

class C_MOMReplayUI;

//-----------------------------------------------------------------------------
// Purpose: Spectator UI
//-----------------------------------------------------------------------------
class CMOMSpectatorGUI : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CMOMSpectatorGUI, vgui::EditablePanel);

  public:
    CMOMSpectatorGUI(IViewPort *pViewPort);
    virtual ~CMOMSpectatorGUI(){};

    const char *GetName(void) OVERRIDE { return PANEL_SPECGUI; }
    void SetData(KeyValues *data) OVERRIDE{};
    void Reset() OVERRIDE{};
    void Update() OVERRIDE;
    bool NeedsUpdate(void) OVERRIDE { return false; }
    bool HasInputElements(void) OVERRIDE { return false; }
    void ShowPanel(bool bShow) OVERRIDE;

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); }
    void OnThink() OVERRIDE;

    void SetMouseInputEnabled(bool bState) OVERRIDE;

    int GetTopBarHeight() { return m_pTopBar->GetTall(); }

    void FireGameEvent(IGameEvent* pEvent) OVERRIDE;

  protected:
    // vgui overrides
    void PerformLayout() OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;

  private:
    Panel *m_pTopBar;

    vgui::Label *m_pPlayerLabel;
    vgui::Label *m_pReplayLabel;
    vgui::Label *m_pGainControlLabel;
    vgui::Label *m_pTimeLabel;

    Color m_cBarColor;

    vgui::ImagePanel *m_pCloseButton, *m_pShowControls, *m_pNextPlayerButton, *m_pPrevPlayerButton;

    C_MOMReplayUI *m_pReplayControls;

    IViewPort *m_pViewPort;
    bool m_bSpecScoreboard;

    float m_flNextUpdateTime;

    wchar_t m_pwGainControl[128], m_pwWatchingReplay[128], m_pwRunTime[128],
        m_pwSpecMap[128], m_pwWatchingGhost[128];
};