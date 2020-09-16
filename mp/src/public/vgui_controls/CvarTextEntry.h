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
        void Reset();
        bool HasBeenModified();
        bool HasBeenModifiedExternally() const;
        void SetPrecision(int precision);
        int GetPrecision() const { return m_iPrecision; }

    protected:
        void ApplySchemeSettings(IScheme *pScheme) override;
        void ApplySettings(KeyValues* inResourceData) override;
        void GetSettings(KeyValues* outResourceData) override;
        void SetText(const char *text) override;
        void InitSettings() override;
        void OnThink() override;
        void OnKillFocus() override;

    private:
        bool ShouldUpdate(const char *szText);
        void SetCvarVal(const char *szText);
        int GetPrecisionOfText(const char *szText);

        ConVarRef m_cvarRef;
        int m_iPrecision;
        float m_fClosestToZeroPossible;
        bool m_bCvarMinAboveZero;
        char m_pszStartValue[64];
        char m_szNumberFormat[8];
    };
}
