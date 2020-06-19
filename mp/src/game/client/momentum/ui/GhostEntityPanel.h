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
    void OnThink() OVERRIDE;
    void OnTick() OVERRIDE;

    bool ShouldDraw();

    void SetShouldDrawEntityName(bool bState) { m_bPaintName = bState; }

    bool GetEntityPosition(int& sx, int& sy);

    // Offset from entity that we should draw
    CPanelAnimationVar(int, m_OffsetX, "OffsetX", "-74");
    CPanelAnimationVar(int, m_OffsetY, "OffsetY", "-60");

private:

    bool m_bPaintName;

    C_MomentumOnlineGhostEntity *m_pEntity;
    vgui::Label *m_pNameLabel;
    vgui::ImagePanel *m_pAvatarImagePanel;
    CAvatarImage *m_pAvatarImage;

    // Position of the panel
    int m_iPosX, m_iPosY;
};
