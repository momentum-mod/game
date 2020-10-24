#include "cbase.h"

#include "ProfileStatsPage.h"

#include "mom_system_user_data.h"

#include "tier0/memdbgon.h"

using namespace vgui;

ProfileStatsPage::ProfileStatsPage(Panel *pParent) : BaseClass(pParent, "ProfileStatsPage")
{
    SetProportional(true);
    UpdateDialogVariables();

    g_pUserData->AddUserDataChangeListener(GetVPanel());
}

ProfileStatsPage::~ProfileStatsPage()
{
    g_pUserData->RemoveUserDataChangeListener(GetVPanel());
}

void ProfileStatsPage::OnResetData()
{
    LoadControlSettings("resource/ui/mainmenu/ProfileStatsPage.res");
}

void ProfileStatsPage::UpdateDialogVariables()
{
    const auto pDialogKV = GetDialogVariables();
    pDialogKV->Clear();

    KeyValuesAD statsKV("Stats");
    g_pUserData->GetLocalUserData().SaveToKV(statsKV);

    statsKV->FindKey("stats", true)->CopySubkeys(pDialogKV);

    ForceSubPanelsToUpdateWithNewDialogVariables();
}