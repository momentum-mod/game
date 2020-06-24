//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
#include <vgui_controls/CvarSlider.h>
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertyPage.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

Panel *Create_CvarSlider() 
{ 
    return new CvarSlider(nullptr, nullptr);
}

DECLARE_BUILD_FACTORY_CUSTOM(CvarSlider, Create_CvarSlider);

//-----------------------------------------------------------------------------
// Purpose: For setting up the slider in a res file instead of in code
//-----------------------------------------------------------------------------
CvarSlider::CvarSlider(Panel *parent, const char *name) : Slider(parent, name), m_cvar("", true)
{
    InitSettings();
    SetupSlider("", 2, false, false);
    m_bCreatedInCode = false;
    AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: For sliders that use min/max from a cvar
//-----------------------------------------------------------------------------
CvarSlider::CvarSlider(Panel *parent, const char *panelName, char const *cvarname, int precision, bool bAutoApplyChanges)
    : Slider(parent, panelName), m_cvar(cvarname)
{
    InitSettings();
    SetupSlider(cvarname, precision, bAutoApplyChanges);
    m_bCreatedInCode = true;
    AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: For sliders that define their own min/max 
//-----------------------------------------------------------------------------
CvarSlider::CvarSlider(Panel *parent, const char *panelName, char const *cvarname, float minValue, float maxValue,
                       int precision, bool bAutoApplyChanges)
    : Slider(parent, panelName), m_cvar(cvarname)
{
    InitSettings();
    SetupSlider(cvarname, precision, bAutoApplyChanges, false, minValue, maxValue);
    m_bCreatedInCode = true;
    AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::SetupSlider(const char *cvarname, int precision, bool bAutoApplyChanges, bool bUseCvarBounds,
                             float minValue, float maxValue)
{
    Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));
    m_cvar = ConVarRef(m_szCvarName, cvarname[0] == '\0');

    m_bModifiedOnce = false;
    m_bAutoApplyChanges = bAutoApplyChanges;
    m_bUseCvarBounds = bUseCvarBounds;
    SetPrecision(precision);

    if (bUseCvarBounds) // ignore min/max param, just use cvar's min/max
    {
        minValue = m_cvar.GetMin(minValue) ? minValue : 0.0f;
        maxValue = m_cvar.GetMax(maxValue) ? maxValue : 1.0f;
    }
    
    m_flMinValue = ceilf(minValue * m_fScaleFactor) / m_fScaleFactor;
    m_flMaxValue = (maxValue * m_fScaleFactor) / m_fScaleFactor;

    SetMinMaxValues();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::SetPrecision(int precision)
{
    m_iPrecision = precision < 0 ? 0 : precision;
    if (m_iPrecision) // dont worry about setting if 0 as float will be rounded
    {
        Q_snprintf(m_szPrecisionFormat, sizeof(m_szPrecisionFormat), "%%.%if", m_iPrecision);
    }
    m_fScaleFactor = powf(10, m_iPrecision);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    float flMinValue = inResourceData->GetFloat("minvalue", 0.0f);
    float flMaxValue = inResourceData->GetFloat("maxvalue", 1.0f);
    const char *cvarname = inResourceData->GetString("cvar_name");
    int iPrecision = inResourceData->GetInt("precision", 2);
    bool bAutoApplyChanges = inResourceData->GetBool("autoapply", false);
    bool bUseCvarBounds = inResourceData->GetBool("usecvarbounds", false);

    if (!m_bCreatedInCode)
        SetupSlider(cvarname, iPrecision, bAutoApplyChanges, bUseCvarBounds, flMinValue, flMaxValue);

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

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void CvarSlider::GetSettings(KeyValues *outResourceData)
{
    BaseClass::GetSettings(outResourceData);

    outResourceData->SetFloat("minvalue", m_flMinValue);
    outResourceData->SetFloat("maxvalue", m_flMaxValue);
    outResourceData->SetString("cvar_name", m_szCvarName);
    outResourceData->SetInt("precision", m_iPrecision);
    outResourceData->SetBool("autoapply", m_bAutoApplyChanges);
    outResourceData->SetBool("usecvarbounds", m_bUseCvarBounds);
}

void CvarSlider::SetCVarName(char const *cvarname)
{
    Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));
    m_cvar = ConVarRef(m_szCvarName);
    m_bModifiedOnce = false;

    // Set slider to current value
    Reset();
}

void CvarSlider::SetMinMaxValues()
{
    SetRange(static_cast<int>(m_fScaleFactor * m_flMinValue), static_cast<int>(m_fScaleFactor * m_flMaxValue));

    char szMin[32];
    char szMax[32];

    if (m_iPrecision)
    {
        Q_snprintf(szMin, sizeof(szMin), m_szPrecisionFormat, m_flMinValue);
        Q_snprintf(szMax, sizeof(szMax), m_szPrecisionFormat, m_flMaxValue);
    }
    else
    {
        Q_snprintf(szMin, sizeof(szMin), "%i", RoundFloatToInt(m_flMinValue));
        Q_snprintf(szMax, sizeof(szMax), "%i", RoundFloatToInt(m_flMaxValue));
    }

    // overridden if leftText/rightText is defined in res file
    SetTickCaptions(szMin, szMax);

    // Set slider to current value
    Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::Paint()
{
    if (!m_cvar.IsValid())
        return;

    float curvalue = m_cvar.GetFloat();

    // did it get changed from under us?
    if (!CloseEnough(curvalue, m_fStartValue, FLT_EPSILON))
    {
        int val = static_cast<int>(m_fScaleFactor * curvalue);
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
void CvarSlider::ApplyChanges()
{
    if (m_bModifiedOnce)
    {
        m_iStartValue = GetValue();
        m_fStartValue =
            m_bUseCvarBounds ? static_cast<float>(m_iStartValue) / m_fScaleFactor : m_fCurrentValue;

        m_cvar.SetValue(m_iPrecision ? m_fStartValue : RoundFloatToInt(m_fStartValue));
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CvarSlider::GetSliderValue()
{
    if (m_bUseCvarBounds)
        return float(GetValue()) / m_fScaleFactor;

    return m_fCurrentValue / m_fScaleFactor;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::SetSliderValue(float fValue)
{
    int nVal = static_cast<int>(m_fScaleFactor * fValue);
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
void CvarSlider::Reset()
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

    int value = static_cast<int>(m_fScaleFactor * m_fStartValue);
    SetValue(value);

    m_iStartValue = GetValue();
    m_iLastSliderValue = m_iStartValue;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CvarSlider::HasBeenModified()
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
void CvarSlider::OnSliderMoved()
{
    if (m_iLastSliderValue != GetValue())
    {
        m_iLastSliderValue = GetValue();
        m_fCurrentValue = static_cast<float>(m_iLastSliderValue) / m_fScaleFactor;

        if (!m_bUseCvarBounds) // cvar value can be above/below bounds
        {
            // if at min/max value but cvar is lower/higher, allow it
            bool bBelowMin = CloseEnough(m_fCurrentValue, m_flMinValue, FLT_EPSILON) && m_cvar.GetFloat() < m_flMinValue,
                 bAboveMax = CloseEnough(m_fCurrentValue, m_flMaxValue, FLT_EPSILON) && m_cvar.GetFloat() > m_flMaxValue;
            if (bBelowMin || bAboveMax)
                return;
        }

        m_bModifiedOnce = true;

        if (ShouldAutoApplyChanges())
        {
            ApplyChanges();
        }

        // tell parent that we've been modified
        PostActionSignal(new KeyValues("ControlModified"));
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CvarSlider::OnApplyChanges(void)
{
    ApplyChanges();
}

void CvarSlider::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING},
    {"minvalue", TYPE_FLOAT},
    {"maxvalue", TYPE_FLOAT},
    {"precision", TYPE_INTEGER},
    {"autoapply", TYPE_BOOL},
    {"usecvarbounds", TYPE_BOOL}
    END_PANEL_SETTINGS();
}
