#pragma once

#include "cbase.h"

#include <vgui_controls/Panel.h>

class CHudFillableBar : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudFillableBar, vgui::Panel);

    CHudFillableBar(vgui::Panel *pParent, const char* pElementName) : Panel(pParent, pElementName), m_flxPos(0),
                                                m_flyPos(0), m_flTall(0), m_flWide(0), m_flInitialValue(0),
                                                m_flValue(0), m_flDesiredValue(0), m_flInterpTime(0),
                                                m_flInterpFromTime(0)
    {
        SetPaintBackgroundEnabled(false);
        SetValue(m_flInitialValue);
    }

    void Paint() OVERRIDE
    {
        Paint(m_FillColor);
    }

    void Paint(const Color BoxColor)
    {
        if (!CloseEnough(m_flValue, 0.0f, FLT_EPSILON))
        {
            DrawBox(m_flxPos, m_flyPos, m_flWide * (m_flValue / 100), m_flTall, BoxColor, 1.0f);
        }
        DrawHollowBox(m_flxPos, m_flyPos, m_flWide, m_flTall, m_BackgroundColor, 1, 2, 2);
    }

    void OnThink() OVERRIDE
    {
        if (m_flInterpTime > 0 && m_flInterpFromTime > 0)
        {
            if (m_flInterpTime + m_flInterpFromTime < gpGlobals->curtime)
            {
                SetValue(m_flDesiredValue, 0);
            }
            else if (m_flInterpFromTime != gpGlobals->curtime)
            {
                float newvalue = (gpGlobals->curtime - m_flInterpFromTime) / m_flInterpTime;
                SetValue(m_flDesiredValue * newvalue);
            }
        }
    }

    // Direct setvalue. Use override for interpolation
    // @pPercent: max->100 , min->0
    void SetValue(float pPercent)
    {
        m_flValue = clamp(pPercent, 0, 100);
    }
    // Sets the new % value of the filled box. If pInterpTime > 0, a interpolation is made.
    // @pPercent: max->100 , min->0
    void SetValue(float pPercent, float pInterpTime)
    {
        m_flDesiredValue = clamp(pPercent, 0, 100);
        if (pInterpTime > 0)
        {
            m_flInterpTime = pInterpTime;
            m_flInterpFromTime = gpGlobals->curtime;
        }
        else
        {
            m_flInterpTime = 0;
            m_flInterpFromTime = 0;
            m_flValue = m_flDesiredValue;
            m_flDesiredValue = -1;
        }
    }

    float GetCurrentValue() const { return m_flValue; }

private:
    CPanelAnimationVarAliasType(float, m_flxPos, "xpos", "0.0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flyPos, "ypos", "0.0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flTall, "tall", "0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flWide, "wide", "0", "proportional_float");
    CPanelAnimationVar(float, m_flInitialValue, "InitialValue", "0.0");
    CPanelAnimationVar(Color, m_BackgroundColor, "BackgroundColor", "FgColor");
    CPanelAnimationVar(Color, m_FillColor, "FillColor", "FgColor");

    // Last value is the % of the box that is filled (From left to right)
    float m_flValue;
    // When not -1, it is the value we want the bar to draw
    float m_flDesiredValue;
    // Time it will take to interpolate to the next value. Checks against m_flInterpFromTime. 0 while not used
    float m_flInterpTime;
    // When did we start interpolating?
    float m_flInterpFromTime;
};