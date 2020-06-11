#pragma once

#include "SettingsPanel.h"

class VControlsListPanel;
namespace vgui
{
    class CvarToggleComboBox;
}

class InputSettingsPanel : public SettingsPanel
{
    DECLARE_CLASS_SIMPLE(InputSettingsPanel, SettingsPanel);

    InputSettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;

    VControlsListPanel *GetControlsList() { return m_pKeyBindList; }

protected:
    void OnKeyCodePressed(vgui::KeyCode code) override;
    void OnKeyCodeTyped(vgui::KeyCode code) override;
    void OnThink() override;
    void OnCommand(const char *command) override;

    void OnCheckboxChecked(Panel *panel) override;

    // Trap row selection message
    MESSAGE_FUNC_INT(ItemSelected, "ItemSelected", itemID);

private:
    // Mouse

    vgui::CvarToggleCheckButton *m_pReverseMouseCheckBox, *m_pMouseFilterCheckBox, *m_pMouseRawCheckbox;

    vgui::CvarSlider *m_pMouseSensitivitySlider;
    vgui::CvarTextEntry *m_pMouseSensitivityEntry;

    vgui::CheckButton *m_pMouseAccelToggle;
    vgui::CvarSlider *m_pMouseAccelSlider;
    vgui::CvarTextEntry *m_pMouseAccelEntry;

    ConVarRef m_cvarCustomAccel;

    // Keyboard

    VControlsListPanel *m_pKeyBindList;

    vgui::Button *m_pSetBindingButton;
    vgui::Button *m_pClearBindingButton;
    vgui::Button *m_pRevertToDefaultBindsButton;

    void Finish(ButtonCode_t code);

    // Get column 0 action descriptions for all keys
    void ParseActionDescriptions();

    void BindKey(const char *key, const char *binding);
    void UnbindKey(const char *key);
    void FillInCurrentBindings();
    void ClearBindItems();
    void FillInDefaultBindings();
    void AddBinding(KeyValues *item, const char *keyname);
    void RemoveKeyFromBindItems(KeyValues *org_item, const char *key);

    KeyValues *GetItemForBinding(const char *binding);
};