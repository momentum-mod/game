#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include "mom_shareddefs.h"
#include <vgui_controls/Label.h>

#include "icommandline.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_versioninfo_enable, "1", FLAG_HUD_CVAR, "Toggles showing the current momentum version string. 0 = OFF, 1 = ON.\n");

class CHudVersionInfo : public CHudElement, public Label
{
    DECLARE_CLASS_SIMPLE(CHudVersionInfo, Label);

    CHudVersionInfo(const char *pElementName);
    bool ShouldDraw() override;

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

bool CHudVersionInfo::ShouldDraw()
{
    return CHudElement::ShouldDraw() && mom_hud_versioninfo_enable.GetBool();
}

void CHudVersionInfo::VidInit()
{
    KeyValuesAD loc("Version");
    loc->SetWString("verLabel", g_pVGuiLocalize->FindSafe("#MOM_StartupMsg_Alpha_Title"));
    loc->SetString("verNum", MOM_CURRENT_VERSION);
    loc->SetString("mappingMode", CommandLine()->CheckParm("-mapping") ? "- Mapping Mode Active" : "");
    SetText(CConstructLocalizedString(L"%verLabel% %verNum% %mappingMode%", (KeyValues*)loc));
    InvalidateLayout();
}

void CHudVersionInfo::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetFont(pScheme->GetFont(m_szTextFont, true));
    InvalidateLayout(true);
}