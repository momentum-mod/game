#pragma once

#include "cbase.h"

#include <vgui_controls/ScrollableEditablePanel.h>
#include "vgui_controls/PropertyPage.h"

class SettingsPage : public vgui::PropertyPage
{
    DECLARE_CLASS_SIMPLE(SettingsPage, vgui::PropertyPage);

    SettingsPage(Panel *pParent, const char *pName);

    ~SettingsPage() {}

    // Let the PropertyDialog know that something changed on me, so the Apply button can be enabled again.
    // This is passed through the scroll panel because the scroll panel overrides this class's parent,
    // yet holds the actual parent in itself.
    void NotifyParentOfUpdate();

    // Update the parent (PropertyDialog) page
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel) { NotifyParentOfUpdate(); }

    // Update the parent (PropertyDialog) page
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel) { NotifyParentOfUpdate(); }

    // This can be sent from ComboBoxes or any TextEntry
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel) { NotifyParentOfUpdate(); }

    // MOM_TODO: Add more message funcs if need be (other controls added)

    // When the "Apply" button is pressed. Each settings panel should handle this separately.
    // Due to different controls for each, which cannot be automated through here.
    void OnApplyChanges() OVERRIDE;

    // Called when this page needs to load the current values into controls on the page.
    // This is primarily used for ComboBoxes, since there is no ConvarComboBox class.
    virtual void LoadSettings()
    {
        // Designed to be overridden.
    }

    virtual void OnMainDialogClosed()
    {
        // Designed to be overridden
    }

    virtual void OnMainDialogShow()
    {
        // Designed to be overridden
    }

    vgui::ScrollableEditablePanel *GetScrollPanel() const { return m_pScrollPanel; }

    // Load the panel's settings
    virtual void OnPageShow() OVERRIDE { LoadSettings(); }
    virtual void OnResetData() OVERRIDE { LoadSettings(); }
    
  private:
    vgui::ScrollableEditablePanel *m_pScrollPanel;
};

class SettingsPageScrollPanel : public vgui::ScrollableEditablePanel
{
    DECLARE_CLASS_SIMPLE(SettingsPageScrollPanel, ScrollableEditablePanel);

    SettingsPageScrollPanel(Panel *pParent, EditablePanel *pChild, const char *pName) : BaseClass(pParent, pChild, pName)
    {
        m_pChild = dynamic_cast<SettingsPage*>(pChild);
    }

    //These are needed because the PropertyDialog fires everything to us,
    //since it displays us, it thinks we are the PropertyPage, so we must
    //forward the messages.
    MESSAGE_FUNC(OnDataReset, "ResetData")
    {
        if (m_pChild)
        {
            m_pChild->OnResetData();
        }
    }
    MESSAGE_FUNC(OnPageShow, "PageShow")
    {
        if (m_pChild)
        {
            m_pChild->OnPageShow();
        }
    }

    MESSAGE_FUNC(OnPageHide, "PageHide")
    {
        if (m_pChild)
        {
            m_pChild->OnPageHide();
        }
    }

    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges")
    {
        if (m_pChild)
        {
            m_pChild->OnApplyChanges();
        }
    }

    MESSAGE_FUNC_PTR(OnPageTabActivated, "PageTabActivated", panel)
    {
        if (m_pChild)
        {
            //Thanks Valve for making the OnPageTabActivated void in PropertyPage protected!
            KeyValues *msg = new KeyValues("PageTabActivated");
            msg->SetPtr("panel", panel);
            PostMessageToChild(m_pChild->GetName(), msg);
        }
    }

    MESSAGE_FUNC(MainDialogClosed, "OnMainDialogClosed")
    {
        if (m_pChild)
            m_pChild->OnMainDialogClosed();
    }

    MESSAGE_FUNC(MainDialogShown, "OnMainDialogShow")
    {
        if (m_pChild)
            m_pChild->OnMainDialogShow();
    }

private:
    SettingsPage *m_pChild;

};