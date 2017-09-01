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

using namespace vgui;

class MomentumURLResolver : public Panel
{
    DECLARE_CLASS_SIMPLE(MomentumURLResolver, Panel);
    MomentumURLResolver(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
    {
        SetVisible(false);
        SetEnabled(false);
    }

protected:
    MESSAGE_FUNC_CHARPTR_CHARPTR(OnCustomURL, "CustomURL", schema, URL)
    {
        DevLog("Going to custom URL %s\n", URL);
        // Substring out the file
        const char* pFile = Q_strstr(URL, schema) + Q_strlen(schema);
        DevLog("Finding file %s...\n", pFile);
        // Create the rough path to the file
        char path[MAX_PATH];
        V_snprintf(path, MAX_PATH, "resource/html/%s.html", pFile);
        // Translate to full path on system
        char fullPath[1024];
        g_pFullFileSystem->RelativePathToFullPath(path, "MOD", fullPath, 1024);
        // Append the file schema and fix the slashes
        char finalPath[1024];
        V_snprintf(finalPath, 1024, "file:///%s", fullPath);
        V_FixSlashes(finalPath, '/'); // Only use forward slashes here
        // Done! Forward this to our parent HTML class
        DevLog("Full file URL path: %s\n", finalPath);
        PostActionSignal(new KeyValues("ResolvedURL", "url", finalPath));
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

    MESSAGE_FUNC_CHARPTR(OnURLResolved, "ResolvedURL", url)
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
    AddActionSignalTarget(this);

    SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);

    m_bInGame = false;
    m_fGameVolume = volumeRef.GetFloat();

    m_pMainMenuHTMLPanel = new MainMenuHTML(this, "CMainMenuHTML");
    int wide, high;
    GetSize(wide, high);
    m_pMainMenuHTMLPanel->SetSize(wide, high);
    m_pMainMenuHTMLPanel->LoadMenu();

    MakeReadyForUse();
    SetZPos(0);
    RequestFocus();

    ivgui()->AddTickSignal(GetVPanel(), 120000); // Tick every 2 minutes
    // First check here
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
}

MainMenu::~MainMenu()
{
    ivgui()->RemoveTickSignal(GetVPanel());
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
        
}

void MainMenu::OnTick()
{
    // We're the only one who should call this! (As we tick once every 2 mins)
    g_pMomentumSteamHelper->RequestCurrentTotalPlayers();
}

bool MainMenu::IsVisible(void)
{
    if (GameUI2().IsInLoading())
        return false;

    return BaseClass::IsVisible();
}

void MainMenu::OnCommand(char const *cmd)
{
    GameUI2().GetGameUI()->SendMainMenuCommand(cmd);
    BaseClass::OnCommand(cmd);
}

void MainMenu::OnSetFocus()
{
    BaseClass::OnSetFocus();
    surface()->PlaySound(m_pszMenuOpenSound);
}

void MainMenu::OnKillFocus()
{
    BaseClass::OnKillFocus();
    surface()->PlaySound(m_pszMenuCloseSound);
}

void MainMenu::ReloadMenu()
{
    if (m_pMainMenuHTMLPanel)
        m_pMainMenuHTMLPanel->LoadMenu();
}
