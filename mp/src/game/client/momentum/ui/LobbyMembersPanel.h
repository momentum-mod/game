#pragma once
#include "vgui_controls/SectionedListPanel.h"
#include "steam/steam_api.h"
#include "GameEventListener.h"

class LobbyMembersPanel : public vgui::SectionedListPanel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(LobbyMembersPanel, vgui::SectionedListPanel);

    LobbyMembersPanel(Panel *pParent);
    ~LobbyMembersPanel();

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyCreated, LobbyCreated_t); // When we create a lobby
    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyEnter, LobbyEnter_t); // When we enter a lobby
    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyDataUpdate, LobbyDataUpdate_t); // People/lobby updates status
    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyChatUpdate, LobbyChatUpdate_t); // People join/leave

    void AddLobbyMember(const CSteamID &steamID); // Adds a lobby member to the panel
    void UpdateLobbyMemberData(const CSteamID &memberID); // Updates the lobby member's status data on the panel

    static bool StaticLobbyMemberSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

    // finds a player in the lobby data panel
    int FindItemIDForLobbyMember(uint64 steamID);
    int FindItemIDForLobbyMember(const CSteamID &id) { return FindItemIDForLobbyMember(id.ConvertToUint64()); }

private:
    CSteamID m_idLobby;
    void PopulateLobbyPanel();
    void InitLobbyPanelSections();

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID, CUtlMap<uint64, int> *pMap, vgui::ImageList *pList);

    CUtlMap<uint64, int> m_mapLobbyIDToImageListIndx;

    vgui::ImageList *m_pImageListLobby;
    int m_iSectionId;
};
