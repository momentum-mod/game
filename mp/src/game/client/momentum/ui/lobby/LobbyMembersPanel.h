#pragma once

#include "vgui_controls/EditablePanel.h"
#include "steam/steam_api_common.h"
#include "steam/isteammatchmaking.h"
#include "GameEventListener.h"
#include "game/client/iviewport.h"

namespace vgui {
    class ListPanelItem;
}

class CLeaderboardsContextMenu;
class SavelocReqFrame;

class LobbyMembersPanel : public IViewPortPanel, public vgui::EditablePanel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(LobbyMembersPanel, vgui::EditablePanel);

    LobbyMembersPanel(IViewPort *pParent);
    ~LobbyMembersPanel();

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyEnter, LobbyEnter_t); // When we enter a lobby
    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyDataUpdate, LobbyDataUpdate_t); // People/lobby updates status
    STEAM_CALLBACK(LobbyMembersPanel, OnLobbyChatUpdate, LobbyChatUpdate_t); // People join/leave

    void AddLobbyMember(const CSteamID &steamID); // Adds a lobby member to the panel
    void UpdateLobbyMemberData(const CSteamID &memberID); // Updates the lobby member's status data on the panel

    static int StaticLobbyMemberSortFunc(vgui::ListPanel *list, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2);

    // finds a player in the lobby data panel
    int FindItemIDForLobbyMember(uint64 steamID);
    int FindItemIDForLobbyMember(const CSteamID &id) { return FindItemIDForLobbyMember(id.ConvertToUint64()); }

    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) OVERRIDE { return BaseClass::SetParent(parent); }
    vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }

    void OnMousePressed(vgui::MouseCode code) OVERRIDE;

protected:
    const char* GetName(void) OVERRIDE;
    void SetData(KeyValues *data) OVERRIDE {}
    void Reset(void) OVERRIDE;
    void Update(void) OVERRIDE {}
    bool NeedsUpdate(void) OVERRIDE {return false;}
    bool HasInputElements(void) OVERRIDE {return true;}
    void ShowPanel(bool state) OVERRIDE;

    void OnCommand(const char* command) OVERRIDE;

    MESSAGE_FUNC_INT(OnItemContextMenu, "OpenContextMenu", itemID);
    MESSAGE_FUNC_CHARPTR(OnContextGoToMap, "ContextGoToMap", map);
    MESSAGE_FUNC_UINT64(OnContextVisitProfile, "ContextVisitProfile", profile);
    MESSAGE_FUNC_UINT64(OnSpectateLobbyMember, "ContextSpectate", target);
    MESSAGE_FUNC_UINT64(OnContextReqSavelocs, "ContextReqSavelocs", target);
    MESSAGE_FUNC_UINT64(OnContextMakeOwner, "ContextMakeOwner", target);
    MESSAGE_FUNC_UINT64(OnContextTeleport, "ContextTeleport", target);

private:
    void LobbyEnterSuccess();
    void PopulateLobbyPanel();
    void InitImageList();
    void InitLobbyPanelSections();
    void UpdateLobbyMemberCount() const;
    void SetLobbyTypeImage() const;
    void SetLobbyType() const;

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID);

    CUtlMap<uint64, int> m_mapLobbyIDToImageListIndx;

    vgui::Label *m_pLobbyMemberCount;
    vgui::ImagePanel *m_pLobbyType;
    vgui::IImage *m_pLobbyTypePublic, *m_pLobbyTypeFriends, *m_pLobbyTypePrivate;
    vgui::ListPanel *m_pMemberList;
    vgui::Button *m_pLobbyToggle, *m_pInviteFriends;
    CLeaderboardsContextMenu *m_pContextMenu;
    SavelocReqFrame *m_pSavelocReqFrame;

    vgui::ImageList *m_pImageListLobby;

    IViewPort *m_pViewport;
};
