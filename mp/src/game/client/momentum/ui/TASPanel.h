#pragma once

#include "cbase.h"

#include <vgui_controls/Frame.h>

namespace vgui
{

class CTASPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CTASPanel, Frame);

  public:
    CTASPanel();
    ~CTASPanel();

    void OnThink() OVERRIDE;
    void OnCommand(const char *pCommand) OVERRIDE;
    void ToggleVisible();
    void VisPredMovements();
    void RunVPM(CBasePlayer* pPlayer);

  private:
    bool m_bToggleVisible, m_bStopVPM_OnGround;
    vgui::ToggleButton* m_pEnableTASMode;
    float m_flVPMTime, m_flOldCurtime, m_flOldFrametime;
};

};