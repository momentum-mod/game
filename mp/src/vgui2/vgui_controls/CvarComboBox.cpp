#include "tier1/KeyValues.h"

#include <vgui_controls/CvarComboBox.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

Panel *Create_CvarComboBox()
{
    return new CvarComboBox( nullptr, nullptr );
}

//not sure which build factory to use here (and if the constructor above is necessary)
DECLARE_BUILD_FACTORY_CUSTOM( CvarComboBox, Create_CvarComboBox );

CvarComboBox::CvarComboBox(Panel *parent, const char *panelName, int numLines, bool allowEdit, char const *cvarname, bool ignoreMissingCvar)
    : ComboBox(parent, panelName, numLines, allowEdit), m_cvar((cvarname) ? cvarname : "", (cvarname) ? ignoreMissingCvar : true)
{
    InitSettings();
    m_bIgnoreMissingCvar = ignoreMissingCvar;
    float m_flCvarMin, m_flCvarMax;
    m_iCvarMin = m_cvar.GetMin(m_flCvarMin) ? RoundFloatToInt(m_flCvarMin) : 0;
    m_iCvarMax = m_cvar.GetMin(m_flCvarMax) ? RoundFloatToInt(m_flCvarMax) : 0;
    float m_flCvarMin;
    if (m_cvar.GetMin(m_flCvarMin))
    {
        m_iCvarMin = RoundFloatToInt(m_flCvarMin);
    }
    else
    {
        m_iCvarMin = 0;
    }
    m_iStartValue = 0;

    Reset();
    AddActionSignalTarget(this);
}

CvarComboBox::~CvarComboBox()
{
}

void CvarComboBox::OnThink()
{
    if (m_cvar.IsValid())
    {
        if (m_cvar.GetInt() != m_iStartValue + m_iCvarMin)
            Reset();
    }
}

void CvarComboBox::OnApplyChanges()
{
    ApplyChanges();
}

void CvarComboBox::ApplyChanges()
{
    if (!m_cvar.IsValid())
        return;

    m_iStartValue = GetActiveItem();
    m_cvar.SetValue(m_iStartValue + m_iCvarMin);
}

void CvarComboBox::Reset()
{
    if (!m_cvar.IsValid())
        return;

    m_iStartValue = m_cvar.GetInt();
    ActivateItemByRow(m_iStartValue - m_iCvarMin);
}

bool CvarComboBox::HasBeenModified()
{
    return GetActiveItem() != m_iStartValue - m_iCvarMin;
}

void CvarComboBox::OnTextChanged()
{
    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
    }
    ApplyChanges();
}

void CvarComboBox::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", nullptr);

    if (!cvarName || !Q_stricmp(cvarName, "") || m_cvar.IsValid())
        return; // Doesn't have cvar set up in res file, or we were constructed with one

    m_cvar.Init(cvarName, m_bIgnoreMissingCvar);
    Reset();
}

void CvarComboBox::GetSettings(KeyValues *pOutResource)
{
    BaseClass::GetSettings(pOutResource);

    pOutResource->SetString("cvar_name", m_cvar.GetName());
    pOutResource->SetString("cvar_value", m_cvar.GetString());
}

void CvarComboBox::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING},
    {"cvar_value", TYPE_STRING}
    END_PANEL_SETTINGS();
}