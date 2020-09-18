#include "cbase.h"

#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>

#include "c_mom_player.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_system_gamemode.h"
#include "weapon/weapon_mom_stickybomblauncher.h"

#include "fmtstr.h"

#include "tier0/memdbgon.h"

#define STICKY_COUNT_ARM_ANIM_NAME "FadeInStickyArm"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_sj_stickycount_enable, "1", FCVAR_ARCHIVE, "Toggles the stickybomb counter.\n");

class Stickybox : public Panel
{
    DECLARE_CLASS_SIMPLE(Stickybox, Panel);

  public:
    Stickybox(Panel *pParent);

    bool HasAnimationStarted() const { return m_bAnimationStarted; }

    void StartAnimation(Color startColor);
    void StopAnimation();

    void OnThink() override;

  private:
    CPanelAnimationVar(Color, m_StickyColor, "StickyColor", "BlackHO");

    bool m_bAnimationStarted;
};

Stickybox::Stickybox(Panel *pParent) : BaseClass(pParent, "Stickybox"), m_bAnimationStarted(false)
{}

void Stickybox::StartAnimation(Color startColor)
{
    m_StickyColor = startColor;
    GetAnimationController()->StartAnimationSequenceForPanel(this, STICKY_COUNT_ARM_ANIM_NAME);
    m_bAnimationStarted = true;
}

void Stickybox::StopAnimation()
{
    GetAnimationController()->StopAnimationSequenceForPanel(this, STICKY_COUNT_ARM_ANIM_NAME);
    m_bAnimationStarted = false;
}

void Stickybox::OnThink()
{
    if (m_bAnimationStarted)
        SetBgColor(m_StickyColor);
}

class CHudStickybombs : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudStickybombs, EditablePanel);

  public:
    CHudStickybombs(const char *pElementName);

    bool ShouldDraw() override;
    void OnThink() override;

  private:
    Label *m_pStickyLabel;
};

DECLARE_HUDELEMENT(CHudStickybombs);

CHudStickybombs::CHudStickybombs(const char *pElementName)
    : CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "CHudStickybombs")
{
    m_pStickyLabel = new Label(this, "StickybombsLabel", "");

    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    LoadControlSettings("resource/ui/HudStickybombs.res");
}

bool CHudStickybombs::ShouldDraw()
{
    if (!mom_hud_sj_stickycount_enable.GetBool() || !g_pGameModeSystem->GameModeIs(GAMEMODE_SJ))
        return false;

    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer || !pPlayer->IsAlive())
        return false;

    return CHudElement::ShouldDraw();
}

void CHudStickybombs::OnThink()
{
    C_MomentumPlayer *pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;

    const auto pStickyPtr = pPlayer->GetWeapon(WEAPON_STICKYLAUNCHER);
    if (!pStickyPtr)
        return;

    const auto pStickylauncher = static_cast<CMomentumStickybombLauncher *>(pStickyPtr);

    const auto iStickybombs = pStickylauncher->GetStickybombCount();

    m_pStickyLabel->SetVisible(mom_hud_sj_stickycount_autohide.GetBool() ? iStickybombs > 0 : true);
    m_pStickyLabel->SetText(CFmtStr("%u", iStickybombs).Get());
}