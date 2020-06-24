#include "tier1/KeyValues.h"

#include <vgui_controls/CvarComboBox.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

Panel *Create_CvarComboBox()
{
    return new CvarComboBox( nullptr, nullptr, nullptr );
}

//not sure which build factory to use here (and if the constructor above is necessary)
DECLARE_BUILD_FACTORY_CUSTOM( CvarComboBox, Create_CvarComboBox );

CvarComboBox::CvarComboBox(Panel *parent, const char *panelName, char const *cvarname)
    : ComboBox(parent, panelName, 5, false), m_cvar(cvarname)
{
    InitSettings();

    float flCvarMin, flCvarMax;
    m_iCvarMin = m_cvar.GetMin(flCvarMin) ? RoundFloatToInt(flCvarMin) : 0;
    m_iCvarMax = m_cvar.GetMax(flCvarMax) ? RoundFloatToInt(flCvarMax) : 0;

    SetNumberOfEditLines( (m_iCvarMax - m_iCvarMin) + 1 );

    m_iStartValue = 0;

    AddActionSignalTarget(this);
}

int CvarComboBox::AddItem(const char* itemText, const KeyValues* userData)
{
    int iRtnVal = BaseClass::AddItem(itemText, userData);
    // reset combobox when it's full
    if (GetItemCount() >= m_iCvarMax - m_iCvarMin) 
    {
        Reset();
    }
    return iRtnVal;
}

void CvarComboBox::OnKeyCodeTyped(KeyCode code)
{
    BaseClass::OnKeyCodeTyped(code);
    // updates cvar to text in combobox when up/down keys are used
    int activeItem = GetActiveItem();
    if (code == KEY_DOWN)
    {
        activeItem++;
        activeItem %= GetItemCount();
    }
    else if (code == KEY_UP)
    {
        activeItem--;
        if (activeItem < 0)
            activeItem = GetItemCount() - 1;
    }
    ActivateItemByRow(activeItem);
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

    m_iStartValue = m_cvar.GetInt() - m_iCvarMin;
    ActivateItemByRow(m_iStartValue);
}

bool CvarComboBox::HasBeenModified()
{
    return GetActiveItem() != m_iStartValue;
}

void CvarComboBox::OnTextChanged()
{
    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
        ApplyChanges();
    }
}

void CvarComboBox::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", nullptr);

    if (!cvarName || !Q_stricmp(cvarName, "") || m_cvar.IsValid())
        return; // Doesn't have cvar set up in res file, or we were constructed with one

    m_cvar.Init(cvarName);
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


CvarToggleComboBox::CvarToggleComboBox(Panel *pParent, const char *pName, const char *pCvar) : BaseClass(pParent, pName, pCvar)
{
    SetEditable(false);
    AddItem("#GameUI_Disabled", nullptr);
    AddItem("#GameUI_Enabled", nullptr);
    SetNumberOfEditLines(2);
}