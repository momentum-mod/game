#include "mom_steam_helper.h"
#include "steam/steam_api.h"

extern IGameEventManager2 *gameeventmanager;

CMomentumSteamHelper::CMomentumSteamHelper() : m_siLobby(), m_i32CurrentTotalPlayers(0), m_bCachedLobbyMembersValid(false)
{
    SteamAPI_Init();
    SteamAPI_SetTryCatchCallbacks(false);
    ListenForGameEvent("lobby_leave");
}

void CMomentumSteamHelper::FireGameEvent(IGameEvent* event)
{
    if (!Q_strcmp(event->GetName(), "lobby_leave"))
    {
        m_siLobby = k_steamIDNil;
    }
}

CSteamID CMomentumSteamHelper::GetLocalSteamID()
{
    return SteamUser() ? SteamUser()->GetSteamID() : k_steamIDNil;
}

CSteamID CMomentumSteamHelper::GetCurrentLobby() const
{
    return m_siLobby;
}

bool CMomentumSteamHelper::IsLobbyValid() const
{
    return GetCurrentLobby().IsValid() && GetCurrentLobby().IsLobby();
}

void CMomentumSteamHelper::RequestCurrentTotalPlayers()
{
    if (!m_cbPlayersCallback.IsActive() && SteamUserStats())
    {
        m_cbPlayersCallback.Set(SteamUserStats()->GetNumberOfCurrentPlayers(), this, &CMomentumSteamHelper::OnNumberOfCurrentPlayers);
    }
}

void CMomentumSteamHelper::SetLobbyMemberData(const char* key, const char* value)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyMemberData(GetCurrentLobby(), key, value);
}

int32 CMomentumSteamHelper::GetCurrentTotalPlayers() const
{
    return m_i32CurrentTotalPlayers;
}

const wchar_t* CMomentumSteamHelper::GetCurrentTotalPlayersAsString() const
{
    return &m_wsCurrentTotalPlayers[0];
}

const char* CMomentumSteamHelper::GetLobbyMemberData(const CSteamID member, const char* key)
{
    return SteamMatchmaking() ?
        SteamMatchmaking()->GetLobbyMemberData(GetCurrentLobby(), member, key) :
        "";
}

const char* CMomentumSteamHelper::GetLobbyLocalMemberData(const char* key)
{
    return GetLobbyMemberData(GetLocalSteamID(), key);
}

bool CMomentumSteamHelper::GetLobbyMembers(CUtlVector<CSteamID>& vec)
{
    if (IsLobbyValid() && SteamMatchmaking())
    {
        if (m_bCachedLobbyMembersValid)
        {
            vec = m_vCachedLobbyMembers;
        }
        else
        {
            const int membersCount = SteamMatchmaking()->GetNumLobbyMembers(GetCurrentLobby());
            vec.SetCount(membersCount);
            for (int i = 0; i < membersCount; ++i)
            {
                vec[i] = SteamMatchmaking()->GetLobbyMemberByIndex(GetCurrentLobby(), i);
            }
            m_bCachedLobbyMembersValid = true;
            return true;
        }
    }
    return false;
}

void CMomentumSteamHelper::OnLobbyEnter(LobbyEnter_t* pParam)
{
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        m_siLobby = pParam->m_ulSteamIDLobby;
        m_bCachedLobbyMembersValid = false; // Just in case...
    }
}

void CMomentumSteamHelper::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    if (pParam->m_ulSteamIDUserChanged != GetLocalSteamID().ConvertToUint64())
    {
        // Invalidate cache
        m_bCachedLobbyMembersValid = false;
    }
}

void CMomentumSteamHelper::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    if (pParam->m_bSuccess)
    {
       
    }
}

void CMomentumSteamHelper::OnNumberOfCurrentPlayers(NumberOfCurrentPlayers_t* pParam, bool bIOerror)
{
    if (!bIOerror && pParam->m_bSuccess)
    {
        m_i32CurrentTotalPlayers = pParam->m_cPlayers;
        char currentPlayers[11];
        Q_snprintf(currentPlayers, 11, "%010d", pParam->m_cPlayers);
        Q_strtowcs(currentPlayers, 11, m_wsCurrentTotalPlayers, BUFSIZ);
    }
}

static CMomentumSteamHelper s_MOMSteamHelper;
CMomentumSteamHelper *g_pMomentumSteamHelper = &s_MOMSteamHelper;
