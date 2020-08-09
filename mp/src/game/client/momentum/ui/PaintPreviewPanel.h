#pragma once

#include <vgui_controls/EditablePanel.h>

class PaintPreviewPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(PaintPreviewPanel, vgui::EditablePanel);

  public:
    PaintPreviewPanel(Panel *parent);
    ~PaintPreviewPanel();

    void Paint() override;

  private:
    int m_iDecalTextureID;
};
