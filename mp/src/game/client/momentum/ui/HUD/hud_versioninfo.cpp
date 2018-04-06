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

  public:
    CHudVersionInfo(const char *pElementName);

    void VidInit() OVERRIDE;

  protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

  private:
    wchar_t uVersionText[BUFSIZELOCL];
};

DECLARE_HUDELEMENT(CHudVersionInfo);

CHudVersionInfo::CHudVersionInfo(const char *pElementName)
    : CHudElement(pElementName), Label(g_pClientMode->GetViewport(), "CHudVersionInfo", "")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void CHudVersionInfo::VidInit()
{
    char m_pszStringVersion[BUFSIZELOCL];
    char strVersion[BUFSIZELOCL];
    LOCALIZE_TOKEN(BuildVersion, "#MOM_StartupMsg_Prealpha_Title", strVersion);

    Q_snprintf(m_pszStringVersion, sizeof(m_pszStringVersion), "%s %s",
               strVersion, // BuildVerison localization
               MOM_CURRENT_VERSION);
    g_pVGuiLocalize->ConvertANSIToUnicode(m_pszStringVersion, uVersionText, sizeof(m_pszStringVersion));
    SetFont(m_hTextFont);
    SetText(uVersionText);
}