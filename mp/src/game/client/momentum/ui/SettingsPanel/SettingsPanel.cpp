#include "cbase.h"

#include "SettingsPanel.h"

#include "vgui_controls/Button.h"

#include "tier0/memdbgon.h"

using namespace vgui;

SettingsPanel::SettingsPanel(Panel *pParent, const char *pName, Button *pAssociate) : BaseClass(pParent, pName), m_pAssociatedButton(pAssociate)
{
    SetVisible(false);
}

void SettingsPanel::OnPageShow()
{
    m_pAssociatedButton->SetSelected(true);
}

void SettingsPanel::OnPageHide()
{
    m_pAssociatedButton->SetSelected(false);
}