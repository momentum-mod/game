#ifndef MOM_STEAM_HELPER_H
#define MOM_STEAM_HELPER_H
#ifdef _WIN32
#pragma once
#endif

#include <platform.h>
#include <strtools.h>

#include <tier1/utlvector.h>
#include "steam/steam_api.h"
#include "../GameEventListener.h"

class CMomentumSteamHelper : public CGameEventListener
{
public:
    CMomentumSteamHelper();

    CMomentumSteamHelper(const CMomentumSteamHelper&) = delete;
    CMomentumSteamHelper(CMomentumSteamHelper&&) = delete;
    CMomentumSteamHelper& operator=(const CMomentumSteamHelper&) = delete;
    CMomentumSteamHelper& operator=(CMomentumSteamHelper&&) = delete;
    ~CMomentumSteamHelper() = default;

    // Override
    void FireGameEvent(IGameEvent* event) OVERRIDE;

    // Getters

    // Local player steam id
    CSteamID GetLocalSteamID();
    // Current Lobby Id
    CSteamID GetCurrentLobby() const;
    // Is the current lobby valid?
    bool IsLobbyValid() const;
    // Get the most up-to-date number of current total players we have
    int32 GetCurrentTotalPlayers() const;
    // Get the most up-to-date number of current total players we have as string
    const wchar_t* GetCurrentTotalPlayersAsString() const;
    // For a given member steamid, gets the value stored for key
    const char* GetLobbyMemberData(const CSteamID member, const char* key);
    // Gets the value stored for key for the local user.
    const char* GetLobbyLocalMemberData(const char* key);
    // Copies into vec the current list of lobby members. Returns true if cache missed
    bool GetLobbyMembers(CUtlVector<CSteamID>& vec);

    // Updaters

    // Ask for an update on the current total players
    void RequestCurrentTotalPlayers();
    // Sets lobby data for the current player
    void SetLobbyMemberData(const char* key, const char* value);


private:
    // Callbacks
    STEAM_CALLBACK(CMomentumSteamHelper, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(CMomentumSteamHelper, OnLobbyChatUpdate, LobbyChatUpdate_t);
    STEAM_CALLBACK(CMomentumSteamHelper, OnLobbyDataUpdate, LobbyDataUpdate_t);

    void OnNumberOfCurrentPlayers(NumberOfCurrentPlayers_t*, bool);
    CCallResult<CMomentumSteamHelper, NumberOfCurrentPlayers_t> m_cbPlayersCallback;

    // Data members

    // Id of the current lobby.
    CSteamID m_siLobby;

    // Total amount of players right now
    int32 m_i32CurrentTotalPlayers;
    wchar_t m_wsCurrentTotalPlayers[11];

    // (Cached) current players on the lobby. Updated on request if necessary
    CUtlVector<CSteamID> m_vCachedLobbyMembers;
    // Flag that indicates if the next request should re-fetch the lobby members from steam
    bool m_bCachedLobbyMembersValid;

    CSteamAPIContext steamapicontext;

};

extern CMomentumSteamHelper *g_pMomentumSteamHelper;

#endif // MOM_STEAM_HELPER_H