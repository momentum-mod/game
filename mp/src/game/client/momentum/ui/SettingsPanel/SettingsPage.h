#pragma once

#include "cbase.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ScrollableEditablePanel.h>
#include <vgui_controls/pch_vgui_controls.h>

using namespace vgui;

class SettingsPage : public PropertyPage
{
    DECLARE_CLASS_SIMPLE(SettingsPage, PropertyPage);

    SettingsPage(Panel *pPanel, const char *pName) : BaseClass(pPanel, pName)
    {
        // Set up the res file
        char m_pszResFilePath[MAX_PATH];
        Q_snprintf(m_pszResFilePath, MAX_PATH, "resource/ui/SettingsPanel_%s.res", pName);

        SetProportional(true);
        LoadControlSettingsAndUserConfig(m_pszResFilePath);

        m_pScrollPanel = new ScrollableEditablePanel(pPanel, this, "ScrollablePanel");
        m_pScrollPanel->SetProportional(true);
    }

    ~SettingsPage()
    {
        if (m_pScrollPanel)
            m_pScrollPanel->DeletePanel();
        m_pScrollPanel = nullptr;
    }

    // Let the PropertyDialog know that something changed on me, so the Apply button can be enabled again.
    // This is passed through the scroll panel because the scroll panel overrides this class's parent,
    // yet holds the actual parent in itself.
    void NotifyParentOfUpdate()
    {
        // Note: Clicking the "apply" button should handle setting the values.
        PostMessage(m_pScrollPanel->GetParent()->GetVPanel(), new KeyValues("ApplyButtonEnable"));
    }

    // Update the parent (PropertyDialog) page
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel) { NotifyParentOfUpdate(); }

    // This can be sent from ComboBoxes or any TextEntry
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel) { NotifyParentOfUpdate(); }

    // MOM_TODO: Add more message funcs if need be (other controls added)

    // When the "Apply" button is pressed. Each settings panel should handle this separately.
    // Due to different comboboxes for each, which cannot be automated through here.
    void OnApplyChanges() override
    {
        // We're going to fire this to all our children, so the CVarToggleCheckButtons apply
        // their new settings to the convars.
        for (int i = 0; i < GetChildCount(); i++)
        {
            Panel *pChild = GetChild(i);
            if (pChild)
            {
                PostMessage(pChild, new KeyValues("ApplyChanges"));
            }
        }
    }

    // Called when this page needs to load the current values into controls on the page.
    // This is primarily used for ComboBoxes, since there is no ConvarComboBox class.
    virtual void LoadSettings()
    {
        // Designed to be overridden.
    }

    ScrollableEditablePanel *GetScrollPanel() const { return m_pScrollPanel; }

  protected:
    // Load the panel's settings
    void OnPageShow() override { LoadSettings(); }
    void OnResetData() override { LoadSettings(); }

  private:
    ScrollableEditablePanel *m_pScrollPanel;
};