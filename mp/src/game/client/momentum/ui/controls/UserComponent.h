#pragma once
#include "vgui_controls/EditablePanel.h"
#include <steam/isteamfriends.h>

class CAvatarImagePanel;
struct UserData;

class UserComponent : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(UserComponent, EditablePanel);
    UserComponent(Panel *pParent, const char *pName = "UserComponent");
    ~UserComponent();

    void SetClickable(bool bClickable);

    void SetUser(uint64 uID);

    void FillControlsWithUserData(const UserData &data);

    MESSAGE_FUNC(OnUserDataUpdate, "UserDataUpdate");

protected:
    void OnReloadControls() override;
    void PerformLayout() override;
    void ApplySchemeSettings(vgui::IScheme* pScheme) override;
    void OnMouseReleased(vgui::MouseCode code) override;

    STEAM_CALLBACK(UserComponent, OnPersonaStateChange, PersonaStateChange_t);

private:
    CAvatarImagePanel *m_pUserImage;
    vgui::Label *m_pUserName, *m_pUserRank;
    vgui::ProgressBar *m_pXPProgressBar;

    bool m_bClickable;

    uint64 m_uSteamID;
};