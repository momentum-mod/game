#pragma once

#include "cbase.h"

#include <vgui_controls/Frame.h>

class C_MomentumPlayer;

namespace vgui
{
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
    float m_flVPMTime, m_flOldFrametime, m_flOldCurtime;
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
    CTASVisPanel *m_pVisualPanel;

  private:
    vgui::ToggleButton *m_pEnableTASMode;
};

extern CTASPanel *g_pTASPanel;

}; // namespace vgui