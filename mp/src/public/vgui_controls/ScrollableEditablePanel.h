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
    DECLARE_CLASS_SIMPLE(ScrollableEditablePanel, vgui::EditablePanel);

  public:
    ScrollableEditablePanel(Panel *pParent, EditablePanel *pChild, const char *pName);
    virtual ~ScrollableEditablePanel() {}

    virtual void ApplySettings(KeyValues *pInResourceData) OVERRIDE;
    virtual void PerformLayout() OVERRIDE;

    void ScrollToTop();

    ScrollBar *GetScrollbar(void) { return m_pScrollBar; }

    MESSAGE_FUNC(OnScrollBarSliderMoved, "ScrollBarSliderMoved");
    virtual void OnMouseWheeled(int delta); // respond to mouse wheel events

  private:
    ScrollBar *m_pScrollBar;
    EditablePanel *m_pChild;
};
}