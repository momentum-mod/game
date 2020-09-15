#include "cbase.h"

#include "UserComponent.h"

#include <steam/isteamuser.h>

#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui_controls/Tooltip.h"
#include "vgui_avatarimage.h"

#include "fmtstr.h"
#include "ilocalize.h"
#include "mom_shareddefs.h"
#include "mom_system_user_data.h"
#include "util/mom_system_xp.h"

#include "tier0/memdbgon.h"
#include "vgui/ILocalize.h"

using namespace vgui;

UserComponent::UserComponent(Panel* pParent, const char *pName /*= "UserComponent"*/) : BaseClass(pParent, pName)
{
    SetProportional(true);
    SetPaintBackgroundEnabled(true);
    SetMouseInputEnabled(true);

    m_bClickable = false;
    m_pUserImage = new CAvatarImagePanel(this, "UserImage");
    m_pUserName = new Label(this, "UserName", "...");
    m_pUserRank = new Label(this, "UserRank", "...");
    m_pXPProgressBar = new ContinuousProgressBar(this, "XPProgress");
    m_pXPProgressBar->SetProgress(0);

    LoadControlSettings("resource/ui/UserComponent.res");
}

UserComponent::~UserComponent()
{
    g_pUserData->RemoveUserDataChangeListener(GetVPanel());
}

void UserComponent::SetClickable(bool bClickable)
{
    m_bClickable = bClickable;

    const auto hCursor = bClickable ? dc_hand : dc_arrow;

    SetCursor(hCursor);
    m_pUserImage->SetCursor(hCursor);
    m_pUserName->SetCursor(hCursor);
    m_pUserRank->SetCursor(hCursor);
    m_pXPProgressBar->SetCursor(hCursor);
}

void UserComponent::SetUser(uint64 uID)
{
    CHECK_STEAM_API(SteamFriends());
    CHECK_STEAM_API(SteamUser());

    const CSteamID userID(uID);

    m_pUserImage->SetPlayer(userID, k_EAvatarSize184x184);

    m_pUserName->SetText(SteamFriends()->GetFriendPersonaName(userID));

    if (uID == SteamUser()->GetSteamID().ConvertToUint64())
    {
        g_pUserData->AddUserDataChangeListener(GetVPanel());
        FillControlsWithUserData(g_pUserData->GetLocalUserData());
    }
    else
    {
        Warning("UserComponent::SetUser MOM_TODO: Implement me!!\n");
    }

    InvalidateLayout();
}

void UserComponent::OnUserDataUpdate()
{
    FillControlsWithUserData(g_pUserData->GetLocalUserData());
}

void UserComponent::FillControlsWithUserData(const UserData &userData)
{
    m_pUserName->SetText(userData.m_szAlias);

    int curLvl = userData.m_iCurrentLevel;
    int curXP = userData.m_iCurrentXP;
    int xpForLvl = g_pXPSystem->GetCosmeticXPForLevel(curLvl + 1);
    float progress = static_cast<float>(curXP) / static_cast<float>(xpForLvl);

    m_pXPProgressBar->SetProgress(progress);
    m_pXPProgressBar->GetTooltip()->SetText(CConstructLocalizedString(g_pVGuiLocalize->FindSafe("#MOM_UserComponent_XP_For_Next_Level"), xpForLvl - curXP, curLvl + 1));

    m_pUserRank->SetVisible(true);
    m_pUserRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->FindSafe("#MOM_UserComponent_Level"), curLvl));

    InvalidateLayout();
}

void UserComponent::OnReloadControls()
{
    BaseClass::OnReloadControls();

    FillControlsWithUserData(g_pUserData->GetLocalUserData());
}

void UserComponent::PerformLayout()
{
    BaseClass::PerformLayout();

    int pWide, pTall;
    GetSize(pWide, pTall);

    m_pUserImage->SetBounds(0, 0, pTall, pTall);

    m_pXPProgressBar->SetWide(pWide);
}

void UserComponent::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
}

void UserComponent::OnMouseReleased(MouseCode code)
{
    if (code == MOUSE_LEFT && m_bClickable)
    {
        PostActionSignal(new KeyValues("UserComponentClicked"));
    }
}