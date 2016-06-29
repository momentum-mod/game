#pragma once

#include "cbase.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ScrollableEditablePanel.h>
#include <vgui_controls/pch_vgui_controls.h>

using namespace vgui;

class SettingsPageScrollPanel : public ScrollableEditablePanel
{
    DECLARE_CLASS_SIMPLE(SettingsPageScrollPanel, ScrollableEditablePanel);

    SettingsPageScrollPanel(Panel *pParent, EditablePanel *pChild, const char *pName) :
        BaseClass(pParent, pChild, pName)
    {
        m_pChild = dynamic_cast<PropertyPage*>(pChild);
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

private:
    PropertyPage *m_pChild;

};


class SettingsPage : public PropertyPage
{
    DECLARE_CLASS_SIMPLE(SettingsPage, PropertyPage);

    SettingsPage(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
    {
        SetAutoDelete(true);

        // Set proportionality of the panels inside the dialog
        SetProportional(true);

        // Set up the res file
        char m_pszResFilePath[MAX_PATH];
        Q_snprintf(m_pszResFilePath, MAX_PATH, "resource/ui/SettingsPanel_%s.res", pName);
        LoadControlSettingsAndUserConfig(m_pszResFilePath);

        //Lastly, the scroll panel so we can scroll through our settings page.
        m_pScrollPanel = new SettingsPageScrollPanel(pParent, this, "ScrollablePanel");
        m_pScrollPanel->AddActionSignalTarget(this);
        m_pScrollPanel->SetProportional(true);
        m_pScrollPanel->SetAutoDelete(true);
    }

    ~SettingsPage()
    {
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
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel) { NotifyParentOfUpdate(); }

    // Update the parent (PropertyDialog) page
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel) { NotifyParentOfUpdate(); }

    // This can be sent from ComboBoxes or any TextEntry
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel) { NotifyParentOfUpdate(); }

    // MOM_TODO: Add more message funcs if need be (other controls added)

    // When the "Apply" button is pressed. Each settings panel should handle this separately.
    // Due to different controls for each, which cannot be automated through here.
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