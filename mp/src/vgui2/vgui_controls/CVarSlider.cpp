//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
#include <vgui_controls/CVarSlider.h>
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertyPage.h>

#define CVARSLIDER_SCALE_FACTOR 100.0f

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY(CCvarSlider);

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CCvarSlider::CCvarSlider(Panel *parent, const char *name) : Slider(parent, name), m_cvar("", true)
{
    SetupSlider(0.0f, 1.0f, "", false);
    m_bCreatedInCode = false;

    AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CCvarSlider::CCvarSlider(Panel *parent, const char *panelName, char const *caption, float minValue, float maxValue,
                         char const *cvarname, bool bAllowOutOfRange)
    : Slider(parent, panelName), m_cvar(cvarname)
{
    AddActionSignalTarget(this);

    SetupSlider(minValue, maxValue, cvarname, bAllowOutOfRange);

    // For backwards compatability. Ignore .res file settings for forced setup sliders.
    m_bCreatedInCode = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::SetupSlider(float minValue, float maxValue, const char *cvarname, bool bAllowOutOfRange)
{
    m_flMinValue = minValue;
    m_flMaxValue = maxValue;

    // scale by CVARSLIDER_SCALE_FACTOR
    SetRange(static_cast<int>(CVARSLIDER_SCALE_FACTOR * minValue), static_cast<int>(CVARSLIDER_SCALE_FACTOR * maxValue));

    char szMin[32];
    char szMax[32];

    Q_snprintf(szMin, sizeof(szMin), "%.2f", minValue);
    Q_snprintf(szMax, sizeof(szMax), "%.2f", maxValue);

    SetTickCaptions(szMin, szMax);

    Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));
    m_cvar = ConVarRef(m_szCvarName);

    m_bModifiedOnce = false;
    m_bAllowOutOfRange = bAllowOutOfRange;

    // Set slider to current value
    Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CCvarSlider::~CCvarSlider() {}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    if (!m_bCreatedInCode)
    {
        float minValue = inResourceData->GetFloat("minvalue", 0.0f);
        float maxValue = inResourceData->GetFloat("maxvalue", 1.0f);
        const char *cvarname = inResourceData->GetString("cvar_name", "");
        bool bAllowOutOfRange = inResourceData->GetBool("allowoutofrange");
        SetupSlider(minValue, maxValue, cvarname, bAllowOutOfRange);

        if (GetParent())
        {
            // HACK: If our parent is a property page, we want the dialog containing it
            if (dynamic_cast<PropertyPage *>(GetParent()) && GetParent()->GetParent())
            {
                GetParent()->GetParent()->AddActionSignalTarget(this);
            }
            else
            {
                GetParent()->AddActionSignalTarget(this);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void CCvarSlider::GetSettings(KeyValues *outResourceData)
{
    BaseClass::GetSettings(outResourceData);

    if (!m_bCreatedInCode)
    {
        outResourceData->SetFloat("minvalue", m_flMinValue);
        outResourceData->SetFloat("maxvalue", m_flMaxValue);
        outResourceData->SetString("cvar_name", m_szCvarName);
        outResourceData->SetBool("allowoutofrange", m_bAllowOutOfRange);
    }
}

void CCvarSlider::SetCVarName(char const *cvarname)
{
    Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));
    m_cvar = ConVarRef(m_szCvarName);
    m_bModifiedOnce = false;

    // Set slider to current value
    Reset();
}

void CCvarSlider::SetMinMaxValues(float minValue, float maxValue, bool bSetTickDisplay)
{
    SetRange(int(CVARSLIDER_SCALE_FACTOR * minValue), int(CVARSLIDER_SCALE_FACTOR * maxValue));

    if (bSetTickDisplay)
    {
        char szMin[32];
        char szMax[32];

        Q_snprintf(szMin, sizeof(szMin), "%.2f", minValue);
        Q_snprintf(szMax, sizeof(szMax), "%.2f", maxValue);

        SetTickCaptions(szMin, szMax);
    }

    // Set slider to current value
    Reset();
}

void CCvarSlider::SetTickColor(Color color) { m_TickColor = color; }

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::Paint()
{
    if (!m_cvar.IsValid())
        return;

    float curvalue = m_cvar.GetFloat();

    // did it get changed from under us?
    if (!CloseEnough(curvalue, m_fStartValue, FLT_EPSILON))
    {
        int val = static_cast<int>(CVARSLIDER_SCALE_FACTOR * curvalue);
        m_fStartValue = curvalue;
        m_fCurrentValue = curvalue;

        SetValue(val);
        m_iStartValue = GetValue();
    }

    BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::ApplyChanges()
{
    if (m_bModifiedOnce)
    {
        m_iStartValue = GetValue();
        if (m_bAllowOutOfRange)
        {
            m_fStartValue = m_fCurrentValue;
        }
        else
        {
            m_fStartValue = static_cast<float>(m_iStartValue) / CVARSLIDER_SCALE_FACTOR;
        }

        m_cvar.SetValue(m_fStartValue);
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CCvarSlider::GetSliderValue()
{
    if (m_bAllowOutOfRange)
    {
        return m_fCurrentValue;
    }

    return float(GetValue()) / CVARSLIDER_SCALE_FACTOR;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::SetSliderValue(float fValue)
{
    int nVal = static_cast<int>(CVARSLIDER_SCALE_FACTOR * fValue);
    SetValue(nVal, false);

    // remember this slider value
    m_iLastSliderValue = GetValue();

    if (!CloseEnough(m_fCurrentValue, fValue, FLT_EPSILON))
    {
        m_fCurrentValue = fValue;
        m_bModifiedOnce = true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set slider to current value
//-----------------------------------------------------------------------------
void CCvarSlider::Reset()
{
    if (!m_cvar.IsValid())
    {
        m_fCurrentValue = m_fStartValue = 0.0f;
        SetValue(0);
        m_iStartValue = GetValue();
        m_iLastSliderValue = m_iStartValue;
        return;
    }

    m_fStartValue = m_cvar.GetFloat();
    m_fCurrentValue = m_fStartValue;

    int value = static_cast<int>(CVARSLIDER_SCALE_FACTOR * m_fStartValue);
    SetValue(value);

    m_iStartValue = GetValue();
    m_iLastSliderValue = m_iStartValue;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CCvarSlider::HasBeenModified()
{
    if (GetValue() != m_iStartValue)
    {
        m_bModifiedOnce = true;
    }

    return m_bModifiedOnce;
}

//-----------------------------------------------------------------------------
// Purpose: This is called when the slider was moved.
//-----------------------------------------------------------------------------
void CCvarSlider::OnSliderMoved()
{
    if (m_iLastSliderValue != GetValue())
    {
        m_iLastSliderValue = GetValue();
        m_fCurrentValue = static_cast<float>(m_iLastSliderValue) / CVARSLIDER_SCALE_FACTOR;
        m_bModifiedOnce = true;
        // tell parent that we've been modified
        PostActionSignal(new KeyValues("ControlModified"));
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvarSlider::OnApplyChanges(void)
{
    if (!m_bCreatedInCode)
    {
        ApplyChanges();
    }
}

const char* CCvarSlider::GetDescription()
{
    static char buf[1024];
    Q_snprintf(buf, 1024, "%s, string cvar_name, string minvalue, string maxvalue, int allowoutofrange", BaseClass::GetDescription());
    return buf;
}