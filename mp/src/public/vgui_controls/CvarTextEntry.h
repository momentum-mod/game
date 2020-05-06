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
        CvarTextEntry(Panel *parent, const char *panelName, char const *cvarname, int precision = 2);

        MESSAGE_FUNC(OnTextChanged, "TextChanged");
        MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");
        void ApplyChanges();
        void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;
        void ApplySettings(KeyValues* inResourceData) OVERRIDE;
        void GetSettings(KeyValues* outResourceData) OVERRIDE;
        void SetText(const char *text) OVERRIDE;
        void InitSettings() OVERRIDE;
        void Reset();
        void OnThink() OVERRIDE;
        void OnKillFocus() OVERRIDE;
        bool HasBeenModified();
        bool HasBeenModifiedExternally() const;
        void SetPrecision(int precision);
        int GetPrecision() const { return m_iPrecision; }

    private:
        ConVarRef m_cvarRef;
        int m_iPrecision;
        char m_pszStartValue[64];
        char m_szNumberFormat[8];
    };
}
