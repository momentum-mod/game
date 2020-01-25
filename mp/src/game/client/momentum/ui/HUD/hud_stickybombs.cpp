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
#include "weapon/weapon_mom_stickybomblauncher.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_sj_stickycount_enable, "1", FCVAR_ARCHIVE, "Toggles the stickybomb counter.\n");

class CHudStickybombs : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickybombs, EditablePanel);

  public:
    CHudStickybombs(const char *pElementName);

    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Reset() OVERRIDE;

  private:
    CMomentumStickybombLauncher *pLauncher;
    Label *m_pStickyLabel;
};

DECLARE_HUDELEMENT(CHudStickybombs);

CHudStickybombs::CHudStickybombs(const char *pElementName)
    : CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "CHudStickybombs")
{
    m_pStickyLabel = new Label(this, "StickybombsLabel", "");

    pLauncher = nullptr;

    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    LoadControlSettings("resource/ui/HudStickybombs.res");
}

void CHudStickybombs::Reset()
{
    pLauncher = nullptr;
}

bool CHudStickybombs::ShouldDraw()
{
    if (!mom_hud_sj_stickycount_enable.GetBool())
        return false;

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ) || !pPlayer->IsAlive())
        return false;

    return CHudElement::ShouldDraw();
}

void CHudStickybombs::OnThink()
{
    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pPlayer->GetActiveWeapon());

    if (!pPlayer || !pLauncher)
        return;

    int iStickybombs = pLauncher->GetStickybombCount();
    if (iStickybombs < 1)
        m_pStickyLabel->SetVisible(false);
    else
        m_pStickyLabel->SetVisible(true);

    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%u", iStickybombs);

    m_pStickyLabel->SetText(buf);
}