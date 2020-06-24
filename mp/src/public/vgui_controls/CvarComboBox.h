#pragma once

#include "tier1/convar.h"
#include "vgui_controls/ComboBox.h"

class KeyValues;

namespace vgui
{

class CvarComboBox : public ComboBox
{
    DECLARE_CLASS_SIMPLE(CvarComboBox, ComboBox);

    CvarComboBox(Panel *parent, const char *panelName, char const *cvarname = nullptr);

    void Reset();
    void ApplyChanges();
    bool HasBeenModified();

    void OnKeyCodeTyped(KeyCode code) OVERRIDE;
    void OnThink() OVERRIDE;
    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues *outResources) OVERRIDE;
    void InitSettings() OVERRIDE;

    int AddItem(const char *itemText, const KeyValues *userData) OVERRIDE;

  private:
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
    MESSAGE_FUNC(OnTextChanged, "TextChanged");

    ConVarRef m_cvar;
    int m_iStartValue, m_iCvarMin, m_iCvarMax;
};

// CvarComboBox that has just an "Enable" and "Disable" option for a cvar
class CvarToggleComboBox : public CvarComboBox
{
    DECLARE_CLASS_SIMPLE(CvarToggleComboBox, vgui::CvarComboBox);

    CvarToggleComboBox(Panel *pParent, const char *pName, const char *cvarName);
};

}
