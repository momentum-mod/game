#pragma once

#include <vgui_controls/Label.h>
#include "mom_shareddefs.h"

class SpeedometerLabel : public vgui::Label
{
    DECLARE_CLASS_SIMPLE(SpeedometerLabel, vgui::Label);

  public:
    SpeedometerLabel(Panel *parent, const char *panelName);

    void OnThink() override; // for applying fadeout
    
    void SetVisible(bool bVisible) override;

    void Update(float value); // main function; called when updating the label
    void Reset();
    void SetText(int value);
    void SetText(float value) { SetText(RoundFloatToInt(value)); }

    // fadeout related functions
    void SetFadeOutAnimation(char *animationName, float *animationAlpha);
    bool HasFadeOutAnimation() { return m_pszAnimationName[0] != '\0' && m_pflAlpha; }

    // getter/setters
    SpeedometerUnits_t GetUnitType() { return m_eUnitType; }
    void SetUnitType(SpeedometerUnits_t type) { m_eUnitType = type; }
    void SetUnitType(int type);

    bool GetSupportsEnergyUnits() { return m_bSupportsEnergyUnits; }
    void SetSupportsEnergyUnits(bool bSupportsEnergyUnits) { m_bSupportsEnergyUnits = bSupportsEnergyUnits; }

  private:
    void ConvertUnits();
    bool StartFadeout();

    float m_flCurrentValue;
    float m_flPastValue;
    float m_flDiff;

    bool m_bSupportsEnergyUnits;

    // fadeout animation fields
    float *m_pflAlpha;
    char m_pszAnimationName[BUFSIZELOCL];

    SpeedometerUnits_t m_eUnitType;
};