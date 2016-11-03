#pragma once

#include <vgui_controls/TextEntry.h>

class CCvarTextEntry : public vgui::TextEntry
{
    DECLARE_CLASS_SIMPLE(CCvarTextEntry, vgui::TextEntry);

    CCvarTextEntry(Panel *parent, const char *panelName);
    ~CCvarTextEntry();

    void SetupText(const char *cvarname, bool pNumericOnly = false, bool bMinValue = false, float minValue = 0.0f, bool bMaxValue = false, float maxValue = 0.0f);

    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues *outResourceData) OVERRIDE;

    void ApplyChanges();

private:
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");

    char m_szCvarName[64];

    bool m_bMinValue;
    float m_flMinValue;
    bool m_bMaxValue;
    float m_flMaxValue;

    bool m_bTextSetup;
};
