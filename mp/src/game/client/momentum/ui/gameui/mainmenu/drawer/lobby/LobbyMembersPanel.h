#pragma once

#include "vgui_controls/EditablePanel.h"

struct LobbyChatUpdate_t;
struct LobbyDataUpdate_t;
struct LobbyEnter_t;

namespace vgui
{
    class ListPanelItem;
}

class ContextMenu;
class SavelocReqFrame;

class LobbyMembersPanel : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(LobbyMembersPanel, vgui::EditablePanel);

    LobbyMembersPanel(Panel *pParent);
    ~LobbyMembersPanel();

    void OnLobbyEnter(LobbyEnter_t *pData);
    void OnLobbyLeave();
    void OnLobbyDataUpdate(LobbyDataUpdate_t *pData);
    void OnLobbyChatUpdate(LobbyChatUpdate_t *pData);

    void AddLobbyMember(const CSteamID &steamID); // Adds a lobby member to the panel
    void UpdateLobbyMemberData(const CSteamID &memberID); // Updates the lobby member's status data on the panel

    static int StaticLobbyMemberSortFunc(vgui::ListPanel *list, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2);

    // finds a player in the lobby data panel
    int FindItemIDForLobbyMember(uint64 steamID);
    int FindItemIDForLobbyMember(const CSteamID& id);
protected:

    void OnReloadControls() override;
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

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID);

    CUtlMap<uint64, int> m_mapLobbyIDToImageListIndx;

    vgui::ListPanel *m_pMemberList;
    ContextMenu *m_pContextMenu;
    SavelocReqFrame *m_pSavelocReqFrame;

    vgui::ImageList *m_pImageListLobby;
    vgui::IImage *m_pLobbyOwnerImage;
};
