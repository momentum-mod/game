#pragma once

#include "controls/DoubleLabel.h"
#include "mom_shareddefs.h"

// for ranged based coloring
struct Range_t
{
    // min/max are inclusive
    int min;
    int max;
    Color color;
};
typedef CUtlVector<Range_t> RangeList;

class SpeedometerLabel : public vgui::DoubleLabel
{
    DECLARE_CLASS_SIMPLE(SpeedometerLabel, vgui::DoubleLabel);

  public:
    SpeedometerLabel(Panel *parent, const char *panelName, SpeedometerColorize_t colorizeType);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnThink() override; // for applying fadeout
    
    void SetVisible(bool bVisible) override;

    void Update(float value); // main function; called when updating the label
    void Reset();
    void SetText(int value);
    void SetText(float value) { SetText(RoundFloatToInt(value)); }

    // for separate comparison label
    void SetCustomDiff(float diff);
    // colorize will function as no color option if false
    void SetDrawComparison(bool bEnabled) { m_bDrawComparison = bEnabled; }

    // fadeout related functions
    void SetFadeOutAnimation(const char *animationName, float *animationAlpha);
    bool HasFadeOutAnimation() { return m_pszAnimationName[0] != '\0' && m_pflAlpha; }

    // getter/setters
    SpeedometerUnits_t GetUnitType() { return m_eUnitType; }
    void SetUnitType(SpeedometerUnits_t type) { m_eUnitType = type; }
    void SetUnitType(int type);

    SpeedometerColorize_t GetColorizeType() { return m_eColorizeType; }
    void SetColorizeType(SpeedometerColorize_t type) { m_eColorizeType = type; Reset(); }
    void SetColorizeType(int type);

    bool GetSupportsEnergyUnits() { return m_bSupportsEnergyUnits; }
    void SetSupportsEnergyUnits(bool bSupportsEnergyUnits) { m_bSupportsEnergyUnits = bSupportsEnergyUnits; }

    bool GetSupportsSeparateComparison() { return m_bSupportsSeparateComparison; }
    void SetSupportsSeparateComparison(bool bSupportsSeparateComparison) { m_bSupportsSeparateComparison = bSupportsSeparateComparison; }
    
    void ApplyKV(KeyValues *pIn);

  private:
    void ConvertUnits();
    void Colorize();
    bool StartFadeout();

    void ColorizeRange();
    void ColorizeComparison();
    void ColorizeComparisonSeparate();

    float m_flCurrentValue;
    float m_flPastValue;
    float m_flDiff;
    float m_flNextUpdateCheck;

    bool m_bDrawComparison;
    bool m_bSupportsEnergyUnits;
    bool m_bSupportsSeparateComparison;
    bool m_bCustomDiff;

    // fadeout animation fields
    float *m_pflAlpha;
    char m_pszAnimationName[BUFSIZELOCL];
    bool m_bDoneFading;

    Color m_NormalColor, m_IncreaseColor, m_DecreaseColor;

    RangeList m_vecRangeList;

    SpeedometerUnits_t m_eUnitType;

    SpeedometerColorize_t m_eColorizeType;
};