//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef CVARCOMBOBOX_H
#define CVARCOMBOBOX_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "vgui_controls/ComboBox.h"

namespace vgui
{

class CvarComboBox : public ComboBox
{
    DECLARE_CLASS_SIMPLE(CvarComboBox, ComboBox);

    // default values taken from ComboBox_Factory()
    CvarComboBox(Panel *parent, const char *panelName, int numLines = 5, bool allowEdit = true, char const *cvarname = nullptr,
                 bool ignoreMissingCvar = false);
    ~CvarComboBox();

    void Reset();
    void ApplyChanges();
    bool HasBeenModified();

    void Paint() OVERRIDE;
    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues *outResources) OVERRIDE;
    void InitSettings() OVERRIDE;

  private:
    // Called when the OK / Apply button is pressed.  Changed data should be written into cvar.
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
    MESSAGE_FUNC(OnTextChanged, "TextChanged");

    ConVarRef m_cvar;
    int m_iStartValue;
    bool m_bIgnoreMissingCvar;
};

} // namespace vgui

#endif // CVARCOMBOBOX_H
