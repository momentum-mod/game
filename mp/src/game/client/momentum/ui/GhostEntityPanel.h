#pragma once

#include <vgui_controls/EditablePanel.h>

class C_MomentumOnlineGhostEntity;
class CAvatarImage;

class CGhostEntityPanel : public vgui::EditablePanel
{
  public:
    DECLARE_CLASS_SIMPLE(CGhostEntityPanel, vgui::EditablePanel);

    CGhostEntityPanel();
    ~CGhostEntityPanel();

    void Init(C_MomentumOnlineGhostEntity *pEntity);
    void OnThink() override;
    void OnTick() override;

    bool ShouldDraw();

    void SetShouldDrawEntityName(bool bState) { m_bPaintName = bState; }

    bool GetEntityPosition(int& sx, int& sy);

    CPanelAnimationVar(int, m_iOffsetY, "OffsetY", "-60");

  private:
    bool m_bPaintName;

    C_MomentumOnlineGhostEntity *m_pEntity;
    vgui::Label *m_pNameLabel;
    vgui::ImagePanel *m_pAvatarImagePanel;
    CAvatarImage *m_pAvatarImage;

    // Position of the panel
    int m_iPosX, m_iPosY;
};
