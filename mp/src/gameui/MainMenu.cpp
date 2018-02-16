#include "BasePanel.h"
#include "GameUI_Interface.h"
#include "MainMenu.h"

#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui/IInput.h"

#include "filesystem.h"
#include "KeyValues.h"

#include "mom_steam_helper.h"
#include "mom_shareddefs.h"
#include "util/jsontokv.h"
#include "fmtstr.h"

#include "vgui_controls/ImagePanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

MainMenu::MainMenu(Panel *parent) : BaseClass(parent, "MainMenu")
{
    SetProportional(true);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    AddActionSignalTarget(this);
    MakePopup(false);
    SetZPos(1);

    // Set our initial size
    SetBounds(0, 0, GameUI().GetViewport().x, GameUI().GetViewport().y);

    m_bInGame = false;

    ivgui()->AddTickSignal(GetVPanel(), 120000); // Tick every 2 minutes
    // First check here
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();

    // Listen for game events
    if (gameeventmanager)
    {
        gameeventmanager->AddListener(this, "lobby_leave", false);
    }

    HScheme hScheme = scheme()->LoadSchemeFromFile("resource/schememainmenu.res", "SchemeMainMenu");
    SetScheme(hScheme);


    m_bFocused = true;
    m_bNeedSort = false;
    m_nSortFlags = FL_SORT_SHARED;

    //m_logoLeft = GameUI2().GetLocalizedString("#GameUI2_LogoLeft");
    //m_logoRight = GameUI2().GetLocalizedString("#GameUI2_LogoRight");
    m_pLogoImage = nullptr;

    m_bInLobby = false;
    m_bIsSpectating = false;

    m_pButtonLobby = new Button_MainMenu(this, this, "engine mom_lobby_create");
    m_pButtonLobby->SetButtonText("#GameUI2_HostLobby");
    m_pButtonLobby->SetButtonDescription("");
    m_pButtonLobby->SetPriority(1);
    m_pButtonLobby->SetBlank(false);
    m_pButtonLobby->SetVisible(true);
    m_pButtonLobby->SetTextAlignment(CENTER);
    m_pButtonLobby->SetButtonType(SHARED);

    m_pButtonInviteFriends = new Button_MainMenu(this, this, "engine mom_lobby_invite");
    m_pButtonInviteFriends->SetButtonText("#GameUI2_InviteFriends");
    m_pButtonInviteFriends->SetButtonDescription("");
    m_pButtonInviteFriends->SetPriority(1);
    m_pButtonInviteFriends->SetBlank(false);
    m_pButtonInviteFriends->SetVisible(false);
    m_pButtonInviteFriends->SetTextAlignment(CENTER);
    m_pButtonInviteFriends->SetButtonType(SHARED);

    m_pVersionLabel = new Label(this, "VersionLabel", CFmtStr("Version %s", MOM_CURRENT_VERSION));
    m_pVersionLabel->SetAutoWide(true);
    // MOM_TODO: finish implementing this

    CreateMenu();

    // MOM_TODO: LoadControlSettings("resource/ui/MainMenuLayout.res");

    CheckVersion();

    MakeReadyForUse();
    RequestFocus();
}

MainMenu::~MainMenu()
{
    ivgui()->RemoveTickSignal(GetVPanel());
}

void MainMenu::OnThink()
{
    BaseClass::OnThink();
    
    VPANEL over = input()->GetMouseOver();
    if (over != GetVPanel())
        OnKillFocus();
    else
        OnSetFocus();

    // Needed so the menu doesn't cover any other engine panels. Blame the closed-source Surface code
    // for moving the menu to the front when it gains focus, automatically. *sigh*
    surface()->MovePopupToBack(GetVPanel());

    if (m_bInLobby != g_pMomentumSteamHelper->IsLobbyValid())
    {
        m_bInLobby = !m_bInLobby;
        //SendLobbyUpdateCommand();
    }

    if (m_bInGame != GameUI().IsInLevel())
    {
        m_bInGame = !m_bInGame;
        //SendGameStatusCommand();
    }
}


void MainMenu::OnTick()
{
    // We're the only one who should call this! (As we tick once every 2 mins)
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
}

bool MainMenu::IsVisible(void)
{
    if (GetBasePanel()->IsInLoading())
        return false;

    return BaseClass::IsVisible();
}

void MainMenu::OnCommand(char const *cmd)
{
     GameUI().SendMainMenuCommand(cmd);
     BaseClass::OnCommand(cmd);
}

void MainMenu::FireGameEvent(IGameEvent* event)
{
    if (!Q_strcmp(event->GetName(), "lobby_leave"))
    {
        m_bInLobby = false;
        //SendLobbyUpdateCommand();
    }
}

void MainMenu::CreateMenu()
{
    m_pButtons.PurgeAndDeleteElements();

    KeyValues *datafile = new KeyValues("MainMenu");
    datafile->UsesEscapeSequences(true);
    if (datafile->LoadFromFile(g_pFullFileSystem, "resource/mainmenu.res"))
    {
        FOR_EACH_SUBKEY(datafile, dat)
        {
            Button_MainMenu *button = new Button_MainMenu(this, this, dat->GetString("command", ""));
            button->SetPriority(dat->GetInt("priority", 1));
            button->SetButtonText(dat->GetString("text", "no text"));
            button->SetButtonDescription(dat->GetString("description", "no description"));
            button->SetBlank(dat->GetBool("blank"));

            const char *specifics = dat->GetString("specifics", "shared");
            if (!Q_strcasecmp(specifics, "ingame"))
                button->SetButtonType(IN_GAME);
            else if (!Q_strcasecmp(specifics, "mainmenu"))
                button->SetButtonType(MAIN_MENU);
            // Save a pointer to this button if it's the spectate one
            if (Q_strcmp(dat->GetName(), "Spectate") == 0)
            {
                m_pButtonSpectate = button;
            }
            m_pButtons.AddToTail(button);
        }
    }

    datafile->deleteThis();
}

int32 __cdecl ButtonsPositionBottom(Button_MainMenu *const *s1, Button_MainMenu *const *s2)
{
    return ((*s1)->GetPriority() > (*s2)->GetPriority());
}

int32 __cdecl ButtonsPositionTop(Button_MainMenu *const *s1, Button_MainMenu *const *s2)
{
    return ((*s1)->GetPriority() < (*s2)->GetPriority());
}

void MainMenu::ApplySchemeSettings(IScheme *pScheme)
{
    // Find a better place for this
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
    BaseClass::ApplySchemeSettings(pScheme);

    m_fButtonsSpace = Q_atof(pScheme->GetResourceString("MainMenu.Buttons.Space"));
    m_fButtonsOffsetX = Q_atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetX"));
    m_fButtonsOffsetY = Q_atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetY"));
    m_fLogoOffsetX = Q_atof(pScheme->GetResourceString("MainMenu.Logo.OffsetX"));
    m_fLogoOffsetY = Q_atof(pScheme->GetResourceString("MainMenu.Logo.OffsetY"));
    m_cLogoLeft = GetSchemeColor("MainMenu.Logo.Left", pScheme);
    m_cLogoRight = GetSchemeColor("MainMenu.Logo.Right", pScheme);
    m_bLogoAttachToMenu = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.AttachToMenu"));
    m_bLogoText = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Text"));
    if (!m_bLogoText)
    {
        if (m_pLogoImage)
            m_pLogoImage->EvictImage();
        else
            m_pLogoImage = new ImagePanel(this, "GameLogo");

        m_pLogoImage->SetAutoDelete(true);
        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        int imageW = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width"));
        int imageH = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height"));
        m_pLogoImage->SetSize(imageW, imageH);
        // Pos is handled in Paint()
    }
    m_bLogoPlayerCount = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.PlayerCount"));
    m_fLogoPlayerCount = pScheme->GetFont("MainMenu.Logo.PlayerCount.Font", true);
    m_cLogoPlayerCount = pScheme->GetColor("MainMenu.Logo.PlayerCount.Color", Color(255, 255, 255, 255));

    m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    Q_strncpy(m_pszMenuOpenSound, pScheme->GetResourceString("MainMenu.Sound.Open"), sizeof(m_pszMenuOpenSound));
    Q_strncpy(m_pszMenuCloseSound, pScheme->GetResourceString("MainMenu.Sound.Close"), sizeof(m_pszMenuCloseSound));
}

inline Button_MainMenu *GetNextVisible(CUtlVector<Button_MainMenu *> *vec, int start)
{
    for (int i = start + 1; i < vec->Count(); i++)
    {
        Button_MainMenu *pButton = vec->Element(i);
        if (pButton->IsVisible())
            return pButton;
    }
    return nullptr;
}

void MainMenu::DrawMainMenu()
{
    for (int8 i = 0; i < m_pButtons.Count(); i++)
    {
        Button_MainMenu *pButton = m_pButtons[i];
        switch (pButton->GetButtonType())
        {
        default:
        case SHARED:
            pButton->SetVisible(GameUI().IsInLevel() || GameUI().IsInMenu());
            break;
        case IN_GAME:
            pButton->SetVisible(GameUI().IsInLevel());
            break;
        case MAIN_MENU:
            pButton->SetVisible(GameUI().IsInMenu());
            break;
        }
    }

    if (GameUI().IsInLevel())
    {
        m_nSortFlags &= ~FL_SORT_MENU;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_INGAME));
        m_nSortFlags |= FL_SORT_INGAME;
    }
    else if (GameUI().IsInBackgroundLevel())
    {
        m_nSortFlags &= ~FL_SORT_INGAME;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_MENU));
        m_nSortFlags |= FL_SORT_MENU;
    }

    if (m_pButtons.Count() > 0 && m_bNeedSort)
    {
        m_bNeedSort = false;
        m_pButtons.Sort(ButtonsPositionTop);
    }

    for (int8 i = 0; i < m_pButtons.Count(); i++)
    {
        Button_MainMenu *pNextVisible = GetNextVisible(&m_pButtons, i);
        if (pNextVisible)
        {
            int32 x0, y0;
            pNextVisible->GetPos(x0, y0);
            m_pButtons[i]->SetPos(m_fButtonsOffsetX, y0 - (m_pButtons[i]->GetHeight() + m_fButtonsSpace));
        }
        else
        {
            m_pButtons[i]->SetPos(m_fButtonsOffsetX,
                GameUI().GetViewport().y - (m_fButtonsOffsetY + m_pButtons[i]->GetHeight()));
        }
    }


    // New lobby state.
    const bool isLobbyValid = g_pMomentumSteamHelper->IsLobbyValid();

    if (isLobbyValid && !m_bInLobby) // We just joined a lobby!
    {
        m_pButtonLobby->SetButtonText("#GameUI2_LeaveLobby");
        m_pButtonLobby->SetCommand("engine mom_lobby_leave");
        m_pButtonInviteFriends->SetVisible(true);
        m_bInLobby = isLobbyValid;
    }
    else if (!isLobbyValid && m_bInLobby) // We left a lobby
    {
        m_pButtonLobby->SetButtonText("GameUI2_HostLobby");
        m_pButtonLobby->SetCommand("engine mom_lobby_create");
        m_pButtonInviteFriends->SetVisible(false);
        m_bInLobby = isLobbyValid;
    }

    const char* spectatingText = g_pMomentumSteamHelper->GetLobbyLocalMemberData(LOBBY_DATA_IS_SPEC);
    const bool isSpectating = spectatingText != nullptr && Q_strlen(spectatingText) > 0;
    if (isSpectating && !m_bIsSpectating) // We just started spectating
    {
        m_pButtonSpectate->SetButtonText("#GameUI2_Respawn");
        m_pButtonSpectate->SetButtonDescription("#GameUI2_RespawnDescription");
        m_pButtonSpectate->SetCommand("engine mom_spectate_stop");
        m_bIsSpectating = isSpectating;
    }
    else if (!isSpectating && m_bIsSpectating) // We respawned
    {
        m_pButtonSpectate->SetButtonText("#GameUI2_Spectate");
        m_pButtonSpectate->SetButtonDescription("#GameUI2_SpectateDescription");
        m_pButtonSpectate->SetCommand("engine mom_spectate");
        m_bIsSpectating = isSpectating;
    }
    m_pButtonSpectate->SetVisible(isLobbyValid);
    m_pButtonLobby->SetPos(GameUI().GetViewport().x - m_pButtonLobby->GetWidth(), m_fButtonsOffsetY);
    m_pButtonInviteFriends->SetPos(GameUI().GetViewport().x - m_pButtonInviteFriends->GetWidth(), m_pButtonLobby->GetTall() + m_fButtonsOffsetY);

}

void MainMenu::DrawLogo()
{
    if (m_bLogoText)
    {
        surface()->DrawSetTextColor(m_cLogoLeft);
        surface()->DrawSetTextFont(m_fLogoFont);

        int32 logoW, logoH;
        surface()->GetTextSize(m_fLogoFont, m_logoLeft, logoW, logoH);

        int32 logoX, logoY;
        if (m_pButtons.Count() <= 0 || m_bLogoAttachToMenu == false)
        {
            logoX = m_fLogoOffsetX;
            logoY = m_fLogoOffsetY;
        }
        else
        {
            int32 x0, y0;
            m_pButtons[0]->GetPos(x0, y0);
            logoX = m_fButtonsOffsetX + m_fLogoOffsetX;
            logoY = y0 - (logoH + m_fLogoOffsetY);
        }
        surface()->DrawSetTextPos(logoX, logoY);
        surface()->DrawPrintText(m_logoLeft, wcslen(m_logoLeft));

        surface()->DrawSetTextColor(m_cLogoRight);
        surface()->DrawSetTextFont(m_fLogoFont);
        surface()->DrawSetTextPos(logoX + logoW, logoY);
        surface()->DrawPrintText(m_logoRight, wcslen(m_logoRight));
    }
    else
    {
        // This is a logo, but let's make sure it's still valid
        if (m_pLogoImage)
        {
            int logoX, logoY;
            if (m_pButtons.Count() <= 0 || m_bLogoAttachToMenu == false)
            {
                logoX = m_fLogoOffsetX;
                logoY = m_fLogoOffsetY;
            }
            else
            {
                int32 x0, y0;
                m_pButtons[0]->GetPos(x0, y0);
                logoX = m_fButtonsOffsetX + m_fLogoOffsetX;
                int imageHeight, dummy;
                m_pLogoImage->GetImage()->GetSize(dummy, imageHeight);
                logoY = y0 - (imageHeight + m_fLogoOffsetY);
            }

            m_pLogoImage->SetPos(logoX, logoY);
            /*if (m_bLogoPlayerCount)
            {
                surface()->DrawSetTextColor(m_cLogoPlayerCount);
                surface()->DrawSetTextFont(m_fLogoPlayerCount);
                surface()->DrawSetTextPos(m_pLogoImage->GetXPos(), m_pLogoImage->GetTall() + m_pLogoImage->GetYPos());
                const wchar_t* currentTotalPlayers = g_pMomentumSteamHelper->GetCurrentTotalPlayersAsString();
                surface()->DrawPrintText(currentTotalPlayers, wcslen(currentTotalPlayers));
            }*/
        }
    }
}

void MainMenu::CheckVersion()
{
    bool bNewVersion = false;

    KeyValues* pVersionKV = new KeyValues("Version");
    KeyValuesAD autoDelete(pVersionKV);
    // Not if-checked here since it doesn't really matter if we don't load it
    pVersionKV->LoadFromFile(g_pFullFileSystem, "version.txt", "MOD");
    if (V_strcmp(pVersionKV->GetString("num", MOM_CURRENT_VERSION), MOM_CURRENT_VERSION) < 0)
    {
        // New version! Make the version in the top right pulse for effect!
        bNewVersion = true;
        // MOM_TODO: Make the version label flash/flicker/etc here
    }

    // Set the current version either way 
    pVersionKV->SetString("num", MOM_CURRENT_VERSION);

    // Save this file either way
    pVersionKV->SaveToFile(g_pFullFileSystem, "version.txt", "MOD");

/*    char command[128];
    Q_snprintf(command, 128, "setVersion('%s', %s)", MOM_CURRENT_VERSION, bNewVersion ? "true" : "false");
    RunJavascript(command);*/
}

void MainMenu::Paint()
{
    BaseClass::Paint();

    DrawMainMenu();
    DrawLogo();
}

void MainMenu::OnScreenSizeChanged(int oldwide, int oldtall)
{
    if (ipanel())
        SetBounds(0, 0, GameUI().GetViewport().x, GameUI().GetViewport().y);
}
