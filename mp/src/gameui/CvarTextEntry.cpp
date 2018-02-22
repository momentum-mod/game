//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "CvarTextEntry.h"
#include "EngineInterface.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const int MAX_CVAR_TEXT = 64;

CCvarTextEntry::CCvarTextEntry(Panel *parent, const char *panelName, char const *cvarname)
    : TextEntry(parent, panelName), m_cvarRef(cvarname, true)
{
    m_pszStartValue[0] = 0;

    if (m_cvarRef.IsValid())
    {
        Reset();
    }
    
    AddActionSignalTarget(this);
}

CCvarTextEntry::~CCvarTextEntry()
{
}

void CCvarTextEntry::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    if (GetMaximumCharCount() < 0 || GetMaximumCharCount() > MAX_CVAR_TEXT)
    {
        SetMaximumCharCount(MAX_CVAR_TEXT - 1);
    }
}

void CCvarTextEntry::ApplyChanges()
{
    if (!m_cvarRef.IsValid())
        return;

    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    if (!szText[0])
        return;

    m_cvarRef.SetValue(szText);

    Q_strncpy(m_pszStartValue, szText, sizeof(m_pszStartValue));
}

void CCvarTextEntry::Reset()
{
    if (!m_cvarRef.IsValid())
        return;

    const char *value = m_cvarRef.GetString();
    if (value && value[0])
    {
        SetText(value);
        Q_strncpy(m_pszStartValue, value, sizeof(m_pszStartValue));
    }
}

bool CCvarTextEntry::HasBeenModified()
{
    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    return stricmp(szText, m_pszStartValue) != 0 ? true : false;
}

void CCvarTextEntry::OnTextChanged()
{
    if (!m_cvarRef.IsValid())
        return;

    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
    }
}
