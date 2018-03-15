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
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

MainMenu::MainMenu(Panel *parent) : BaseClass(parent, "MainMenu")
{
    MakePopup(false);
    SetProportional(true);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    AddActionSignalTarget(this);
    SetZPos(0);

    // Set our initial size
    SetBounds(0, 0, GameUI().GetViewport().x, GameUI().GetViewport().y);

    //ivgui()->AddTickSignal(GetVPanel(), 120000); // Tick every 2 minutes
    // First check here
    //g_pMomentumSteamHelper->RequestCurrentTotalPlayers();

    // Listen for game events
    if (gameeventmanager)
    {
        gameeventmanager->AddListener(this, "lobby_leave", false);
        gameeventmanager->AddListener(this, "lobby_join", false);
        gameeventmanager->AddListener(this, "spec_start", false);
        gameeventmanager->AddListener(this, "spec_stop", false);
    }

    if (!GetAnimationController()->SetScriptFile(GetVPanel(), "scripts/HudAnimations.txt"))
        AssertMsg(0, "Couldn't load the animations!");

    HScheme hScheme = scheme()->LoadSchemeFromFile("resource/schememainmenu.res", "SchemeMainMenu");
    SetScheme(hScheme);

    m_bFocused = true;
    m_bNeedSort = false;
    m_nSortFlags = FL_SORT_SHARED;

    GameUI().GetLocalizedString("#GameUI2_LogoLeft", &m_logoLeft);
    GameUI().GetLocalizedString("#GameUI2_LogoRight", &m_logoRight);
    m_pLogoImage = nullptr;

    CreateMenu();

    m_pButtonLobby = new Button_MainMenu(this, this);
    m_pButtonLobby->SetButtonText("#GameUI2_HostLobby");
    m_pButtonLobby->SetButtonDescription("");
    m_pButtonLobby->SetEngineCommand("mom_lobby_create");
    m_pButtonLobby->SetVisible(true);
    m_pButtonLobby->SetTextAlignment(RIGHT);
    m_pButtonLobby->SetButtonType(SHARED);

    m_pButtonInviteFriends = new Button_MainMenu(this, this);
    m_pButtonInviteFriends->SetButtonText("#GameUI2_InviteLobby");
    m_pButtonInviteFriends->SetButtonDescription("");
    m_pButtonInviteFriends->SetEngineCommand("mom_lobby_invite");
    m_pButtonInviteFriends->SetTextAlignment(RIGHT);
    m_pButtonInviteFriends->SetButtonType(SHARED);

    m_pButtonSpectate = new Button_MainMenu(this, this);
    m_pButtonSpectate->SetButtonText("#GameUI2_Spectate");
    m_pButtonSpectate->SetButtonDescription("#GameUI2_SpectateDescription");
    m_pButtonSpectate->SetEngineCommand("mom_spectate");
    m_pButtonSpectate->SetPriority(90);
    m_pButtonSpectate->SetButtonType(IN_GAME);
    m_pButtonSpectate->SetTextAlignment(RIGHT);

    m_pVersionLabel = new Button(this, "VersionLabel", CFmtStr("v%s", MOM_CURRENT_VERSION).Access(), this, "ShowVersion");
    m_pVersionLabel->SetPaintBackgroundEnabled(false);
    m_pVersionLabel->SetAutoWide(true);
    m_pVersionLabel->SetAutoTall(true);
    // MOM_TODO: LoadControlSettings("resource/ui/MainMenuLayout.res");

    CheckVersion();

    MakeReadyForUse();
    RequestFocus();
}

MainMenu::~MainMenu()
{
    ivgui()->RemoveTickSignal(GetVPanel());

    // Stop listening for events
    if (gameeventmanager)
    {
        gameeventmanager->RemoveListener(this);
    }
}


void MainMenu::OnTick()
{
    // We're the only one who should call this! (As we tick once every 2 mins)
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
}

void MainMenu::OnThink()
{
    BaseClass::OnThink();

    surface()->MovePopupToBack(GetVPanel());
}

bool MainMenu::IsVisible(void)
{
    if (GetBasePanel()->IsInLoading())
        return false;

    return BaseClass::IsVisible();
}

void MainMenu::OnMenuButtonCommand(KeyValues* pKv)
{
    const char *pNormalCommand = pKv->GetString("command", nullptr);
    const char *pEngineCommand = pKv->GetString("EngineCommand", nullptr);
    if (pNormalCommand)
    {
        GameUI().SendMainMenuCommand(pNormalCommand);
    }
    else if (pEngineCommand)
    {
        ConCommand* pCommand = g_pCVar->FindCommand(pEngineCommand);
        if (pCommand) // can we directly call this command?
        {
            CCommand blah;
            blah.Tokenize(pEngineCommand);
            pCommand->Dispatch(blah);
        }
        else // fallback to old code
        {
            GetBasePanel()->RunEngineCommand(pEngineCommand);
        }
    }
}

void MainMenu::OnCommand(char const *cmd)
{
    if (!Q_strcmp(cmd, "ShowVersion"))
    {
        GetBasePanel()->RunEngineCommand("mom_show_changelog\n");
        GetAnimationController()->StartAnimationSequence(this, "VersionPulseStop");
    }

     GameUI().SendMainMenuCommand(cmd);
     BaseClass::OnCommand(cmd);
}

void MainMenu::FireGameEvent(IGameEvent* event)
{
    if (!Q_strcmp(event->GetName(), "lobby_leave"))
    {
        m_pButtonLobby->SetButtonText("#GameUI2_HostLobby");
        m_pButtonLobby->SetEngineCommand("mom_lobby_create");
        m_pButtonInviteFriends->SetVisible(false);
        m_pButtonSpectate->SetVisible(false);
    }
    else if (!Q_strcmp(event->GetName(), "lobby_join"))
    {
        m_pButtonLobby->SetButtonText("#GameUI2_LeaveLobby");
        m_pButtonLobby->SetEngineCommand("mom_lobby_leave");
        m_pButtonInviteFriends->SetVisible(true);
        m_pButtonSpectate->SetVisible(true);
    }
    else if (!Q_strcmp(event->GetName(), "spec_start"))
    {
        m_pButtonSpectate->SetButtonText("#GameUI2_Respawn");
        m_pButtonSpectate->SetEngineCommand("mom_spectate_stop");
    }
    else if (!Q_strcmp(event->GetName(), "spec_stop"))
    {
        m_pButtonSpectate->SetButtonText("#GameUI2_Spectate");
        m_pButtonSpectate->SetButtonDescription("#GameUI2_SpectateDescription");
        m_pButtonSpectate->SetEngineCommand("mom_spectate");
    }
}

void MainMenu::CreateMenu()
{
    m_pButtons.PurgeAndDeleteElements();

    KeyValues *datafile = new KeyValues("MainMenu");
    datafile->UsesEscapeSequences(true);
    if (datafile->LoadFromFile(g_pFullFileSystem, "resource/mainmenu.res", "GAME"))
    {
        FOR_EACH_SUBKEY(datafile, dat)
        {
            Button_MainMenu *button = new Button_MainMenu(this, this, "");
            button->SetPriority(dat->GetInt("priority", 1));
            button->SetButtonText(dat->GetString("text", "no text"));
            button->SetButtonDescription(dat->GetString("description", "no description"));
            button->SetBlank(dat->GetBool("blank"));

            const char *pCommand = dat->GetString("command", nullptr);
            if (pCommand)
            {
                button->SetCommand(pCommand);
            }
            else
            {
                pCommand = dat->GetString("EngineCommand", nullptr);
                if (pCommand)
                    button->SetEngineCommand(pCommand);
            }

            const char *specifics = dat->GetString("specifics", "shared");
            if (!Q_strcasecmp(specifics, "ingame"))
                button->SetButtonType(IN_GAME);
            else if (!Q_strcasecmp(specifics, "mainmenu"))
                button->SetButtonType(MAIN_MENU);

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

#define SC(val) scheme()->GetProportionalScaledValueEx(GetScheme(), val)

void MainMenu::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_iButtonsSpace = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.Space")));
    m_iButtonsOffsetX = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.OffsetX")));
    m_iButtonsOffsetY = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.OffsetY")));
    m_iLogoOffsetX = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.OffsetX")));
    m_iLogoOffsetY = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.OffsetY")));
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

        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        m_iLogoWidth = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width")));
        m_iLogoHeight = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height")));
        // Size and pos are handled in Paint()
    }
    m_bLogoPlayerCount = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.PlayerCount"));
    m_fLogoPlayerCount = pScheme->GetFont("MainMenu.Logo.PlayerCount.Font", true);
    m_cLogoPlayerCount = pScheme->GetColor("MainMenu.Logo.PlayerCount.Color", Color(255, 255, 255, 255));

    m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    m_hFontVersionLabel = pScheme->GetFont("MainMenu.VersionLabel.Font", true);
    if (m_pVersionLabel)
    {
        m_pVersionLabel->SetFont(m_hFontVersionLabel);
        m_pVersionLabel->InvalidateLayout(true, true);
    }

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
    else if (GameUI().IsInMenu())
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
            m_pButtons[i]->SetPos(m_iButtonsOffsetX, y0 - (m_pButtons[i]->GetHeight() + m_iButtonsSpace));
        }
        else
        {
            m_pButtons[i]->SetPos(m_iButtonsOffsetX,
                GameUI().GetViewport().y - (m_iButtonsOffsetY + m_pButtons[i]->GetHeight()));
        }
    }

    const Vector2D vp = GameUI().GetViewport();

    m_pButtonLobby->SetPos(vp.x - m_pButtonLobby->GetWidth() - m_iButtonsOffsetX, 
        vp.y - m_pButtonLobby->GetTall() - m_iButtonsOffsetY);

    m_pButtonInviteFriends->SetPos(vp.x - m_pButtonInviteFriends->GetWidth() - m_iButtonsOffsetX, 
        m_pButtonLobby->GetYPos() - m_pButtonInviteFriends->GetTall() - m_iButtonsSpace);

    m_pButtonSpectate->SetPos(vp.x - m_pButtonSpectate->GetWidth() - m_iButtonsOffsetX, 
        m_pButtonInviteFriends->GetYPos() - m_pButtonSpectate->GetTall() - m_iButtonsSpace);

    m_pVersionLabel->SetPos(vp.x - m_pVersionLabel->GetWide() - 4, 2);
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
            logoX = m_iLogoOffsetX;
            logoY = m_iLogoOffsetY;
        }
        else
        {
            int32 x0, y0;
            m_pButtons[0]->GetPos(x0, y0);
            logoX = m_iButtonsOffsetX + m_iLogoOffsetX;
            logoY = y0 - (logoH + m_iLogoOffsetY);
        }
        surface()->DrawSetTextPos(logoX, logoY);
        surface()->DrawPrintText(m_logoLeft, wcslen(m_logoLeft));

        surface()->DrawSetTextColor(m_cLogoRight);
        surface()->DrawSetTextFont(m_fLogoFont);
        surface()->DrawSetTextPos(logoX + logoW, logoY);
        surface()->DrawPrintText(m_logoRight, wcslen(m_logoRight));
    }
    // This is a logo, but let's make sure it's still valid
    else if (m_pLogoImage)
    {
        int logoX, logoY;
        if (m_pButtons.Count() <= 0 || m_bLogoAttachToMenu == false)
        {
            logoX = m_iLogoOffsetX;
            logoY = m_iLogoOffsetY;
        }
        else
        {
            int32 x0, y0;
            m_pButtons[0]->GetPos(x0, y0);
            logoX = m_iButtonsOffsetX + m_iLogoOffsetX;
            int imageHeight, dummy;
            m_pLogoImage->GetImage()->GetSize(dummy, imageHeight);
            logoY = y0 - (imageHeight + m_iLogoOffsetY);
        }

        m_pLogoImage->SetPos(logoX, logoY);

        m_pLogoImage->SetSize(m_iLogoWidth, m_iLogoHeight);

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

void MainMenu::CheckVersion()
{
    KeyValues* pVersionKV = new KeyValues("Version");
    KeyValuesAD autoDelete(pVersionKV);
    // Not if-checked here since it doesn't really matter if we don't load it
    pVersionKV->LoadFromFile(g_pFullFileSystem, "version.txt", "MOD");
    if (V_strcmp(pVersionKV->GetString("num", MOM_CURRENT_VERSION), MOM_CURRENT_VERSION) < 0)
    {
        // New version! Make the version in the top right pulse for effect!
        GetAnimationController()->StartAnimationSequence(this, "VersionPulse");
    }

    // Set the current version either way 
    pVersionKV->SetString("num", MOM_CURRENT_VERSION);

    // Save this file either way
    pVersionKV->SaveToFile(g_pFullFileSystem, "version.txt", "MOD");
}

void MainMenu::Paint()
{
    BaseClass::Paint();

    DrawMainMenu();
    DrawLogo();
}