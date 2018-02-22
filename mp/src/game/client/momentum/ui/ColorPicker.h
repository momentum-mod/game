#ifndef C_CPICKER_H
#define C_CPICKER_H

#pragma once

#include "vgui_controls/Frame.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/TextEntry.h"

using namespace vgui;

class PickerHelper;

namespace vgui
{

class HSV_Select_Base : public Panel
{
  public:
    DECLARE_CLASS_SIMPLE(HSV_Select_Base, Panel);
    HSV_Select_Base(Panel *parent, const char *pElementName);

    void OnMousePressed(MouseCode code) OVERRIDE;
    void OnMouseReleased(MouseCode code) OVERRIDE;
    void OnMouseCaptureLost() OVERRIDE;
    void OnCursorMoved(int x, int y) OVERRIDE;

  protected:
    bool m_bIsReading;
    int m_iMat;

    virtual void ReadValues();
};

class HSV_Select_SV : public HSV_Select_Base
{
  public:
    DECLARE_CLASS_SIMPLE(HSV_Select_SV, HSV_Select_Base);
    HSV_Select_SV(Panel *parent, const char *pElementName);

    void Paint() OVERRIDE;

    void SetSV(float s, float v)
    {
        m_flS = clamp(s, 0, 1);
        m_flV = clamp(v, 0, 1);
    };
    float GetS() const { return m_flS; };
    float GetV() const { return m_flV; };
    void SetHue(float h) { m_flH = clamp(h, 0, 360); };

  private:
    float m_flS;
    float m_flV;
    float m_flH;

    void ReadValues() OVERRIDE;
};

class HSV_Select_Hue : public HSV_Select_Base
{
  public:
    DECLARE_CLASS_SIMPLE(HSV_Select_Hue, HSV_Select_Base);
    HSV_Select_Hue(Panel *parent, const char *pElementName);

    void Paint() OVERRIDE;

    void SetHue(float h) { m_flHue = clamp(h, 0, 360.0f); };
    float GetHue() const { return m_flHue; };

  private:
    float m_flHue;

    void ReadValues() OVERRIDE;
};

class ColorPicker : public Frame
{
  public:
    DECLARE_CLASS_SIMPLE(ColorPicker, Frame);
    ColorPicker(Panel *parent, TextEntry *pTargetEntry);
    ColorPicker(Panel *parent, Panel *pActionsignalTarget);
    ~ColorPicker();

    void Init();
    void Show();

    void OnCommand(const char *cmd) OVERRIDE;

    void SetPickerColor(const Color &col);
    void SetPickerColor(const Vector4D &col);
    Vector4D GetPickerColor() const;
    void SetPickerColorHSV(const Vector &col);
    Vector GetPickerColorHSV() const;

    MESSAGE_FUNC_PTR(OnHSVUpdate, "HSVUpdate", panel); // update from hsv

    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel); // update from textentry

    MESSAGE_FUNC_PTR(OnSliderMoved, "SliderMoved", panel); // update from alpha slider

    virtual void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;

    void UpdateAlpha(bool bWasSlider);

    void OnThink() OVERRIDE;

    void Paint() OVERRIDE;

    void SetTargetCallback(Panel *pPanel) { pTargetCallback = pPanel; }

  private:
    TextEntry *pTarget;
    Panel *pTargetCallback; // Target used in the ColorPicked command

    Vector4D m_vecColor;
    Vector m_vecHSV;

    HSV_Select_Hue *m_pSelect_Hue;
    HSV_Select_SV *m_pSelect_SV;

    TextEntry *m_pText_RGBA[4];
    TextEntry *m_pText_HEX;

    Panel *m_pColorPreview;
    Slider *m_pAlphaSlider;

    void UpdateAllVars(Panel *pIgnore = nullptr);

    int m_iVgui_Pick_Hue;
    int m_iVgui_Pick_SV;

    PickerHelper *pDrawPicker_Hue;
    PickerHelper *pDrawPicker_SV;
};

} // namespace vgui

#endif