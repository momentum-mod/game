//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"

#include "CVarTextEntry.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include <vgui_controls/Panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY(CCvarTextEntry);

CCvarTextEntry::CCvarTextEntry(Panel* parent, const char* panelName) : TextEntry(parent, panelName)
{
    m_bMinValue = false;
    m_bMaxValue = false;
    m_flMinValue = 0.0f;
    m_flMaxValue = 0.0f;
    m_bTextSetup = false;
    AddActionSignalTarget(this);
}

CCvarTextEntry::~CCvarTextEntry()
{
    // MOM_TODO: Ensure a good destructor
}

void CCvarTextEntry::SetupText(const char* cvarname, bool pNumericOnly, bool bMinValue, float minValue, bool bMaxValue, float maxValue)
{
    // We need a valid cvar for us to work properly
    if (!cvarname || Q_strlen(cvarname) == 0)
        Assert(false);
    Q_strcpy(m_szCvarName, cvarname);
    ConVarRef cvarref(cvarname);
    if (!cvarref.IsValid())
        Assert(false);  // You are here because the target cvar is not valid. Not big deal tho. It just won't work
    else
        SetText(cvarref.GetString());

    SetAllowNumericInputOnly(pNumericOnly);

    m_bMinValue = bMinValue;
    if (m_bMinValue != cvarref.IsFlagSet(EFL_HAS_PLAYER_CHILD))
        m_flMinValue = minValue;

    m_bMaxValue = bMaxValue;
    m_flMaxValue = maxValue;

    m_bTextSetup = true;
}

void CCvarTextEntry::ApplySettings(KeyValues* inResourceData)
{
    BaseClass::ApplySettings(inResourceData);
    const char* pCvarname = inResourceData->GetString("cvar_name", nullptr);
    bool pNumericOnly = inResourceData->GetInt("NumericInputOnly", 0);
    // With a value of 2, we let the cvar itself decide our propietes for max/min (or both)
    int pHasMin = inResourceData->GetInt("hasminvalue", 2);
    int pHasMax = inResourceData->GetInt("hasmaxvalue", 2);
    float pMin = inResourceData->GetFloat("minvalue", 0);
    float pMax = inResourceData->GetFloat("maxvalue", 0);
    if (pCvarname && (pHasMin == 2 || pHasMax == 2) && Q_strlen(pCvarname) > 0)
    {
        // Using this method so we get access to GetMin() and GetMax()
        // previous verison added those methods to ConVarRef wich is not wanted atm
        ConVar* pTargetCVar = cvar->FindVar(pCvarname);
        if (pTargetCVar)
        {
            pHasMin = pHasMin == 2 ? pTargetCVar->GetMin(pMin) : pHasMin;
            pHasMax = pHasMax == 2 ? pTargetCVar->GetMax(pMax) : pHasMax;
        }
    }
    SetupText(pCvarname, pNumericOnly, pHasMin, pMin, pHasMax, pMax);
}

void CCvarTextEntry::GetSettings(KeyValues* outResourceData)
{
    BaseClass::GetSettings(outResourceData);
    outResourceData->SetString("cvar_name", m_szCvarName);
    outResourceData->SetBool("hasminvalue", m_bMinValue);
    outResourceData->SetFloat("minvalue", m_flMinValue);
    outResourceData->SetBool("hasmaxvalue", m_bMaxValue);
    outResourceData->SetFloat("maxvalue", m_flMaxValue);
}

void CCvarTextEntry::ApplyChanges()
{
    char buf[BUFSIZ];
    GetText(buf, BUFSIZ);
    ConVarRef cvarref(m_szCvarName);
    if (cvarref.IsValid())
    {
        cvarref.SetValue(buf);
    }
}

void CCvarTextEntry::OnApplyChanges()
{
    ApplyChanges();
}