//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once

#include <vgui_controls/TextEntry.h>

class CCvarTextEntry : public vgui::TextEntry
{
    DECLARE_CLASS_SIMPLE(CCvarTextEntry, vgui::TextEntry);

  public:
    CCvarTextEntry(vgui::Panel *parent, const char *panelName, char const *cvarname);
    ~CCvarTextEntry();

    MESSAGE_FUNC(OnTextChanged, "TextChanged");
    void ApplyChanges();
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void Reset();
    bool HasBeenModified();

  private:
    ConVarRef m_cvarRef;
    char m_pszStartValue[64];
};