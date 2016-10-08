#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

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

    // player controls
    ToggleButton *m_pPlayPauseResume;
    Button *m_pGoStart;
    Button *m_pGoEnd;
    Button *m_pPrevFrame;
    Button *m_pNextFrame;
    PFrameButton *m_pFastForward;
    PFrameButton *m_pFastBackward;
    Button *m_pGo;

    TextEntry *m_pGoToTimeScale;
    Button *m_pGoTimeScale;

    ProgressBar *m_pProgress;
    Label *m_pProgressLabelFrame;
    Label *m_pProgressLabelTime;

    TextEntry *m_pGotoTick;
};