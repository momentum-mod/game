#pragma once

#include "vgui_controls/ListPanel.h"

struct LobbyDataUpdate_t;
struct LobbyEnter_t;
class LobbyListProvider;

class LobbySearchPanel : public vgui::ListPanel
{
public:
    DECLARE_CLASS_SIMPLE(LobbySearchPanel, ListPanel);

    LobbySearchPanel(Panel *pParent, bool bSearchingFriends);
    ~LobbySearchPanel();

    void SearchForLobbies();
    void CancelSearch();

    void OnLobbyEnter(LobbyEnter_t *pEnter);
    void OnLobbyLeave();
    void OnLobbyDataUpdate(LobbyDataUpdate_t *pUpdate);

    void AddLobby(const CSteamID &hLobbyID);
    bool UpdateLobby(const CSteamID &hLobbyID, KeyValues *pLobbyData);
    void RemoveLobby(const CSteamID &hLobbyID);

protected:
    void OnKeyCodeTyped(vgui::KeyCode code) override;
    void OnThink() override;

private:
    bool LobbyTypeIsValid(const CSteamID &hLobbyID);

    void RefreshLobbyData();

    void InitImageList();

    int TryAddAvatar(uint64 uID);

    bool m_bSearchingFriends;

    CUtlMap<uint64, int> m_mapLobbyIDToListPanelIndex;
    CUtlMap<uint64, int> m_mapLobbyIDToImageListIndex;

    vgui::ImageList *m_pAvatarImages;

    friend class LobbyListProvider;
    LobbyListProvider *m_pProvider;
};