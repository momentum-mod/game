#include "mom_steam_helper.h"
#include "steam/steam_api.h"

extern IGameEventManager2 *gameeventmanager;

CMomentumSteamHelper::CMomentumSteamHelper() : m_siLobby(), m_i32CurrentTotalPlayers(0), m_bCachedLobbyMembersValid(false)
{
    SteamAPI_InitSafe();
    SteamAPI_SetTryCatchCallbacks(false);
    steamapicontext.Init();
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
    return steamapicontext.SteamUser() ? steamapicontext.SteamUser()->GetSteamID() : k_steamIDNil;
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
    if (!m_cbPlayersCallback.IsActive() && steamapicontext.SteamUserStats())
    {
        m_cbPlayersCallback.Set(steamapicontext.SteamUserStats()->GetNumberOfCurrentPlayers(), this, &CMomentumSteamHelper::OnNumberOfCurrentPlayers);
    }
}

void CMomentumSteamHelper::SetLobbyMemberData(const char* key, const char* value)
{
    if (steamapicontext.SteamMatchmaking())
        steamapicontext.SteamMatchmaking()->SetLobbyMemberData(GetCurrentLobby(), key, value);
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
    return steamapicontext.SteamMatchmaking() ?
        steamapicontext.SteamMatchmaking()->GetLobbyMemberData(GetCurrentLobby(), member, key) :
        "";
}

const char* CMomentumSteamHelper::GetLobbyLocalMemberData(const char* key)
{
    return GetLobbyMemberData(GetLocalSteamID(), key);
}

bool CMomentumSteamHelper::GetLobbyMembers(CUtlVector<CSteamID>& vec)
{
    if (IsLobbyValid() && steamapicontext.SteamMatchmaking())
    {
        if (m_bCachedLobbyMembersValid)
        {
            vec = m_vCachedLobbyMembers;
        }
        else
        {
            const int membersCount = steamapicontext.SteamMatchmaking()->GetNumLobbyMembers(GetCurrentLobby());
            vec.SetCount(membersCount);
            for (int i = 0; i < membersCount; ++i)
            {
                vec[i] = steamapicontext.SteamMatchmaking()->GetLobbyMemberByIndex(GetCurrentLobby(), i);
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
