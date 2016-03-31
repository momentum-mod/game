#include "cbase.h"
#include "hud_fillablebar.h"

using namespace vgui;

CHudFillableBar::CHudFillableBar(const char *pElementName) : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudFillableBar")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
};

DECLARE_HUDELEMENT(CHudFillableBar);

void CHudFillableBar::Init()
{
    m_flDesiredValue = -1;
}

void CHudFillableBar::Paint()
{
    DrawBox(m_flxPos, m_flyPos, m_flWide * (m_flValue / 100), m_flTall, m_FillColor, 1);
    DrawHollowBox(m_flxPos, m_flyPos, m_flWide, m_flTall, m_BackgroundColor, 1, 2, 2);
}

void CHudFillableBar::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    SetPaintBackgroundEnabled(false);
    SetValue(m_flInitialValue);
}


void CHudFillableBar::SetValue(float pPercent, float pInterpTime)
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

void CHudFillableBar::SetValue(float pPercent)
{
    m_flValue = clamp(pPercent, 0, 100);
}


void CHudFillableBar::OnThink()
{
    if (m_flInterpTime > 0 && m_flInterpFromTime > 0)
    {
        if (m_flInterpTime + m_flInterpFromTime < gpGlobals->curtime)
        {
            SetValue(m_flDesiredValue, 0);
        }
        else if (m_flInterpFromTime != gpGlobals->curtime)
        {
            float newvalue = m_flInterpTime / (gpGlobals->curtime - m_flInterpFromTime);
            SetValue(m_flDesiredValue * newvalue);
        }
    }
    else
    {
        //SetValue(RandomFloat(0, 100), 2);
    }
}

CON_COMMAND(barValue, "new value")
{
    CHudFillableBar *Bar = GET_HUDELEMENT(CHudFillableBar);
    if (!Bar)
        return;
    else
    {
        Bar->SetValue((int)args[0], 2);
    }
}