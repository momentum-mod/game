#pragma once

#include <vgui_controls/Slider.h>

namespace vgui
{
    class CvarSlider : public vgui::Slider
    {
        DECLARE_CLASS_SIMPLE(CvarSlider, vgui::Slider);

    public:
        CvarSlider(Panel *parent, const char *panelName);
        CvarSlider(Panel *parent, const char *panelName, char const *cvarname, int precision, bool bAutoApplyChanges = false);
        CvarSlider(Panel *parent, const char *panelName, char const *cvarname, float minValue, float maxValue,
                   int precision, bool bAutoApplyChanges = false);

        void SetupSlider(const char *cvarname, int precision, bool bAutoApplyChanges, bool bUseCvarBounds = true,
                         float minValue = 0.0f, float maxValue = 1.0f);

        void SetCVarName(char const *cvarname);
        void SetTickColor(Color color) { m_TickColor = color; }

        void Paint() OVERRIDE;

        void ApplySettings(KeyValues *inResourceData) OVERRIDE;
        void GetSettings(KeyValues *outResourceData) OVERRIDE;
        void InitSettings() OVERRIDE;

        bool ShouldAutoApplyChanges() const { return m_bAutoApplyChanges; }
        void SetAutoApplyChanges(bool val) { m_bAutoApplyChanges = val; }
        void SetPrecision(int precision);
        int GetPrecision() const { return m_iPrecision; }

        void ApplyChanges();
        float GetSliderValue();
        void SetSliderValue(float fValue);
        void Reset();
        bool HasBeenModified();

    private:
        MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
        MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");

        void SetMinMaxValues();

        bool m_bUseCvarBounds, m_bModifiedOnce;
        int m_iStartValue, m_iLastSliderValue;
        float m_fStartValue, m_fCurrentValue, m_fScaleFactor;
        char m_szCvarName[64];

        bool m_bAutoApplyChanges, m_bCreatedInCode;
        float m_flMinValue, m_flMaxValue;
        int m_iPrecision;
        char m_szPrecisionFormat[8];

        ConVarRef m_cvar;
    };
}