#include "cbase.h"

#include "LobbyInfoPanel.h"

#include <steam/isteammatchmaking.h>
#include <steam/isteamuser.h>

#include "fmtstr.h"
#include "ilocalize.h"
#include "controls/ContextMenu.h"
#include "controls/FileImage.h"

#include "util/mom_util.h"

#include "vgui/IInput.h"
#include "vgui/ILocalize.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Tooltip.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/TextEntryBox.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static CSteamID s_LobbyID = k_steamIDNil;

static const char *s_pLobbyTypeStrings[3] =
{
    "#MOM_Lobby_Type_Private",
    "#MOM_Lobby_Type_FriendsOnly",
    "#MOM_Lobby_Type_Public"
};

LobbyInfoPanel::LobbyInfoPanel(Panel* pParent): BaseClass(pParent, "LobbyInfoPanel")
{
    SetProportional(true);

    m_bInLobby = false;

    m_pMainStatus = new Label(this, "MainStatusLabel", "#MOM_Drawer_Lobby_Searching");

    m_pLobbyToggleButton = new Button(this, "LobbyToggleButton", "#GameUI2_HostLobby", this, "LobbyToggle");
    m_pInviteUserButton = new Button(this, "InviteUserButton", "Inv", this, "InviteUser");
    m_pInviteUserButton->SetAutoClearImages(false);
    m_pLobbyInviteImage = new FileImage("materials/vgui/icon/lobby_panel/lobby_invite.png");
    m_pInviteUserButton->SetImageAtIndex(0, m_pLobbyInviteImage, 0);

    m_pLobbyType = new ImagePanel(this, "LobbyType");
    m_pLobbyType->InstallMouseHandler(this);

    m_pContextMenu = new ContextMenu(m_pLobbyType);

    LoadControlSettings("resource/ui/mainmenu/LobbyInfoPanel.res");

    m_pLobbyTypeSearching = new FileImage("materials/vgui/icon/lobby_panel/lobby_searching.png");
    m_pLobbyTypePublic = new FileImage("materials/vgui/icon/lobby_panel/lobby_public.png");
    m_pLobbyTypeFriends = new FileImage("materials/vgui/icon/lobby_panel/lobby_friends.png");
    m_pLobbyTypePrivate = new FileImage("materials/vgui/icon/lobby_panel/lobby_private.png");


    m_pLobbyType->SetImage(m_pLobbyTypeSearching);
}

LobbyInfoPanel::~LobbyInfoPanel()
{
    delete m_pLobbyInviteImage;
    delete m_pLobbyTypeSearching;
    delete m_pLobbyTypePublic;
    delete m_pLobbyTypeFriends;
    delete m_pLobbyTypePrivate;
}

void LobbyInfoPanel::OnLobbyEnter(LobbyEnter_t *pParam)
{
    m_bInLobby = true;

    s_LobbyID = CSteamID(pParam->m_ulSteamIDLobby);

    LobbyEnterSuccess();
}

void LobbyInfoPanel::OnLobbyDataUpdate(LobbyDataUpdate_t *pParam)
{
    if (!m_bInLobby)
        return;

    if (pParam->m_ulSteamIDLobby != s_LobbyID.ConvertToUint64())
        return;

    if (pParam->m_ulSteamIDMember == pParam->m_ulSteamIDLobby)
    {
        UpdateLobbyName();
        SetLobbyTypeImage();
    }
}

void LobbyInfoPanel::OnLobbyLeave()
{
    m_bInLobby = false;

    s_LobbyID = k_steamIDNil;

    m_pLobbyToggleButton->SetText("#GameUI2_HostLobby");
    m_pInviteUserButton->SetVisible(false);

    m_pLobbyType->SetImage(m_pLobbyTypeSearching);
    m_pLobbyType->SetMouseInputEnabled(false);

    m_pMainStatus->SetText("#MOM_Drawer_Lobby_Searching");
}

void LobbyInfoPanel::OnCommand(const char* command)
{
    if (FStrEq(command, "LobbyToggle"))
    {
        if (m_bInLobby)
        {
            MomUtil::DispatchConCommand("mom_lobby_leave");
        }
        else
        {
            MomUtil::DispatchConCommand("mom_lobby_create");
        }
    }
    else if (FStrEq(command, "InviteUser"))
    {
        MomUtil::DispatchConCommand("mom_lobby_invite");
    }
    else if (FStrEq(command, "ChangeLobbyLimit"))
    {
        OnChangeLobbyLimit();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void LobbyInfoPanel::OnMousePressed(MouseCode code)
{
    if (input()->GetMouseOver() == m_pLobbyType->GetVPanel())
    {
        if (code == MOUSE_LEFT)
        {
            IncrementLobbyType(); // Cycle and update the lobby type
        }
        else
        {
            ShowLobbyContextMenu();
        }
    }
}

void LobbyInfoPanel::OnReloadControls()
{
    BaseClass::OnReloadControls();

    if (s_LobbyID.IsValid())
    {
        LobbyEnterSuccess();
    }
    else
    {
        OnLobbyLeave();
    }
}

void LobbyInfoPanel::PerformLayout()
{
    BaseClass::PerformLayout();

    m_pLobbyInviteImage->SetSize(GetScaledVal(12), GetScaledVal(12));
}

void LobbyInfoPanel::LobbyEnterSuccess()
{
    m_pLobbyToggleButton->SetText("#GameUI2_LeaveLobby");
    m_pInviteUserButton->SetVisible(true);

    UpdateLobbyName();
    SetLobbyTypeImage();
}

void LobbyInfoPanel::IncrementLobbyType() const
{
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

    if (!s_LobbyID.IsValid())
        return;

    if (SteamUser()->GetSteamID() != SteamMatchmaking()->GetLobbyOwner(s_LobbyID))
        return;

    const auto pTypeStr = SteamMatchmaking()->GetLobbyData(s_LobbyID, LOBBY_DATA_TYPE);
    if (pTypeStr && Q_strlen(pTypeStr) == 1)
    {
        const auto nType = clamp<int>(Q_atoi(pTypeStr), k_ELobbyTypePrivate, k_ELobbyTypePublic);
        const auto newType = (nType + 1) % (k_ELobbyTypePublic + 1);

        SetLobbyType(newType);
    }
}

void LobbyInfoPanel::SetLobbyType(int type) const
{
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

    if (!s_LobbyID.IsValid())
        return;

    if (SteamUser()->GetSteamID() != SteamMatchmaking()->GetLobbyOwner(s_LobbyID))
        return;

    const auto nType = clamp<int>(type, k_ELobbyTypePrivate, k_ELobbyTypePublic);
    engine->ClientCmd_Unrestricted(CFmtStr("mom_lobby_type %i", nType));
}

void LobbyInfoPanel::SetLobbyTypeImage() const
{
    const auto pTypeStr = SteamMatchmaking()->GetLobbyData(s_LobbyID, LOBBY_DATA_TYPE);
    if (pTypeStr && Q_strlen(pTypeStr) == 1)
    {
        const auto nType = clamp<int>(Q_atoi(pTypeStr), k_ELobbyTypePrivate, k_ELobbyTypePublic);

        static IImage *pImages[3] =
        {
            m_pLobbyTypePrivate,
            m_pLobbyTypeFriends,
            m_pLobbyTypePublic
        };

        m_pLobbyType->SetMouseInputEnabled(true);
        m_pLobbyType->SetImage(pImages[nType]);
        m_pLobbyType->GetTooltip()->SetEnabled(true);
        m_pLobbyType->GetTooltip()->SetText(s_pLobbyTypeStrings[nType]);
    }
}

void LobbyInfoPanel::UpdateLobbyName()
{
    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamFriends());

    if (!m_bInLobby)
        return;

    const auto ownerID = SteamMatchmaking()->GetLobbyOwner(s_LobbyID);
    const auto pOwnerName = SteamFriends()->GetFriendPersonaName(ownerID);

    KeyValuesAD data("Data");
    data->SetString("name", pOwnerName);

    m_pMainStatus->SetText(CConstructLocalizedString(g_pVGuiLocalize->FindSafe("#MOM_Drawer_Lobby_Owner"), static_cast<KeyValues*>(data)));
}

void LobbyInfoPanel::OnChangeLobbyType(int type)
{
    SetLobbyType(type);
}

void LobbyInfoPanel::OnChangeLobbyLimitValue(int value)
{
    engine->ClientCmd_Unrestricted(CFmtStr("mom_lobby_max_players %i", value));
}

void LobbyInfoPanel::OnChangeLobbyLimit()
{
    CHECK_STEAM_API(SteamMatchmaking());

    if (!s_LobbyID.IsValid())
        return;

    const auto iMemberLimit = SteamMatchmaking()->GetLobbyMemberLimit(s_LobbyID);

    const auto pEntryBox = new TextEntryBox("#MOM_Lobby_Set_Limit", "#MOM_Lobby_Set_Limit_Label", CFmtStr("%i", iMemberLimit).Get(), false);
    pEntryBox->AddActionSignalTarget(this);
    pEntryBox->SetCommand(new KeyValues("ChangeLobbyLimitValue"));

    pEntryBox->GetTextEntry()->SetAllowNumericInputOnly(true);

    pEntryBox->DoModal();
}

void LobbyInfoPanel::ShowLobbyContextMenu()
{
    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamUser());

    if (!s_LobbyID.IsValid())
        return;

    if (SteamMatchmaking()->GetLobbyOwner(s_LobbyID) != SteamUser()->GetSteamID())
        return;

    m_pContextMenu->OnKillFocus();
    m_pContextMenu->DeleteAllItems();

    const auto pMenuTypes = new Menu(this, "LobbySubmenu_Types");

    for (int i = k_ELobbyTypePrivate; i < k_ELobbyTypeInvisible; i++)
    {
        pMenuTypes->AddMenuItem("LobbyType", s_pLobbyTypeStrings[i], new KeyValues("ChangeLobbyType", "type", i), this);
    }

    m_pContextMenu->AddCascadingMenuItem("LobbyTypes", "#MOM_Lobby_Set_Type", "", this, pMenuTypes);

    m_pContextMenu->AddMenuItem("LobbyMemberLimit", "#MOM_Lobby_Set_Limit", "ChangeLobbyLimit", this);

    m_pContextMenu->ShowMenu();
}