#include "basepanel.h"
#include "gameui2_interface.h"
#include "mainmenu.h"

#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui_controls/ImagePanel.h"

#include "KeyValues.h"
#include "filesystem.h"

#include "mom_steam_helper.h"
#include "mom_shareddefs.h"
#include "vgui_controls/HTML.h"
#include "util/jsontokv.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define USE_OLD_MENU 0 // MOM_TODO: Remove this and anything that relies on it being 1

using namespace vgui;

class MomentumURLResolver : public Panel
{
    DECLARE_CLASS_SIMPLE(MomentumURLResolver, Panel);
    MomentumURLResolver(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
    {
        SetAutoDelete(true);
        SetVisible(false);
        SetEnabled(false);
    }

protected:
    MESSAGE_FUNC_CHARPTR_CHARPTR(OnCustomURL, "CustomURL", schema, URL)
    {
        DevLog("Going to custom URL %s\n", URL);
        const char* pFile = strstr(URL, "://") + 3; //+3 so we skip past the ://
        DevLog("Finding file %s...\n", pFile);
        char path[MAX_PATH];
        V_snprintf(path, MAX_PATH, "resource/html/%s.html", pFile);
        char fullPath[1024];
        g_pFullFileSystem->RelativePathToFullPath(path, "MOD", fullPath, 1024);
        char finalPath[1024];
        V_snprintf(finalPath, 1024, "file:///%s", fullPath);

        for (int i = 0; i < V_strlen(finalPath); i++) 
        {
            if (finalPath[i] == '\\') finalPath[i] = '/';
        }
        DevLog("Full file URL path: %s\n", finalPath);
        PostActionSignal(new KeyValues("GoToURL", "url", finalPath));
    } 
};

class MainMenuHTML : public vgui::HTML
{
    DECLARE_CLASS_SIMPLE(MainMenuHTML, vgui::HTML);

    MainMenuHTML(Panel *pParent, const char *pName) : BaseClass(pParent, pName, true)
    {
        AddActionSignalTarget(pParent);
        m_pURLResolver = new MomentumURLResolver(this, "MomentumURLResolver");
        m_pURLResolver->AddActionSignalTarget(this);
        AddCustomURLHandler("mom://", m_pURLResolver);
        SetPaintBackgroundEnabled(false);
    }
    ~MainMenuHTML()
    {
    }

    void LoadMenu()
    {
        OpenURL("mom://menu");
    }

    void OpenURL(const char *pURL)
    {
        BaseClass::OpenURL(pURL, nullptr);
    }
    MESSAGE_FUNC_CHARPTR(OnURLResolved, "GoToURL", url)
    {
        OpenURL(url);
    }
    void OnFinishRequest(const char* url, const char* pageTitle, const CUtlMap<CUtlString, CUtlString>& headers) OVERRIDE 
    {
        char command[128];
        const char *pLanguage = m_SteamAPIContext.SteamApps()->GetCurrentGameLanguage();
        Q_snprintf(command, 128, "setLocalization('%s')", pLanguage);
        RunJavascript(command);
        SendVolumeCommand(ConVarRef("volume").GetFloat()); // The initial volume to set
    }

    void SendVolumeCommand(float flVolume)
    {
        char command[128];
        Q_snprintf(command, 128, "setVolume(%.3f)", flVolume);
        RunJavascript(command);
    }

    void OnJSAlert(HTML_JSAlert_t* pAlert) OVERRIDE
    {
        KeyValues *pKv = CJsonToKeyValues::ConvertJsonToKeyValues(pAlert->pchMessage);
        KeyValuesAD autodelete(pKv);

        if (pKv)
        {
            if (!Q_strcmp(pKv->GetString("id"), "menu"))
            {
                if (pKv->GetBool("special"))
                {
                    GameUI2().GetGameUI()->SendMainMenuCommand(pKv->GetString("com"));
                }
                else
                {
                    GameUI2().GetEngineClient()->ClientCmd_Unrestricted(pKv->GetString("com"));
                }
            }
            else if (!Q_strcmp(pKv->GetString("id"), "echo"))
            {
                DevLog("%s\n", pKv->GetString("com"));
            }
        }

        // This must be called!
        DismissJSDialog(true);
    }

    void OnMousePressed(MouseCode mc) OVERRIDE
    {
        if (mc != MOUSE_RIGHT)
        {
            BaseClass::OnMousePressed(mc);
        }
    }
private:
    MomentumURLResolver *m_pURLResolver;
};



MainMenu::MainMenu(Panel *parent) : BaseClass(parent, "MainMenu"), volumeRef("volume")
{
    HScheme Scheme = scheme()->LoadSchemeFromFile("resource2/schememainmenu.res", "SchemeMainMenu");
    SetScheme(Scheme);

    SetProportional(true);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetAutoDelete(true);
    AddActionSignalTarget(this);

    SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);

    m_bFocused = true;
    m_bNeedSort = false;
    m_bInGame = false;
    m_fGameVolume = volumeRef.GetFloat();
    m_nSortFlags = FL_SORT_SHARED;

    m_logoLeft = GameUI2().GetLocalizedString("#GameUI2_LogoLeft");
    m_logoRight = GameUI2().GetLocalizedString("#GameUI2_LogoRight");
    m_pLogoImage = nullptr;
#if USE_OLD_MENU
    m_pButtonFeedback = new Button_MainMenu(this, this, "engine mom_contact_show");
    m_pButtonFeedback->SetButtonText("#GameUI2_SendFeedback");
    m_pButtonFeedback->SetButtonDescription("#GameUI2_SendFeedbackDescription");
    m_pButtonFeedback->SetPriority(1);
    m_pButtonFeedback->SetBlank(false);
    m_pButtonFeedback->SetVisible(true);
    m_pButtonFeedback->SetTextAlignment(RIGHT);

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
    

    CreateMenu("resource2/mainmenu.res");
#else
    m_pMainMenuHTMLPanel = new MainMenuHTML(this, "CMainMenuHTML");
    int wide, high;
    GetSize(wide, high);
    m_pMainMenuHTMLPanel->SetSize(wide, high);
    m_pMainMenuHTMLPanel->LoadMenu();
#endif
    MakeReadyForUse();
    SetZPos(0);
    RequestFocus();
}

MainMenu::~MainMenu()
{
    //m_pButtonFeedback->DeletePanel();
    //m_pButtonFeedback->DeletePanel();
    //m_pButtonLobby->DeletePanel();
    //m_pButtonInviteFriends->DeletePanel();
}

void MainMenu::CreateMenu(const char *menu)
{
    KeyValues *datafile = new KeyValues("MainMenu");
    datafile->UsesEscapeSequences(true);
    if (datafile->LoadFromFile(g_pFullFileSystem, menu))
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
    //g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
#if USE_OLD_MENU
    BaseClass::ApplySchemeSettings(pScheme);

    m_fButtonsSpace = atof(pScheme->GetResourceString("MainMenu.Buttons.Space"));
    m_fButtonsOffsetX = atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetX"));
    m_fButtonsOffsetY = atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetY"));
    m_fLogoOffsetX = atof(pScheme->GetResourceString("MainMenu.Logo.OffsetX"));
    m_fLogoOffsetY = atof(pScheme->GetResourceString("MainMenu.Logo.OffsetY"));
    m_cLogoLeft = GetSchemeColor("MainMenu.Logo.Left", pScheme);
    m_cLogoRight = GetSchemeColor("MainMenu.Logo.Right", pScheme);
    m_bLogoAttachToMenu = atoi(pScheme->GetResourceString("MainMenu.Logo.AttachToMenu"));
    m_bLogoText = atoi(pScheme->GetResourceString("MainMenu.Logo.Text"));
    if (!m_bLogoText)
    {
        if (m_pLogoImage)
            m_pLogoImage->EvictImage();
        else
            m_pLogoImage = new ImagePanel(this, "GameLogo");

        m_pLogoImage->SetAutoDelete(true);
        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        int imageW = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width")));
        int imageH = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height")));
        m_pLogoImage->SetSize(imageW, imageH);
        // Pos is handled in Paint()
    }
    m_bLogoPlayerCount = atoi(pScheme->GetResourceString("MainMenu.Logo.PlayerCount"));
    m_fLogoPlayerCount = pScheme->GetFont("MainMenu.Logo.PlayerCount.Font", true);
    m_cLogoPlayerCount = pScheme->GetColor("MainMenu.Logo.PlayerCount.Color", Color(255, 255, 255, 255));

    m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    Q_strncpy(m_pszMenuOpenSound, pScheme->GetResourceString("MainMenu.Sound.Open"), sizeof(m_pszMenuOpenSound));
    Q_strncpy(m_pszMenuCloseSound, pScheme->GetResourceString("MainMenu.Sound.Close"), sizeof(m_pszMenuCloseSound));
#endif
}

void MainMenu::OnThink()
{
    BaseClass::OnThink();
    if (ipanel())
        SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);

    if (m_pMainMenuHTMLPanel)
    {
        if (m_bInGame != GameUI2().IsInLevel())
        {
            m_bInGame = GameUI2().IsInLevel();
            char visCommand[32];
            Q_snprintf(visCommand, 32, "updateVisibility(%s)", m_bInGame ? "true" : "false");
            m_pMainMenuHTMLPanel->RunJavascript(visCommand);
        }

        if (!CloseEnough(m_fGameVolume, volumeRef.GetFloat(), 0.0001f))
        {
            m_fGameVolume = volumeRef.GetFloat();
            m_pMainMenuHTMLPanel->SendVolumeCommand(m_fGameVolume);
        }
    }
    
        

    //g_pMomentumSteamHelper->CheckLobby();
}

bool MainMenu::IsVisible(void)
{
    if (GameUI2().IsInLoading())
        return false;

    return BaseClass::IsVisible();
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
            pButton->SetVisible(GameUI2().IsInLevel() || GameUI2().IsInBackgroundLevel());
            break;
        case IN_GAME:
            pButton->SetVisible(GameUI2().IsInLevel());
            break;
        case MAIN_MENU:
            pButton->SetVisible(GameUI2().IsInBackgroundLevel());
            break;
        }
    }

    if (GameUI2().IsInLevel())
    {
        m_nSortFlags &= ~FL_SORT_MENU;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_INGAME));
        m_nSortFlags |= FL_SORT_INGAME;
    }
    else if (GameUI2().IsInBackgroundLevel())
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
                                  GameUI2().GetViewport().y - (m_fButtonsOffsetY + m_pButtons[i]->GetHeight()));
        }
    }

    // MOM_TODO: Remove this after it's not needed
        m_pButtonFeedback->SetPos(GameUI2().GetViewport().x - m_pButtonFeedback->GetWidth(),
                                  GameUI2().GetViewport().y - (m_pButtonFeedback->GetHeight() + m_fButtonsOffsetY));
    

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
    m_pButtonLobby->SetPos(GameUI2().GetViewport().x - m_pButtonLobby->GetWidth(), m_fButtonsOffsetY);
    m_pButtonInviteFriends->SetPos(GameUI2().GetViewport().x - m_pButtonInviteFriends->GetWidth(), m_pButtonLobby->GetTall() + m_fButtonsOffsetY);
    
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

            m_pLogoImage->SetPos(SCALE(logoX), SCALE(logoY));
            if (m_bLogoPlayerCount)
            {
                surface()->DrawSetTextColor(m_cLogoPlayerCount);
                surface()->DrawSetTextFont(m_fLogoPlayerCount);
                surface()->DrawSetTextPos(m_pLogoImage->GetXPos(), m_pLogoImage->GetTall() + m_pLogoImage->GetYPos());
                const wchar_t* currentTotalPlayers = g_pMomentumSteamHelper->GetCurrentTotalPlayersAsString();
                surface()->DrawPrintText(currentTotalPlayers, wcslen(currentTotalPlayers));
            }
        }
    }
}

void MainMenu::Paint()
{
    BaseClass::Paint();

#if USE_OLD_MENU
    DrawMainMenu();
    DrawLogo();
#endif
}

void MainMenu::OnCommand(char const *cmd)
{
    GameUI2().GetGameUI()->SendMainMenuCommand(cmd);
    BaseClass::OnCommand(cmd);
}

void MainMenu::OnSetFocus()
{
    BaseClass::OnSetFocus();
    m_bFocused = true;
    surface()->PlaySound(m_pszMenuOpenSound);
}

void MainMenu::OnKillFocus()
{
    BaseClass::OnKillFocus();
    m_bFocused = false;
    surface()->PlaySound(m_pszMenuCloseSound);
}

void MainMenu::ReloadMenu()
{
    if (m_pMainMenuHTMLPanel)
        m_pMainMenuHTMLPanel->LoadMenu();
}
