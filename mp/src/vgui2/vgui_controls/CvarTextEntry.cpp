//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/CvarTextEntry.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const int MAX_CVAR_TEXT = 64;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(CvarTextEntry, "");

CvarTextEntry::CvarTextEntry(Panel *parent, const char *panelName, char const *cvarname, int precision)
    : TextEntry(parent, panelName), m_cvarRef(cvarname, true)
{
    InitSettings();
    SetPrecision(precision);
    m_pszStartValue[0] = 0;

    if (m_cvarRef.IsValid())
    {
        float min;
        m_bCvarMinAboveZero = m_cvarRef.GetMin(min) ? min >= 0.0f : true;
        Reset();
    }
    
    AddActionSignalTarget(this);
}

void CvarTextEntry::SetPrecision(int precision)
{
    m_iPrecision = precision < 0 ? 0 : precision;
    if (m_iPrecision) // dont worry about setting if 0 as float will be rounded
    {
        Q_snprintf(m_szNumberFormat, sizeof(m_szNumberFormat), "%%.%if", m_iPrecision);
    }
    m_fClosestToZeroPossible = 1 / powf(10, m_iPrecision);
}

void CvarTextEntry::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    if (GetMaximumCharCount() < 0 || GetMaximumCharCount() > MAX_CVAR_TEXT)
    {
        SetMaximumCharCount(MAX_CVAR_TEXT - 1);
    }
}

void CvarTextEntry::ApplySettings(KeyValues* inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", "");

    m_cvarRef.Init(cvarName);

    if (m_cvarRef.IsValid())
    {
        Reset();
    }
}

void CvarTextEntry::GetSettings(KeyValues* pOutResource)
{
    BaseClass::GetSettings(pOutResource);

    pOutResource->SetString("cvar_name", m_cvarRef.GetName());
}

void CvarTextEntry::SetText(const char* text)
{
    if (!text || !text[0]) 
        return;

    if (GetAllowNumericInputOnly())
    {
        char newText[MAX_CVAR_TEXT];
        if (m_iPrecision)
        {
            Q_snprintf(newText, MAX_CVAR_TEXT, m_szNumberFormat, atof(text));
        }
        else
        {
            Q_snprintf(newText, MAX_CVAR_TEXT, "%i", atoi(text));
        }
        BaseClass::SetText(newText);
    }
    else
    {
        BaseClass::SetText(text);
    }
}

void CvarTextEntry::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING}
    END_PANEL_SETTINGS();
}

void CvarTextEntry::OnApplyChanges()
{
    ApplyChanges();
}

void CvarTextEntry::ApplyChanges()
{
    if (!m_cvarRef.IsValid())
        return;

    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    if (!szText[0])
        return;

    SetCvarVal(szText);
}

void CvarTextEntry::Reset()
{
    if (!m_cvarRef.IsValid())
        return;

    const char *value = m_cvarRef.GetString();
    if (value && value[0])
    {
        SetText(value);
        Q_strncpy(m_pszStartValue, value, sizeof(m_pszStartValue));
        GotoTextEnd();
    }
}

void CvarTextEntry::OnThink()
{
    if (HasBeenModifiedExternally())
        Reset();
}

void CvarTextEntry::OnKillFocus()
{
    if (!m_cvarRef.IsValid())
        return;

    char entryValue[MAX_CVAR_TEXT];
    GetText(entryValue, MAX_CVAR_TEXT);
    if (!entryValue[0] || stricmp(m_cvarRef.GetString(), m_pszStartValue) != 0)
    {
        Reset();
    }
}

bool CvarTextEntry::HasBeenModified()
{
    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    return stricmp(szText, m_pszStartValue) != 0;
}

bool CvarTextEntry::HasBeenModifiedExternally() const
{
    return m_cvarRef.IsValid() && stricmp(m_cvarRef.GetString(), m_pszStartValue) != 0;
}

bool CvarTextEntry::ShouldUpdate(const char *szText)
{
    if (m_iPrecision == 0) // always update if just integers
        return true;

    int strLength = Q_strlen(szText);

    if (strLength == 1 && szText[0] == '-')
        return false;

    bool isJustZero = szText[strLength - 1] == '0' && 
        (strLength == 1 || (strLength == 2 && szText[0] == '-')); // 0 or -0
    if (isJustZero)
        return true;

    if (szText[strLength - 1] == '.') // dont reset if end is a dot (inputing decimal)
        return false;

    bool isEqToZero = CloseEnough(Q_atof(szText), 0.0f, FLT_EPSILON), // current text value is 0, 0.0, 0.00, etc
         lastNumIsZero = szText[strLength - 1] == '0'; // last number is zero -> hasn't finished inputting
    if (lastNumIsZero && isEqToZero)
        return false;

    return true;
}

void CvarTextEntry::SetCvarVal(const char* szText)
{
    if (!GetAllowNumericInputOnly())
    {
        m_cvarRef.SetValue(szText);
        return;
    }

    char szCorrectedText[MAX_CVAR_TEXT];
    Q_strncpy(szCorrectedText, szText, MAX_CVAR_TEXT);
    if (m_bCvarMinAboveZero && szCorrectedText[0] == '-') // prevent negative if cvar's min is >= 0
    {
        Q_StrSubst(szCorrectedText, "-", "", szCorrectedText, MAX_CVAR_TEXT);
        Q_strncpy(m_pszStartValue, szCorrectedText, sizeof(m_pszStartValue));
    }

    if (!ShouldUpdate(szCorrectedText))
        return;

    if (m_iPrecision == 0) // integer
    {
        m_cvarRef.SetValue(Q_atoi(szCorrectedText));
        Q_strncpy(m_pszStartValue, szCorrectedText, sizeof(m_pszStartValue));
        return;
    }

    // now just floating point numbers which need precision checks
    float fVal = Q_atof(szCorrectedText);
    float fAbsVal = fabsf(fVal);
    // if value is lower than the smallest (in magnitude) allowed value, change it to that & reset
    if (fAbsVal < 1.0f && fAbsVal < m_fClosestToZeroPossible && !CloseEnough(fVal, 0.0f, FLT_EPSILON))
    {
        m_cvarRef.SetValue(fVal > 0.0f ? m_fClosestToZeroPossible : m_fClosestToZeroPossible * -1.0f);
        Reset();
    }
    else
    {
        if (GetPrecisionOfText(szCorrectedText) > m_iPrecision) // too precise; change & reset
        {
            Q_snprintf(szCorrectedText, MAX_CVAR_TEXT, m_szNumberFormat, fVal);
            m_cvarRef.SetValue(szCorrectedText);
            Reset();
        }
        else // good to set
        {
            m_cvarRef.SetValue(szCorrectedText);
            Q_strncpy(m_pszStartValue, szCorrectedText, sizeof(m_pszStartValue));
        }
    }
}

int CvarTextEntry::GetPrecisionOfText(const char *szText)
{
    int precision = 0;
    for (int i = Q_strlen(szText) - 1; i >= 0; i--)
    {
        if (szText[i] != '.')
        {
            precision++;
            if (i == 0) // none found
            {
                precision = 0;
            }
        }
        else
        {
            break;
        }
    }
    return precision;
}

void CvarTextEntry::OnTextChanged()
{
    if (!m_cvarRef.IsValid())
        return;

    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
        ApplyChanges();
    }
}
