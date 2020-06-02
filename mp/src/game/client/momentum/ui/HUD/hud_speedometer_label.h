#pragma once

#include <vgui_controls/Label.h>

class SpeedometerLabel : public vgui::Label
{
    DECLARE_CLASS_SIMPLE(SpeedometerLabel, vgui::Label);

  public:
    SpeedometerLabel(Panel *parent, const char *panelName);
    
    void SetVisible(bool bVisible) override;

    void Update(float value); // main function; called when updating the label
    void Reset();
    void SetText(int value);
    void SetText(float value) { SetText(RoundFloatToInt(value)); }

  private:

    float m_flCurrentValue;
    float m_flPastValue;
    float m_flDiff;
};