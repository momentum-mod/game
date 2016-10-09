#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>
#include "CVarSlider.h"
#include "ScrubbableProgressBar.h"

enum Selections
{
    RUI_NOTHING,
    RUI_MOVEBW,
    RUI_MOVEFW,
};

class C_ReplayUI : public Frame
{
    DECLARE_CLASS_SIMPLE(C_ReplayUI, vgui::Frame);
    C_ReplayUI(const char *pElementName);

    virtual void OnThink() OVERRIDE;

    // Command issued
    virtual void OnCommand(const char *command) OVERRIDE;

    // When the slider changes, we want to update the text panel
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);

    // When the text entry updates, we want to update the slider
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

    // When the user clicks (and potentially drags) on the Progress Bar
    MESSAGE_FUNC_FLOAT(OnNewProgress, "ScrubbedProgress", scale);

    // When the user scrolls on the Progress Bar
    MESSAGE_FUNC_PARAMS(OnMouseWheeled, "MouseWheeled", pKv);


    void SetLabelText() const;

    // player controls
private:
    ToggleButton *m_pPlayPauseResume;
    Button *m_pGoStart;
    Button *m_pGoEnd;
    Button *m_pPrevFrame;
    Button *m_pNextFrame;
    PFrameButton *m_pFastForward;
    PFrameButton *m_pFastBackward;
    Button *m_pGo;

    CCvarSlider *m_pTimescaleSlider;
    TextEntry *m_pTimescaleEntry;
    Label *m_pTimescaleLabel;

    ScrubbableProgressBar *m_pProgress;
    Label *m_pProgressLabelFrame;
    Label *m_pProgressLabelTime;

    TextEntry *m_pGotoTick;

    int m_iTotalDuration;

};