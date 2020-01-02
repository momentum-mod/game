#include "cbase.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include "c_mom_player.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "mom_system_gamemode.h"
#include "weapon/weapon_base.h"
#include "weapon/weapon_mom_stickybomblauncher.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_sj_stickycount_enable, "1", FCVAR_ARCHIVE, "Toggles the stickybomb counter.\n");

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CHudStickybombs : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickybombs, EditablePanel);

  public:
    CHudStickybombs(const char *pElementName);

    void ApplySchemeSettings(IScheme *scheme) OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void OnTick() OVERRIDE;

  private:
    vgui::EditablePanel *m_pStickybombsPresent;
    vgui::EditablePanel *m_pNoStickybombsPresent;
    vgui::Label *m_pStickyLabel;
    vgui::Label *m_pNoStickyLabel;
};

DECLARE_HUDELEMENT(CHudStickybombs);

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CHudStickybombs::CHudStickybombs(const char *pElementName)
    : CHudElement(pElementName), BaseClass(NULL, "CHudStickybombs")
{
    Panel *pParent = g_pClientMode->GetViewport();
    SetParent(pParent);

    m_pStickybombsPresent = new EditablePanel(this, "StickybombsPresentPanel");
    m_pNoStickybombsPresent = new EditablePanel(this, "NoStickybombsPresentPanel");
    m_pStickyLabel = new Label(m_pStickybombsPresent, "NumStickybombsLabel", "");
    m_pNoStickyLabel = new Label(m_pNoStickybombsPresent, "NumNoStickybombsLabel", "");

    SetHiddenBits(HIDEHUD_MISCSTATUS);

    LoadControlSettings("resource/ui/HudStickybombs.res");

    vgui::ivgui()->AddTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudStickybombs::ApplySchemeSettings(IScheme *pScheme) { BaseClass::ApplySchemeSettings(pScheme); }

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudStickybombs::ShouldDraw(void)
{
    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ) || !pPlayer->IsAlive())
        return false;

    if (!mom_sj_stickycount_enable.GetBool())
        return false;

    return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudStickybombs::OnTick(void)
{
    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer)
        return;

    CWeaponBase *pGun = dynamic_cast<CWeaponBase *>(pPlayer->GetActiveWeapon());

    if (!pGun)
        return;

    int iWeaponID = pGun->GetWeaponID();

    if (iWeaponID != WEAPON_STICKYLAUNCHER)
        return;

    CMomentumStickybombLauncher *pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pGun);

    if (!pLauncher)
        return;

    int iStickybombs = pLauncher->GetStickybombCount();

    m_pStickybombsPresent->SetDialogVariable("activestickybombs", iStickybombs);
    m_pNoStickybombsPresent->SetDialogVariable("activestickybombs", iStickybombs);

    m_pStickybombsPresent->SetVisible(iStickybombs > 0);
    m_pNoStickybombsPresent->SetVisible(iStickybombs <= 0);
}