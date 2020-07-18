#pragma once

#include <vgui_controls/EditablePanel.h>

#include "hudelement.h"

namespace vgui
{
    class ContinuousProgressBar;
}

class C_MomConcProjectile;
class C_MomentumConcGrenade;

class CHudConcTimer : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudConcTimer, EditablePanel);

    CHudConcTimer(const char *pElementName);

    bool ShouldDraw() override;
    void OnThink() override;
    void Reset() override;

  private:
    C_MomentumConcGrenade *m_pGrenade;
    vgui::ContinuousProgressBar *m_pTimer;
    vgui::Label *m_pTimerLabel;
};

DECLARE_HUDELEMENT(CHudConcTimer);

class CHudConcEntPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudConcEntPanel, vgui::Panel);

    CHudConcEntPanel();
    ~CHudConcEntPanel();

    void Init(C_MomConcProjectile *pEntity);
    void OnThink() override;
    void OnTick() override;

    bool ShouldDraw();

    bool GetEntityPosition(int &sx, int &sy);
    void ComputeAndSetSize();

  private:
    C_MomConcProjectile *m_pGrenade;
    vgui::ContinuousProgressBar *m_pHudTimer;
    vgui::Label *m_pTimerLabel;

    int m_iOrgWidth;
    int m_iOrgHeight;
    int m_iOrgOffsetX;
    int m_iOrgOffsetY;
    // Offset from entity that we should draw
    int m_OffsetX, m_OffsetY;
    // Position of the panel
    int m_iPosX, m_iPosY;
};