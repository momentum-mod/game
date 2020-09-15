#pragma once

#include "vgui_controls/EditablePanel.h"

struct LobbyEnter_t;
struct LobbyChatUpdate_t;
struct LobbyDataUpdate_t;
class ContextMenu;

class LobbyInfoPanel : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(LobbyInfoPanel, EditablePanel);

    LobbyInfoPanel(Panel* pParent);
    ~LobbyInfoPanel();

    void OnLobbyEnter(LobbyEnter_t *pParam);
    void OnLobbyLeave();

    void OnLobbyDataUpdate(LobbyDataUpdate_t *pData);

protected:
    void OnCommand(const char* command) override;
    void OnMousePressed(vgui::MouseCode code) override;
    void OnReloadControls() override;
    void PerformLayout() override;

    MESSAGE_FUNC_INT(OnChangeLobbyType, "ChangeLobbyType", type);
    MESSAGE_FUNC_INT(OnChangeLobbyLimitValue, "ChangeLobbyLimitValue", value);

private:
    void LobbyEnterSuccess();
    void SetLobbyTypeImage() const;
    void UpdateLobbyName();
    void IncrementLobbyType() const;
    void SetLobbyType(int type) const;
    void OnChangeLobbyLimit();

    void ShowLobbyContextMenu();

    bool m_bInLobby;

    ContextMenu *m_pContextMenu;

    vgui::ImagePanel *m_pLobbyType;
    vgui::IImage *m_pLobbyTypeSearching, *m_pLobbyInviteImage;
    vgui::IImage *m_pLobbyTypePublic, *m_pLobbyTypeFriends, *m_pLobbyTypePrivate;

    vgui::Label *m_pMainStatus;

    vgui::Button *m_pLobbyToggleButton, *m_pInviteUserButton;
};