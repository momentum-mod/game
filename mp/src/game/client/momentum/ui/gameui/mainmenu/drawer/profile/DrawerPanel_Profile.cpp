#include "cbase.h"

#include "DrawerPanel_Profile.h"

#include <steam/isteamuser.h>

#include "ProfileActivityPage.h"
#include "ProfileStatsPage.h"

#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Tooltip.h"
#include "controls/UserComponent.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DrawerPanel_Profile::DrawerPanel_Profile(Panel *pParent) : BaseClass(pParent, "DrawerPanel_Profile")
{
    SetProportional(true);
}

void DrawerPanel_Profile::OnResetData()
{
    /*
    m_pUserComponent = new UserComponent(this);

    if (SteamUser())
    {
        m_pUserComponent->SetUser(SteamUser()->GetSteamID().ConvertToUint64());
    }

    m_pStatsAndActivitySheet = new PropertySheet(this, "StatsAndActivity");

    m_pProfileStats = new ProfileStatsPage(this);
    m_pProfileActivity = new ProfileActivityPage(this);

    m_pStatsAndActivitySheet->AddPage(m_pProfileActivity, "#MOM_Drawer_Profile_Activity");
    m_pStatsAndActivitySheet->AddPage(m_pProfileStats, "#MOM_Drawer_Profile_Stats");


    LoadControlSettings("resource/ui/mainmenu/DrawerPanel_Profile.res");
     */
    GetTooltip()->SetText("Your profile is unavailable in this build!");
}