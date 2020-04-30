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
            char const *cvarname, bool bAllowOutOfRange = false, bool bAutoApplyChanges = false, bool bOnlyIntegers = false);
        ~CvarSlider();

        void SetupSlider(float minValue, float maxValue, const char *cvarname, bool bAllowOutOfRange,
                         bool bAutoApplyChanges, bool bOnlyIntegers);

        void SetCVarName(char const *cvarname);
        void SetMinMaxValues(float minValue, float maxValue, bool bSetTickdisplay = true);
        void SetTickColor(Color color);

        void Paint() OVERRIDE;

        void ApplySettings(KeyValues *inResourceData) OVERRIDE;
        void GetSettings(KeyValues *outResourceData) OVERRIDE;
        void InitSettings() OVERRIDE;

        bool ShouldAutoApplyChanges() const { return m_bAutoApplyChanges; }
        void SetAutoApplyChanges(bool val) { m_bAutoApplyChanges = val; }
        bool IsOnlyIntegers() const { return m_bOnlyIntegers; }
        void SetOnlyIntegers(bool val) { m_bOnlyIntegers = val; }

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

        bool m_bAutoApplyChanges;
        bool m_bOnlyIntegers;
        bool m_bCreatedInCode;
        float m_flMinValue;
        float m_flMaxValue;
    };
}