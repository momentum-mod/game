#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>
#include "CVarSlider.h"

enum Selections
{
    RUI_NOTHING,
    RUI_MOVEBW,
    RUI_MOVEFW,
};

class CHudReplay : public Frame
{
    DECLARE_CLASS_SIMPLE(CHudReplay, vgui::Frame);
    CHudReplay(const char *pElementName);

    virtual void OnThink() OVERRIDE;

    // Command issued
    virtual void OnCommand(const char *command) OVERRIDE;

    // When the slider changes, we want to update the text panel
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);

    // When the text entry updates, we want to update the slider
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

    // player controls
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

    ProgressBar *m_pProgress;
    Label *m_pProgressLabelFrame;
    Label *m_pProgressLabelTime;

    TextEntry *m_pGotoTick;
};