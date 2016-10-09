//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once

#include "GameEventListener.h"
#include <igameevents.h>
#include <vgui/IScheme.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Frame.h>

#include <game/client/iviewport.h>

class KeyValues;

namespace vgui
{
class TextEntry;
class Button;
class Panel;
class ImagePanel;
class ComboBox;
}

#define BLACK_BAR_COLOR Color(0, 0, 0, 196)

class IBaseFileSystem;

//-----------------------------------------------------------------------------
// Purpose: Spectator UI
//-----------------------------------------------------------------------------
class CMOMSpectatorGUI : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CMOMSpectatorGUI, vgui::EditablePanel);

  public:
    CMOMSpectatorGUI(IViewPort *pViewPort);
    virtual ~CMOMSpectatorGUI(){};

    const char *GetName(void) override { return PANEL_SPECGUI; }
    void SetData(KeyValues *data) override{};
    void Reset() override{};
    void Update() override;
    bool NeedsUpdate(void) override { return false; }
    bool HasInputElements(void) override { return false; }
    void ShowPanel(bool bShow) override;

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
    bool IsVisible() override { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) override { BaseClass::SetParent(parent); }
    void OnThink() override;

    void SetMouseInputEnabled(bool bState) override
    {
        BaseClass::SetMouseInputEnabled(bState);

        if (m_pGainControlLabel)
            m_pGainControlLabel->SetVisible(!bState);
    }

    int GetTopBarHeight() { return m_pTopBar->GetTall(); }

    Color GetBlackBarColor(void) { return BLACK_BAR_COLOR; }

    const char *GetResFile(void) const { return "resource/UI/Spectator.res"; }

    void FireGameEvent(IGameEvent *pEvent) override
    {
        if (!Q_strcmp(pEvent->GetName(), "spec_target_updated"))
        {
            // So apparently calling Update from here doesn't work, due to some weird
            // thing that happens upon the player's m_hObserverTarget getting updated.
            // Pushing this back three ticks is more than long enough to delay the Update()
            // to fill the panel with the replay's info.
            m_flNextUpdateTime = gpGlobals->curtime + gpGlobals->interval_per_tick * 3.0f;
        }
    }

protected:
    enum
    {
        INSET_OFFSET = 2
    };

    // vgui overrides
    void PerformLayout() override;
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnMousePressed(vgui::MouseCode code) override;

private:
    Panel *m_pTopBar;

    vgui::Label *m_pPlayerLabel;
    vgui::Label *m_pReplayLabel;
    vgui::Label *m_pGainControlLabel;
    vgui::Label *m_pMapLabel;
    vgui::Label *m_pTimeLabel;

    vgui::ImagePanel *m_pCloseButton;

    IViewPort *m_pViewPort;

    // bool m_bHelpShown;
    // bool m_bInsetVisible;
    bool m_bSpecScoreboard;

    float m_flNextUpdateTime;
};