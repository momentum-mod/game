#include "hudelement.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#ifdef _WIN32
#pragma once
#endif

enum Selections
{
    RUI_NOTHING,
    RUI_MOVEBW,
    RUI_MOVEFW,
};

class CHudReplay : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CHudReplay, vgui::Frame);
    CHudReplay(const char *pElementName);

    virtual void OnThink();

    // Command issued
    virtual void OnCommand(const char *command);

    // player controls
    vgui::Button *m_pPlayPauseResume;
    vgui::Button *m_pGoStart;
    vgui::Button *m_pGoEnd;
    vgui::Button *m_pPrevFrame;
    vgui::Button *m_pNextFrame;
    vgui::Button *m_pFastForward;
    vgui::Button *m_pFastBackward;
    vgui::Button *m_pGo;

    vgui::TextEntry *m_pGotoTick2;
    vgui::Button *m_pGo2;

    vgui::ProgressBar *m_pProgress;
    vgui::Label *m_pProgressLabelFrame;
    vgui::Label *m_pProgressLabelTime;

    vgui::TextEntry *m_pGotoTick;
};