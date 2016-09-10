#include "hudelement.h"
#include <vgui_controls/Panel.h>

#ifdef _WIN32
#pragma once
#endif

namespace vgui
{
class Button;
class CheckButton;
class Label;
class ProgressBar;
class FileOpenDialog;
class Slider;
};

class CHudReplay : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CHudReplay, vgui::Frame);

  public:
    CHudReplay(const char *pElementName);

    virtual void OnTick();

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

    vgui::ProgressBar *m_pProgress;
    vgui::Label *m_pProgressLabelFrame;
    vgui::Label *m_pProgressLabelTime;

    vgui::Slider *m_pSpeedScale;
    vgui::Label *m_pSpeedScaleLabel;
    vgui::TextEntry *m_pGotoTick;
};
extern CHudReplay *HudReplay;
