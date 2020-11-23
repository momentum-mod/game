#pragma once

#include "vgui_controls/PropertyPage.h"

#include "steam/isteammatchmaking.h"
#include <steam/isteamfriends.h>

class LobbyMembersPanel;
class LobbyInfoPanel;
class ChatContainer;
class LobbySearchPanel;

class DrawerPanel_Lobby : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(DrawerPanel_Lobby, PropertyPage);

    DrawerPanel_Lobby(Panel *pParent);

    STEAM_CALLBACK(DrawerPanel_Lobby, OnLobbyEnter, LobbyEnter_t); // When we enter a lobby
    STEAM_CALLBACK(DrawerPanel_Lobby, OnLobbyDataUpdate, LobbyDataUpdate_t); // People/lobby updates status
    STEAM_CALLBACK(DrawerPanel_Lobby, OnLobbyChatUpdate, LobbyChatUpdate_t); // People join/leave
    STEAM_CALLBACK(DrawerPanel_Lobby, OnPersonaStateChange, PersonaStateChange_t); // People change name

    void OnLobbyLeave(); // No dedicated steam callback, thanks valve

protected:
    // This is where we actually need to initialize our children
    void OnResetData() override;
    void OnReloadControls() override;

private:
    void OnLobbyStateChange(); // Entered/exited lobby, update our panels

    bool m_bInLobby;

    LobbyInfoPanel *m_pLobbyInfo;

    // When not in lobby
    vgui::PropertySheet *m_pPublicLobbies;
    LobbySearchPanel *m_pPublicLobbiesSearch;
    LobbySearchPanel *m_pFriendsLobbiesSearch;

    // When in lobby
    LobbyMembersPanel *m_pLobbyMembers;
    ChatContainer *m_pLobbyChat;
};