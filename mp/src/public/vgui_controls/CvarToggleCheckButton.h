//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef CVARTOGGLECHECKBUTTON_H
#define CVARTOGGLECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "vgui_controls/CheckButton.h"

namespace vgui
{

class CvarToggleCheckButton : public CheckButton
{
    DECLARE_CLASS_SIMPLE(CvarToggleCheckButton, CheckButton);

    CvarToggleCheckButton(Panel *parent, const char *panelName, const char *text = "", char const *cvarname = nullptr,
                          bool ignoreMissingCvar = false);
    ~CvarToggleCheckButton();

    void Reset();
    void ApplyChanges();
    bool HasBeenModified();

    void SetSelected(bool state) OVERRIDE;
    void Paint() OVERRIDE;
    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues *outResources) OVERRIDE;
    const char *GetDescription() OVERRIDE;

  private:
    // Called when the OK / Apply button is pressed.  Changed data should be written into cvar.
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
    MESSAGE_FUNC(OnButtonChecked, "CheckButtonChecked");

    ConVarRef m_cvar;
    bool m_bStartValue;
    bool m_bIgnoreMissingCvar;
};

} // namespace vgui

#endif // CVARTOGGLECHECKBUTTON_H
