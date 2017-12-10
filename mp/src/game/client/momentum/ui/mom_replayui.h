#pragma once

#include "cbase.h"

#include "PFrameButton.h"
#include "game/client/iviewport.h"
#include "momentum/mom_shareddefs.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/ScrubbableProgressBar.h>

enum Selections
{
    RUI_NOTHING,
    RUI_MOVEBW,
    RUI_MOVEFW,
};

class C_MOMReplayUI : public vgui::Frame, public IViewPortPanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(C_MOMReplayUI, vgui::Frame);
    C_MOMReplayUI(IViewPort *pViewport);

    virtual void OnThink() OVERRIDE;

    // Command issued
    virtual void OnCommand(const char *command) OVERRIDE;

    void SetLabelText() const;

    virtual const char *GetName(void) OVERRIDE { return PANEL_REPLAY; }
    virtual void SetData(KeyValues *data) OVERRIDE {}
    virtual void Reset(void) OVERRIDE{}; // clears internal state, deactivates it
    virtual void Update(void) OVERRIDE{};
    virtual bool NeedsUpdate(void) OVERRIDE { return false; }
    virtual bool HasInputElements(void) OVERRIDE { return true; }

    virtual void ShowPanel(bool state) OVERRIDE; // activate VGUI Frame

    virtual void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    // VGUI functions:
    virtual vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }
    virtual bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }; // true if panel is visible
    virtual void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); };

  protected:
    // When the slider changes, we want to update the text panel
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);

    // When the text entry updates, we want to update the slider
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

    // When the user clicks (and potentially drags) on the Progress Bar
    MESSAGE_FUNC_FLOAT(OnNewProgress, "ScrubbedProgress", scale);

    // When the user scrolls on the Progress Bar
    MESSAGE_FUNC_INT(OnPBMouseWheeled, "PBMouseWheeled", delta);

  private:
    // player controls
    vgui::ToggleButton *m_pPlayPauseResume;
    vgui::Button *m_pGoStart;
    vgui::Button *m_pGoEnd;
    vgui::Button *m_pPrevFrame;
    vgui::Button *m_pNextFrame;
    vgui::PFrameButton *m_pFastForward;
    vgui::PFrameButton *m_pFastBackward;
    vgui::Button *m_pGo;

    vgui::CCvarSlider *m_pTimescaleSlider;
    vgui::TextEntry *m_pTimescaleEntry;
    vgui::Label *m_pTimescaleLabel;

    vgui::ScrubbableProgressBar *m_pProgress;
    vgui::Label *m_pProgressLabelFrame;
    vgui::Label *m_pProgressLabelTime;

    vgui::TextEntry *m_pGotoTick;

    IViewPort *m_pViewport;

    int m_iTotalDuration, m_iPlayButtonSelected;

    bool m_bWasVisible;

    wchar_t m_pwReplayTime[BUFSIZELOCL], m_pwReplayTimeTick[BUFSIZELOCL];
};