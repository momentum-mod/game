#include "cbase.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "menu.h"
#include "time.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class CHudVersionWarn : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudVersionWarn, Panel);

  public:
    CHudVersionWarn(const char *pElementName);

    bool ShouldDraw() override { return CHudElement::ShouldDraw(); }

    void Paint() override;

    void Init() override;

  protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

  private:
    wchar_t uVersionText[BUFSIZELOCL];
};

DECLARE_HUDELEMENT(CHudVersionWarn);

CHudVersionWarn::CHudVersionWarn(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudVersionWarn")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void CHudVersionWarn::Init()
{
    char m_pszStringVersion[BUFSIZELOCL];
    char strVersion[BUFSIZELOCL];
    LOCALIZE_TOKEN(BuildVersion, "#MOM_BuildVersion", strVersion);

    Q_snprintf(m_pszStringVersion, sizeof(m_pszStringVersion), "%s %s",
               strVersion, // BuildVerison localization
               MOM_CURRENT_VERSION);
    g_pVGuiLocalize->ConvertANSIToUnicode(m_pszStringVersion, uVersionText, sizeof(m_pszStringVersion));
}

void CHudVersionWarn::Paint()
{
    surface()->DrawSetTextPos(0, 0);
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(225, 225, 225, 225);
    surface()->DrawPrintText(uVersionText, wcslen(uVersionText));
}
