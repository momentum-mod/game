#include "cbase.h"

#include "ProfileActivityPage.h"

#include "util/mom_util.h"

#include "time.h"

#include "mom_api_models.h"
#include "mom_api_requests.h"
#include "mom_system_user_data.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/ListPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

struct UserActivity
{
    Run m_Run;
    MapRank m_Rank;
    User m_User;
    MapData m_Map;
};

enum ActivityHeaders_t
{
    HEADER_RANK = 0,
    HEADER_TIME,
    HEADER_MAP,
    HEADER_DATE
};

static int __cdecl ActivitySortFunc(ListPanel *pPanel, const ListPanelItem &item1, const ListPanelItem &item2)
{
    uint64 date1 = item1.kv->GetUint64("date_t");
    uint64 date2 = item2.kv->GetUint64("date_t");

    if (date1 < date2)
        return 1;

    if (date2 > date1)
        return -1;

    return 0;
}

ProfileActivityPage::ProfileActivityPage(Panel *pParent): BaseClass(pParent, "ProfileActivityPage")
{
    SetProportional(true);

    m_pActivityPanel = new ListPanel(this, "ActivityList");

    m_pActivityPanel->AddColumnHeader(HEADER_RANK, "rank", "#MOM_Rank", GetScaledVal(40), ListPanel::COLUMN_FIXEDSIZE);
    m_pActivityPanel->AddColumnHeader(HEADER_TIME, "time", "#MOM_Time", GetScaledVal(80), GetScaledVal(80), GetScaledVal(120), 0);
    m_pActivityPanel->AddColumnHeader(HEADER_MAP, "map", "#MOM_Map", GetScaledVal(100), GetScaledVal(100), GetScaledVal(350), 0);
    m_pActivityPanel->AddColumnHeader(HEADER_DATE, "date", "#MOM_Date", GetScaledVal(80), GetScaledVal(80), 9001, 0);

    m_pActivityPanel->SetSortColumn(HEADER_DATE);
    m_pActivityPanel->SetSortFunc(HEADER_DATE, ActivitySortFunc);

    m_pActivityPanel->SetColumnSortable(HEADER_RANK, false);
    m_pActivityPanel->SetColumnSortable(HEADER_TIME, false);
    m_pActivityPanel->SetColumnSortable(HEADER_MAP, false);

    m_pActivityPanel->SetColumnHeaderTextAlignment(HEADER_RANK, Label::Alignment::a_center);
    m_pActivityPanel->SetColumnTextAlignment(HEADER_RANK, Label::Alignment::a_center);

    m_pActivityPanel->SetAutoTallHeaderToFont(true);
    m_pActivityPanel->SetRowHeightOnFontChange(false);
    m_pActivityPanel->SetRowHeight(GetScaledVal(26));


    m_pActivityPanel->SetShouldCenterEmptyListText(true);
    m_pActivityPanel->SetEmptyListText("#MOM_API_WaitingForResponse");

    g_pUserData->AddUserDataChangeListener(GetVPanel());
}

ProfileActivityPage::~ProfileActivityPage()
{
    g_pUserData->RemoveUserDataChangeListener(GetVPanel());
}

void ProfileActivityPage::PerformLayout()
{
    BaseClass::PerformLayout();

    int wide, tall;
    GetSize(wide, tall);
    m_pActivityPanel->SetSize(wide, tall);
}

void ProfileActivityPage::OnResetData()
{
    LoadControlSettings("resource/ui/mainmenu/ProfileActivityPage.res");
}

void ProfileActivityPage::OnUserDataUpdate()
{
    FetchRecentActivity();
}

void ProfileActivityPage::OnUserActivityReceived(KeyValues *pKv)
{
    const auto pData = pKv->FindKey("data");
    const auto pErr = pKv->FindKey("error");
    if (pData)
    {
        const auto iCount = pData->GetInt("count");
        if (iCount <= 0)
        {
            m_pActivityPanel->SetEmptyListText("#MOM_API_NoActivityReturned");
            return;
        }

        const auto pKvRuns = pData->FindKey("runs");
        if (!pKvRuns)
            return;

        CUtlVector<UserActivity> vecUserActivities;

        FOR_EACH_SUBKEY(pKvRuns, pKvRunData)
        {
            UserActivity userAct;
            userAct.m_Run.FromKV(pKvRunData);

            const auto pKvUserData = pKvRunData->FindKey("user");
            const auto pKvRank = pKvRunData->FindKey("rank");
            const auto pKvMapData = pKvRunData->FindKey("map");
            if (!pKvUserData || !pKvRank || !pKvMapData)
                continue;

            userAct.m_User.FromKV(pKvUserData);
            userAct.m_Rank.FromKV(pKvRank);

            userAct.m_Map.m_eSource = MODEL_FROM_INFO_API_CALL;
            userAct.m_Map.FromKV(pKvMapData);

            vecUserActivities.AddToTail(userAct);
        }

        if (!vecUserActivities.IsEmpty())
        {
            UpdateRecentActivity(vecUserActivities);
        }
    }
    else if (pErr)
    {
        m_pActivityPanel->SetEmptyListText("#MOM_API_ServerError");

        // MOM_TODO error handle
    }
}

void ProfileActivityPage::FetchRecentActivity()
{
    const auto hData = g_pUserData->GetLocalUserData();

    KeyValuesAD filters("filters");
    filters->SetInt("limit", 20);

    g_pAPIRequests->GetUserRunHistory(hData.m_iID, UtlMakeDelegate(this, &ProfileActivityPage::OnUserActivityReceived), filters);
}

void ProfileActivityPage::UpdateRecentActivity(const CUtlVector<UserActivity> &vecActivity)
{
    m_pActivityPanel->RemoveAll();

    FOR_EACH_VEC(vecActivity, i)
    {
        const auto hActivity = vecActivity[i];

        KeyValuesAD pActivityKV("Activity");

        if (hActivity.m_Rank.m_iRank)
        {
            pActivityKV->SetInt("rank", hActivity.m_Rank.m_iRank);
        }
        else
        {
            pActivityKV->SetString("rank", "-");
        }

        char timeOut[64];
        MomUtil::FormatTime(hActivity.m_Run.m_fTime, timeOut, 2);
        pActivityKV->SetString("time", timeOut);

        pActivityKV->SetString("map", hActivity.m_Map.m_szMapName);

        time_t date_t = 0;
        if (MomUtil::ISODateToTimeT(hActivity.m_Run.m_szDateAchieved, &date_t))
        {
            wchar_t date[32];
            wcsftime(date, 32, L"%b %d, %Y", localtime(&date_t));
            pActivityKV->SetWString("date", date);

            pActivityKV->SetUint64("date_t", date_t);
        }
        else
        {
            pActivityKV->SetString("date", "ERROR!");
            pActivityKV->SetUint64("date_t", time(nullptr));
        }

        m_pActivityPanel->AddItem(pActivityKV, 0, false, true);
    }
}