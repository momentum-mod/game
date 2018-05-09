//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once

#include <vgui_controls/TextEntry.h>

namespace vgui
{
    class CvarTextEntry : public TextEntry
    {
        DECLARE_CLASS_SIMPLE(CvarTextEntry, vgui::TextEntry);

    public:
        CvarTextEntry(Panel *parent, const char *panelName, char const *cvarname);

        MESSAGE_FUNC(OnTextChanged, "TextChanged");
        MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
        void ApplyChanges();
        void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;
        void ApplySettings(KeyValues* inResourceData) OVERRIDE;
        void GetSettings(KeyValues* outResourceData) OVERRIDE;
        void InitSettings() OVERRIDE;
        void Reset();
        bool HasBeenModified();

    private:
        ConVarRef m_cvarRef;
        char m_pszStartValue[64];
    };
}
