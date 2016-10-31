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
    HSV_Select_Base(vgui::Panel *parent, const char *pElementName);

    void OnMousePressed(MouseCode code);
    void OnMouseReleased(MouseCode code);
    void OnMouseCaptureLost();
    void OnCursorMoved(int x, int y);

  protected:
    bool m_bIsReading;
    int m_iMat;

    virtual void ReadValues();
};

class HSV_Select_SV : public HSV_Select_Base
{
  public:
    DECLARE_CLASS_SIMPLE(HSV_Select_SV, HSV_Select_Base);
    HSV_Select_SV(vgui::Panel *parent, const char *pElementName);

    void Paint();

    void SetSV(float s, float v)
    {
        m_flS = clamp(s, 0, 1);
        m_flV = clamp(v, 0, 1);
    };
    const float GetS() { return m_flS; };
    const float GetV() { return m_flV; };
    void SetHue(float h) { m_flH = clamp(h, 0, 360); };

  private:
    float m_flS;
    float m_flV;
    float m_flH;

    void ReadValues();
};

class HSV_Select_Hue : public HSV_Select_Base
{
  public:
    DECLARE_CLASS_SIMPLE(HSV_Select_Hue, HSV_Select_Base);
    HSV_Select_Hue(vgui::Panel *parent, const char *pElementName);

    void Paint();

    void SetHue(float h) { m_flHue = clamp(h, 0, 360.0f); };
    const float GetHue() { return m_flHue; };

  private:
    float m_flHue;

    void ReadValues();
};

class ColorPicker : public Frame
{
  public:
    DECLARE_CLASS_SIMPLE(ColorPicker, Frame);
    ColorPicker(vgui::Panel *parent, TextEntry *pTargetEntry);
    ColorPicker(vgui::Panel *parent, Panel *pActionsignalTarget);
    ~ColorPicker();

    void Init();

    void OnCommand(const char *cmd);

    void SetPickerColor(const Vector &col);
    Vector GetPickerColor();
    void SetPickerColorHSV(const Vector &col);
    Vector GetPickerColorHSV();

    MESSAGE_FUNC_PARAMS(OnHSVUpdate, "HSVUpdate", pKV); // update from hsv

    MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", pKV); // update from textentry

    virtual void ApplySchemeSettings(IScheme *pScheme);

    void OnThink();

    void Paint();

  private:
    TextEntry *pTarget;

    Vector m_vecColor;
    Vector m_vecHSV;

    HSV_Select_Hue *m_pSelect_Hue;
    HSV_Select_SV *m_pSelect_SV;

    TextEntry *m_pText_RGB[3];
    TextEntry *m_pText_HEX;

    void UpdateAllVars(Panel *pIgnore = nullptr);

    int m_iVgui_Pick_Hue;
    int m_iVgui_Pick_SV;

    PickerHelper *pDrawPicker_Hue;
    PickerHelper *pDrawPicker_SV;
};

} // namespace vgui

#endif
