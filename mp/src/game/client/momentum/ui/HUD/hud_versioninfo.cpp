#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include "mom_shareddefs.h"
#include <vgui_controls/Label.h>

#include "tier0/memdbgon.h"

using namespace vgui;

class CHudVersionInfo : public CHudElement, public Label
{
    DECLARE_CLASS_SIMPLE(CHudVersionInfo, Label);

    CHudVersionInfo(const char *pElementName);

protected:
    void VidInit() OVERRIDE;
    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;
    CPanelAnimationStringVar(32, m_szTextFont, "TextFont", "MomHudDropText");
};

DECLARE_HUDELEMENT(CHudVersionInfo);

CHudVersionInfo::CHudVersionInfo(const char *pElementName) : CHudElement(pElementName),
    Label(g_pClientMode->GetViewport(), "CHudVersionInfo", "")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetAutoWide(true);
    SetAutoTall(true);
}

void CHudVersionInfo::VidInit()
{
    KeyValuesAD loc("Version");
    loc->SetWString("verLabel", g_pVGuiLocalize->Find("#MOM_StartupMsg_Prealpha_Title"));
    loc->SetString("verNum", MOM_CURRENT_VERSION);
    SetText(CConstructLocalizedString(L"%verLabel% %verNum%", (KeyValues*)loc));
    InvalidateLayout();
}

void CHudVersionInfo::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetFont(pScheme->GetFont(m_szTextFont, true));
    InvalidateLayout(true);
}