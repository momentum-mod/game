#pragma once

#include <vgui_controls/Slider.h>

namespace vgui
{
    class CvarSlider : public vgui::Slider
    {
        DECLARE_CLASS_SIMPLE(CvarSlider, vgui::Slider);

    public:
        CvarSlider(Panel *parent, const char *panelName);
        CvarSlider(Panel *parent, const char *panelName, char const *caption, float minValue, float maxValue,
            char const *cvarname, bool bAllowOutOfRange = false);
        ~CvarSlider();

        void SetupSlider(float minValue, float maxValue, const char *cvarname, bool bAllowOutOfRange);

        void SetCVarName(char const *cvarname);
        void SetMinMaxValues(float minValue, float maxValue, bool bSetTickdisplay = true);
        void SetTickColor(Color color);

        void Paint() OVERRIDE;

        void ApplySettings(KeyValues *inResourceData) OVERRIDE;
        void GetSettings(KeyValues *outResourceData) OVERRIDE;
        void InitSettings() OVERRIDE;

        void ApplyChanges();
        float GetSliderValue();
        void SetSliderValue(float fValue);
        void Reset();
        bool HasBeenModified();

    private:
        MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
        MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");

        bool m_bAllowOutOfRange;
        bool m_bModifiedOnce;
        float m_fStartValue;
        int m_iStartValue;
        int m_iLastSliderValue;
        float m_fCurrentValue;
        char m_szCvarName[64];
        ConVarRef m_cvar;

        bool m_bCreatedInCode;
        float m_flMinValue;
        float m_flMaxValue;
    };
}