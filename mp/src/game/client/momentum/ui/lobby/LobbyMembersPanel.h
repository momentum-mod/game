#pragma once

#include "vgui_controls/EditablePanel.h"
#include "steam/steam_api_common.h"
#include "steam/isteammatchmaking.h"
#include "GameEventListener.h"
#include "game/client/iviewport.h"

class CLeaderboardsContextMenu;
class SavelocReqFrame;

class LobbyMembersPanel : public IViewPortPanel, public vgui::EditablePanel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(LobbyMembersPanel, vgui::EditablePanel);

    LobbyMembersPanel(IViewPort *pParent);
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

    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) OVERRIDE { return BaseClass::SetParent(parent); }
    vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }

protected:
    const char* GetName(void) OVERRIDE;
    void SetData(KeyValues *data) OVERRIDE {}
    void Reset(void) OVERRIDE;
    void Update(void) OVERRIDE {}
    bool NeedsUpdate(void) OVERRIDE {return false;}
    bool HasInputElements(void) OVERRIDE {return true;}
    void ShowPanel(bool state) OVERRIDE;

    void OnCommand(const char* command) OVERRIDE;

    MESSAGE_FUNC_PARAMS(OnItemContextMenu, "ItemContextMenu", data);
    MESSAGE_FUNC_CHARPTR(OnContextGoToMap, "ContextGoToMap", map);
    MESSAGE_FUNC_UINT64(OnContextVisitProfile, "ContextVisitProfile", profile);
    MESSAGE_FUNC_UINT64(OnSpectateLobbyMember, "ContextSpectate", target);
    MESSAGE_FUNC_UINT64(OnContextReqSavelocs, "ContextReqSavelocs", target);

private:
    CSteamID m_idLobby;
    void PopulateLobbyPanel();
    void InitLobbyPanelSections();

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID, CUtlMap<uint64, int> *pMap, vgui::ImageList *pList);

    CUtlMap<uint64, int> m_mapLobbyIDToImageListIndx;

    vgui::SectionedListPanel *m_pMemberList;
    vgui::Button *m_pLobbyToggle, *m_pInviteFriends;
    CLeaderboardsContextMenu *m_pContextMenu;
    SavelocReqFrame *m_pSavelocReqFrame;

    vgui::ImageList *m_pImageListLobby;
    int m_iSectionId;
};
