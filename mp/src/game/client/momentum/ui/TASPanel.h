#pragma once

#include "cbase.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ScrubbableProgressBar.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ToggleButton.h>
#include "PFrameButton.h"
#include "game/client/iviewport.h"
#include "momentum/mom_shareddefs.h"

class C_MomentumPlayer;

namespace vgui
{

class C_TASMOMReplayUI : public vgui::Frame, public IViewPortPanel
{
    DECLARE_CLASS_SIMPLE(C_TASMOMReplayUI, vgui::Frame);
    C_TASMOMReplayUI();

    virtual void OnThink() OVERRIDE;

    // Command issued
    virtual void OnCommand(const char *command) OVERRIDE;

    void SetLabelText() const;

    virtual const char *GetName(void) OVERRIDE { return "TASReplayControls"; }
    virtual void SetData(KeyValues *data) OVERRIDE {}
    virtual void Reset(void) OVERRIDE{}; // clears internal state, deactivates it
    virtual void Update(void) OVERRIDE{};
    virtual bool NeedsUpdate(void) OVERRIDE { return false; }
    virtual bool HasInputElements(void) OVERRIDE { return true; }

    virtual void ShowPanel(bool state) OVERRIDE; // activate VGUI Frame

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

    vgui::CvarSlider *m_pTimescaleSlider;
    vgui::TextEntry *m_pTimescaleEntry;
    vgui::Label *m_pTimescaleLabel;

    vgui::ScrubbableProgressBar *m_pProgress;
    vgui::Label *m_pProgressLabelFrame;
    vgui::Label *m_pProgressLabelTime;

    vgui::TextEntry *m_pGotoTick;

    int m_iTotalDuration, m_iPlayButtonSelected;

    wchar_t m_pwReplayTime[BUFSIZELOCL], m_pwReplayTimeTick[BUFSIZELOCL];
};

class CTASVisPanel : public Panel
{
    DECLARE_CLASS_SIMPLE(CTASVisPanel, Panel);

  public:
    CTASVisPanel();
    ~CTASVisPanel();

    void Paint() OVERRIDE;

    void VisPredMovements();
    void RunVPM(C_MomentumPlayer *pPlayer);

  private:
    float m_flVPMTime;
    CUtlVector<Vector> m_vecOrigins;
};

class CTASPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CTASPanel, Frame);

  public:
    CTASPanel();
    ~CTASPanel();

    void OnThink() OVERRIDE;
    void OnCommand(const char *pCommand) OVERRIDE;
    void ToggleVisible();
    void SetVisible(bool state) OVERRIDE;
    
    CTASVisPanel *m_pVisualPanel;
    C_TASMOMReplayUI *m_pReplayUI;

  private:
    vgui::ToggleButton *m_pEnableTASMode;
    vgui::ToggleButton *m_pVisPredMove;
    vgui::ToggleButton *m_pAutostrafe;
    ConVarRef mom_tas_autostrafe;

  public:
    vgui::ToggleButton *m_pToggleReplayUI;
};

extern CTASPanel *g_pTASPanel;

}; // namespace vgui