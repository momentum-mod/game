#pragma once

#include "tier1/convar.h"
#include "vgui_controls/ComboBox.h"

class KeyValues;

namespace vgui
{

class CvarComboBox : public ComboBox
{
    DECLARE_CLASS_SIMPLE(CvarComboBox, ComboBox);

    CvarComboBox(Panel *parent, const char *panelName, int numLines = 5, bool allowEdit = true, char const *cvarname = nullptr,
                 bool ignoreMissingCvar = false);
    ~CvarComboBox();

    void Reset();
    void ApplyChanges();
    bool HasBeenModified();

    void OnThink() OVERRIDE;
    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues *outResources) OVERRIDE;
    void InitSettings() OVERRIDE;

  private:
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
    MESSAGE_FUNC(OnTextChanged, "TextChanged");

    ConVarRef m_cvar;
    int m_iStartValue;
    bool m_bIgnoreMissingCvar;
};

}
