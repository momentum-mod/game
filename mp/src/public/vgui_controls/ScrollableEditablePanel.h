#pragma once

#include "vgui_controls/EditablePanel.h"

namespace vgui
{
class ScrollBar;
//-----------------------------------------------------------------------------
// An editable panel that has a scrollbar
//-----------------------------------------------------------------------------
class ScrollableEditablePanel : public EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(ScrollableEditablePanel, vgui::EditablePanel);

    ScrollableEditablePanel(Panel *pParent, EditablePanel *pChild, const char *pName);
    virtual ~ScrollableEditablePanel() {}

    void SetChild(Panel *pChild);

    void ApplySettings(KeyValues *pInResourceData) override;
    void PerformLayout() override;

    void ScrollToTop();

    ScrollBar *GetScrollbar() const { return m_pScrollBar; }
    Panel *GetDirectChild() const { return m_pChild; }

protected:
    MESSAGE_FUNC(OnScrollBarSliderMoved, "ScrollBarSliderMoved");
    void OnMouseWheeled(int delta) override; // respond to mouse wheel events

  private:
    ScrollBar *m_pScrollBar;
    Panel *m_pChild;
};
}