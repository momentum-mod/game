#include "cbase.h"

#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>

#include "c_mom_player.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_system_gamemode.h"
#include "weapon/weapon_mom_stickybomblauncher.h"
#include "weapon/weapon_shareddefs.h"

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
    void ApplySchemeSettings(IScheme *pScheme) override;
    void PerformLayout() override;

  private:
    CUtlVector<Stickybox *> m_Stickyboxes;

    CPanelAnimationVar(int, m_iBoxDimension, "BoxDimension", "8");
    CPanelAnimationVar(Color, m_BgColor, "BgColor", "Blank");
    CPanelAnimationVar(Color, m_PreArmColor, "PreArmColor", "BlackHO");
    CPanelAnimationVar(Color, m_NoStickyColor, "NoStickyColor", "BlackHO");
};

DECLARE_HUDELEMENT(CHudStickybombs);

CHudStickybombs::CHudStickybombs(const char *pElementName)
    : CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "CHudStickybombs")
{
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_Stickyboxes.EnsureCount(MOM_WEAPON_STICKYBOMB_COUNT);
    for (int i = 0; i < MOM_WEAPON_STICKYBOMB_COUNT; i++)
    {
        m_Stickyboxes[i] = new Stickybox(this);
    }

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

    for (int i = 0; i < MOM_WEAPON_STICKYBOMB_COUNT; i++)
    {
        if (i < iStickybombs)
        {
            if (!m_Stickyboxes[i]->HasAnimationStarted())
            {
                m_Stickyboxes[i]->StartAnimation(m_PreArmColor);
            }
        }
        else if (m_Stickyboxes[i]->HasAnimationStarted())
        {
            if (i > 0 && i >= iStickybombs)
            {
                // moving into left-most box, so swap positions
                V_swap(m_Stickyboxes[i], m_Stickyboxes[0]);
                InvalidateLayout();
            }

            m_Stickyboxes[i]->StopAnimation();
            m_Stickyboxes[i]->SetBgColor(m_NoStickyColor);
        }
    }
}

void CHudStickybombs::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(m_BgColor);

    for (int i = 0; i < MOM_WEAPON_STICKYBOMB_COUNT; i++)
    {
        m_Stickyboxes[i]->SetBgColor(m_NoStickyColor);
        m_Stickyboxes[i]->SetBorder(pScheme->GetBorder("StickyboxBorder"));
    }
}

void CHudStickybombs::PerformLayout()
{
    BaseClass::PerformLayout();

    int iSpacePerBox = GetWide() / MOM_WEAPON_STICKYBOMB_COUNT;
    int iXPosAcc = 0;
    for (int i = 0; i < MOM_WEAPON_STICKYBOMB_COUNT; i++)
    {
        int iScaledBoxDimension = GetScaledVal(m_iBoxDimension);

        m_Stickyboxes[i]->SetWide(iScaledBoxDimension);
        m_Stickyboxes[i]->SetTall(iScaledBoxDimension);
        m_Stickyboxes[i]->SetPos(iXPosAcc + (iSpacePerBox - iScaledBoxDimension) / 2,
                                 GetTall() / 2 - iScaledBoxDimension / 2);

        iXPosAcc += iSpacePerBox;
    }
}
