#pragma once

#include "cbase.h"

#include "view.h"
#include "menu.h"
#include "iclientmode.h"
#include "utlvector.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "text_message.h"
#include "hud_macros.h"
#include "weapon_selection.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>


class CHudFillableBar : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudFillableBar, vgui::Panel);

public:

    CHudFillableBar(const char* pElementName) : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), pElementName){}

    void Init(void) 
    {
        SetPaintBackgroundEnabled(false);
        SetValue(m_flInitialValue);
    };
    virtual void Reset(void) {};
    virtual bool ShouldDraw(void) { return CHudElement::ShouldDraw(); }

    void Paint()
    {
        Paint(m_FillColor);
    }

    void Paint(Color BoxColor)
    {
        if (GetCurrentValue() != 0)
        {
            DrawBox(m_flxPos, m_flyPos, m_flWide * (GetCurrentValue() / 100), m_flTall, BoxColor, 1);
        }
        DrawHollowBox(m_flxPos, m_flyPos, m_flWide, m_flTall, m_BackgroundColor, 1, 2, 2);
    }
    virtual void OnThink()
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
    void PaintString(const wchar_t *text, int textlen, vgui::HFont& font, int x, int y);

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
    float GetCurrentValue() { return m_flValue; }

private:
    void(*SelectFunc)(int);
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