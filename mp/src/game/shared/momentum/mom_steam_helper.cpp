#include "mom_steam_helper.h"

extern CSteamAPIContext* steamapicontext;

CMomentumSteamHelper::CMomentumSteamHelper() : m_siLobby(), m_i32CurrentTotalPlayers(0), m_bCachedLobbyMembersValid(false)
{

}

CSteamID CMomentumSteamHelper::GetLocalSteamID()
{
    return steamapicontext->SteamUser()->GetSteamID();
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
    if (!m_cbPlayersCallback.IsActive())
    {
        m_cbPlayersCallback.Set(steamapicontext->SteamUserStats()->GetNumberOfCurrentPlayers(), this, &CMomentumSteamHelper::OnNumberOfCurrentPlayers);
    }
}

void CMomentumSteamHelper::CheckLobby()
{
    // If we currently think we're on a lobby but the 0th member is not valid...
    if (IsLobbyValid() && !steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(GetCurrentLobby(), 0).IsValid())
    {
        // it means we're actually not on a lobby!
        m_siLobby = k_steamIDNil;
        m_bCachedLobbyMembersValid = false; // Invalidate the cache too just in case
    }
}

void CMomentumSteamHelper::SetLobbyMemberData(const char* key, const char* value) const
{
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(GetCurrentLobby(), key, value);
}

int32 CMomentumSteamHelper::GetCurrentTotalPlayers() const
{
    return m_i32CurrentTotalPlayers;
}

const wchar_t* CMomentumSteamHelper::GetCurrentTotalPlayersAsString() const
{
    return &m_wsCurrentTotalPlayers[0];
}

const char* CMomentumSteamHelper::GetLobbyMemberData(const CSteamID member, const char* key) const
{
    return steamapicontext->SteamMatchmaking()->GetLobbyMemberData(GetCurrentLobby(), member, key);
}

const char* CMomentumSteamHelper::GetLobbyLocalMemberData(const char* key) const
{
    return GetLobbyMemberData(GetLocalSteamID(), key);
}

bool CMomentumSteamHelper::GetLobbyMembers(CUtlVector<CSteamID>& vec)
{
    if (IsLobbyValid())
    {
        if (m_bCachedLobbyMembersValid)
        {
            vec = m_vCachedLobbyMembers;
        }
        else
        {
            const int membersCount = steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(GetCurrentLobby());
            vec.SetCount(membersCount);
            for (int i = 0; i < membersCount; ++i)
            {
                vec[i] = steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(GetCurrentLobby(), i);
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
