#include "cbase.h"

#include "mom_system_user_data.h"

#include "filesystem.h"

#include "vgui_controls/Controls.h"
#include "vgui/IVGui.h"

#include "mom_api_requests.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

#define LOCAL_USER_DATA_FILE "localuser.dat"

UserData::UserData()
{
    m_iID = 0;
    m_uSteamID = 0;
    m_szAlias[0] = '\0';
    m_iCurrentLevel = 0;
    m_iCurrentXP = 0;
    m_iMapsCompleted = 0;
    m_iRunsSubmitted = 0;
    m_iTotalJumps = 0;
    m_iTotalStrafes = 0;
}

bool UserData::ParseFromKV(KeyValues *pUserKv)
{
    const auto pUserStats = pUserKv->FindKey("stats");
    if (!pUserStats)
        return false;

    m_iID = pUserKv->GetInt("id");
    m_uSteamID = pUserKv->GetUint64("steamID");
    Q_strncpy(m_szAlias, pUserKv->GetString("alias"), sizeof(m_szAlias));

    m_iCurrentLevel = pUserStats->GetInt("level");
    m_iCurrentXP = pUserStats->GetInt("CosXP");

    m_iTotalStrafes = pUserStats->GetInt("TotalStrafes");
    m_iTotalJumps = pUserStats->GetInt("TotalJumps");

    m_iMapsCompleted = pUserStats->GetInt("MapsCompleted");
    m_iRunsSubmitted = pUserStats->GetInt("RunsSubmitted");

    return true;
}

void UserData::SaveToKV(KeyValues *pKvOut) const
{
    pKvOut->SetInt("id", m_iID);
    pKvOut->SetString("alias", m_szAlias);
    pKvOut->SetUint64("steamID", m_uSteamID);

    const auto pKvStats = new KeyValues("stats");

    pKvStats->SetInt("level", m_iCurrentLevel);
    pKvStats->SetInt("CosXP", m_iCurrentXP);

    pKvStats->SetInt("TotalStrafes", m_iTotalStrafes);
    pKvStats->SetInt("TotalJumps", m_iTotalJumps);
    pKvStats->SetInt("MapsCompleted", m_iMapsCompleted);
    pKvStats->SetInt("RunsSubmitted", m_iRunsSubmitted);

    pKvOut->AddSubKey(pKvStats);
}

MomentumUserData::MomentumUserData() : CAutoGameSystem("MomentumUserData")
{
}

void MomentumUserData::AddUserDataChangeListener(vgui::VPANEL hPanel)
{
    m_vecUserDataChangeListeners.AddToTail(hPanel);
}

void MomentumUserData::RemoveUserDataChangeListener(vgui::VPANEL hPanel)
{
    m_vecUserDataChangeListeners.FindAndRemove(hPanel);
}

void MomentumUserData::PostInit()
{
    ListenForGameEvent("site_auth");
    ListenForGameEvent("run_upload");

    // Load cached local user data
    KeyValuesAD localCache("LocalUserDataCache");
    if (localCache->LoadFromFile(g_pFullFileSystem, LOCAL_USER_DATA_FILE, "MOD"))
    {
        if (m_LocalUserData.ParseFromKV(localCache))
            FireUserDataUpdate();
    }
}

void MomentumUserData::Shutdown()
{
    SaveLocalUserData();
}

bool MomentumUserData::SaveLocalUserData()
{
    if (m_LocalUserData.m_uSteamID == 0)
        return false;

    KeyValuesAD localCache("LocalUserDataCache");

    m_LocalUserData.SaveToKV(localCache);

    return localCache->SaveToFile(g_pFullFileSystem, LOCAL_USER_DATA_FILE, "MOD");
}

void MomentumUserData::FireGameEvent(IGameEvent *event)
{
    CHECK_STEAM_API(SteamUser());

    if ( ( FStrEq(event->GetName(), "site_auth") && !event->GetBool("success") ) ||
         ( FStrEq(event->GetName(), "run_upload") && !event->GetBool("run_posted") ) )
        return;

    g_pAPIRequests->GetUserStats(SteamUser()->GetSteamID().ConvertToUint64(), UtlMakeDelegate(this, &MomentumUserData::OnUserDataReceived));
}

CON_COMMAND_F(test_user_data_message, "Fires user data message\n", FCVAR_DEVELOPMENTONLY)
{
    g_pUserData->FireUserDataUpdate();
}

void MomentumUserData::FireUserDataUpdate()
{
    FOR_EACH_VEC(m_vecUserDataChangeListeners, i)
    {
        vgui::ivgui()->PostMessage(m_vecUserDataChangeListeners[i], new KeyValues("UserDataUpdate"), 0.0f);
    }

    SaveLocalUserData();
}

void MomentumUserData::OnUserDataReceived(KeyValues *pKv)
{
    const auto pData = pKv->FindKey("data");
    const auto pErr = pKv->FindKey("error");
    if (pData)
    {
        const auto pUser = pData->FindKey("users", true)->FindKey("1");
        if (!pUser)
            return;

        if (!m_LocalUserData.ParseFromKV(pUser))
            return;

        FireUserDataUpdate();
    }
    else if (pErr)
    {
        Warning("Failed to fetch local user data!\n");
    }
}

static MomentumUserData s_MomentumUserData;
MomentumUserData *g_pUserData = &s_MomentumUserData;