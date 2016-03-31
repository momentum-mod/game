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
    CHudFillableBar(const char*);

    void Init(void);
    void Reset(void) {};
    bool ShouldDraw(void) { return CHudElement::ShouldDraw(); }
    void Paint();
    void OnThink();

    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    void PaintString(const wchar_t *text, int textlen, vgui::HFont& font, int x, int y);

    // Sets the new % value of the filled box. If pInterpTime > 0, a interpolation is made.
    // @pPercent: max->100 , min->0
    void SetValue(float pPercent, float pInterpTime);
    // Direct setvalue. Use override for interpolation
    // @pPercent: max->100 , min->0
    void SetValue(float pPercent);
    float GetCurrentValue() { return m_flValue; }
    void InterpolateValues(float pNewPercent);

private:
    void(*SelectFunc)(int);
    CPanelAnimationVar(float, m_flxPos, "xpos", "0.0");
    CPanelAnimationVar(float, m_flyPos, "ypos", "0.0");
    CPanelAnimationVar(float, m_flTall, "tall", "200.0");
    CPanelAnimationVar(float, m_flWide, "wide", "20.0");
    CPanelAnimationVar(float, m_flInitialValue, "InitialValue", "50.0");
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