#pragma once

#include <vgui_controls/EditablePanel.h>

class SettingsPanel : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(SettingsPanel, vgui::EditablePanel);

    SettingsPanel(Panel *pParent, const char *pName, vgui::Button *pAssosciate);

    // Load the panel's settings
    virtual void OnPageShow();
    virtual void OnPageHide();

    // Update the parent (PropertyDialog) page
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel) { }

    // This can be sent from ComboBoxes or any TextEntry
    MESSAGE_FUNC_PTR_CHARPTR(OnTextChanged, "TextChanged", panel, text) { }

private:
    vgui::Button *m_pAssociatedButton;
};