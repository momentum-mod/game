#include "GameUI_Interface.h"

#include <tier0/dbg.h>

#include "filesystem.h"
#include "tier0/icommandline.h"

// interface to engine
#include "EngineInterface.h"
#include "ienginevgui.h"
#include "IGameUIFuncs.h"
#include "game/client/IGameClientExports.h"
#include "iachievementmgr.h"
#include "materialsystem/imaterialsystem.h"

// vgui2 interface
// note that GameUI project uses ..\vgui2\include, not ..\utils\vgui\include
#include "matsys_controls/matsyscontrols.h"
#include "steam/steam_api.h"
#include "tier1/KeyValues.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "MainMenu.h"

#include "BasePanel.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IVEngineClient *engine = nullptr;
IGameUIFuncs *gameuifuncs = nullptr;
IEngineVGui *enginevguifuncs = nullptr;
IEngineSound *enginesound = nullptr;
vgui::ISurface *enginesurfacefuncs = nullptr;
IAchievementMgr *achievementmgr = nullptr;
IGameEventManager2 *gameeventmanager = nullptr;
static CBasePanel *staticPanel = nullptr;

vgui::VPANEL g_hLoadingBackgroundDialog = NULL;

static IGameClientExports *g_pGameClientExports = nullptr;
IGameClientExports *GameClientExports() { return g_pGameClientExports; }

//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
CGameUI *g_pGameUI = nullptr;
static CGameUI g_GameUI;
CGameUI &GameUI() { return g_GameUI; }
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameUI, IGameUI, GAMEUI_INTERFACE_VERSION, g_GameUI);

CGameUI::CGameUI()
{
    g_pGameUI = this;
    m_iPlayGameStartupSound = 0;
}

CGameUI::~CGameUI() { g_pGameUI = nullptr; }

//-----------------------------------------------------------------------------
// Purpose: Initialization
//-----------------------------------------------------------------------------
void CGameUI::Initialize(CreateInterfaceFn factory)
{
    MEM_ALLOC_CREDIT();
    ConnectTier1Libraries(&factory, 1);
    ConnectTier2Libraries(&factory, 1);
    ConVar_Register(FCVAR_CLIENTDLL);
    ConnectTier3Libraries(&factory, 1);
    
    SteamAPI_InitSafe();

    vgui::VGui_InitInterfacesList("GameUI", &factory, 1);
    vgui::VGui_InitMatSysInterfacesList("GameUI", &factory, 1);

    // load localization file
    g_pVGuiLocalize->AddFile("resource/gameui_%language%.txt", "GAME", true);
    g_pVGuiLocalize->AddFile("resource/momentum_%language%.txt", "GAME", true);

    // load localization file for kb_act.lst
    g_pVGuiLocalize->AddFile("resource/valve_%language%.txt", "GAME", true);

    enginevguifuncs = static_cast<IEngineVGui *>(factory(VENGINE_VGUI_VERSION, nullptr));
    enginesurfacefuncs = static_cast<vgui::ISurface *>(factory(VGUI_SURFACE_INTERFACE_VERSION, nullptr));
    gameuifuncs = static_cast<IGameUIFuncs *>(factory(VENGINE_GAMEUIFUNCS_VERSION, nullptr));
    enginesound = static_cast<IEngineSound *>(factory(IENGINESOUND_CLIENT_INTERFACE_VERSION, nullptr));
    engine = static_cast<IVEngineClient *>(factory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
    gameeventmanager = static_cast<IGameEventManager2 *>(factory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr));

    if (!engine || !enginesound || !enginesurfacefuncs || !gameuifuncs || !enginevguifuncs)
    {
        Error("CGameUI::Initialize() failed to get necessary interfaces\n");
    }

    // setup base panel
    staticPanel = new CBasePanel();

    staticPanel->SetBounds(0, 0, 640, 480);
    staticPanel->SetPaintBorderEnabled(false);
    staticPanel->SetPaintBackgroundEnabled(true);
    staticPanel->SetPaintEnabled(true);
    staticPanel->SetVisible(true);

    staticPanel->SetMouseInputEnabled(true);
    staticPanel->SetKeyBoardInputEnabled(true);

    vgui::VPANEL rootpanel = enginevguifuncs->GetPanel(PANEL_GAMEUIDLL);
    staticPanel->SetParent(rootpanel);
}

void CGameUI::PostInit()
{
}

//-----------------------------------------------------------------------------
// Purpose: Sets the specified panel as the background panel for the loading
//		dialog.  If NULL, default background is used.  If you set a panel,
//		it should be full-screen with an opaque background, and must be a VGUI popup.
//-----------------------------------------------------------------------------
void CGameUI::SetLoadingBackgroundDialog(vgui::VPANEL panel) { g_hLoadingBackgroundDialog = panel; }

vgui::VPANEL CGameUI::GetLoadingBackgroundDialog()
{
    return g_hLoadingBackgroundDialog;
}

//-----------------------------------------------------------------------------
// Purpose: connects to client interfaces
//-----------------------------------------------------------------------------
void CGameUI::Connect(CreateInterfaceFn gameFactory)
{
    g_pGameClientExports = static_cast<IGameClientExports *>(gameFactory(GAMECLIENTEXPORTS_INTERFACE_VERSION, nullptr));

    achievementmgr = engine->GetAchievementMgr();

    if (!g_pGameClientExports)
    {
        Error("CGameUI::Initialize() failed to get necessary interfaces\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Searches for GameStartup*.mp3 files in the sound/ui folder and plays one
//-----------------------------------------------------------------------------
void CGameUI::PlayGameStartupSound()
{
    if (CommandLine()->FindParm("-nostartupsound"))
        return;

    FileFindHandle_t fh;

    CUtlVector<char *> fileNames;

    char path[512];
    Q_snprintf(path, sizeof(path), "sound/ui/gamestartup*.mp3");
    Q_FixSlashes(path);

    char const *fn = g_pFullFileSystem->FindFirstEx(path, "MOD", &fh);
    if (!fn)
    {
        g_pFullFileSystem->FindClose(fh);
        return;
    }

    do
    {
        char ext[10];
        Q_ExtractFileExtension(fn, ext, sizeof(ext));

        if (!Q_stricmp(ext, "mp3"))
        {
            char temp[512];
            Q_snprintf(temp, sizeof(temp), "ui/%s", fn);

            char *found = new char[strlen(temp) + 1];
            Q_strncpy(found, temp, strlen(temp) + 1);

            Q_FixSlashes(found);
            fileNames.AddToTail(found);
        }

        fn = g_pFullFileSystem->FindNext(fh);

    } while (fn);

    g_pFullFileSystem->FindClose(fh);

    if (fileNames.Count() > 0)
    {
        char found[512];

        // escape chars "*#" make it stream, and be affected by snd_musicvolume
        Q_snprintf(found, sizeof(found), "play *#%s", fileNames.Random());

        engine->ClientCmd_Unrestricted(found);

        fileNames.PurgeAndDeleteElements();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Called to setup the game UI
//-----------------------------------------------------------------------------
void CGameUI::Start()
{
    char szConfigDir[512];
    g_pFullFileSystem->RelativePathToFullPath("config", "PLATFORM", szConfigDir, sizeof(szConfigDir));

    DevMsg("Steam config directory: %s\n", szConfigDir);

    g_pFullFileSystem->AddSearchPath(szConfigDir, "CONFIG");
    g_pFullFileSystem->CreateDirHierarchy("", "CONFIG");

    vgui::system()->SetUserConfigFile("InGameDialogConfig.vdf", "CONFIG");

    g_pFullFileSystem->AddSearchPath("platform", "PLATFORM");
    

    g_pVGuiLocalize->AddFile("resource/platform_%language%.txt");
    g_pVGuiLocalize->AddFile("resource/vgui_%language%.txt");

    // Delay playing the startup music until two frames
    // this allows cbuf commands that occur on the first frame that may start a map
    m_iPlayGameStartupSound = 2;
}

//-----------------------------------------------------------------------------
// Purpose: Called to Shutdown the game UI system
//-----------------------------------------------------------------------------
void CGameUI::Shutdown()
{
    ConVar_Unregister();
    DisconnectTier3Libraries();
    DisconnectTier2Libraries();
    DisconnectTier1Libraries();
}

//-----------------------------------------------------------------------------
// Purpose: just wraps an engine call to activate the gameUI
//-----------------------------------------------------------------------------
void CGameUI::ActivateGameUI() { engine->ExecuteClientCmd("gameui_activate"); }

//-----------------------------------------------------------------------------
// Purpose: just wraps an engine call to hide the gameUI
//-----------------------------------------------------------------------------
void CGameUI::HideGameUI() { engine->ExecuteClientCmd("gameui_hide"); }

//-----------------------------------------------------------------------------
// Purpose: Toggle allowing the engine to hide the game UI with the escape key
//-----------------------------------------------------------------------------
void CGameUI::PreventEngineHideGameUI() { engine->ExecuteClientCmd("gameui_preventescape"); }

//-----------------------------------------------------------------------------
// Purpose: Toggle allowing the engine to hide the game UI with the escape key
//-----------------------------------------------------------------------------
void CGameUI::AllowEngineHideGameUI() { engine->ExecuteClientCmd("gameui_allowescape"); }

//-----------------------------------------------------------------------------
// Purpose: Activate the game UI
//-----------------------------------------------------------------------------
void CGameUI::OnGameUIActivated()
{
    GetBasePanel()->OnGameUIActivated();
}

//-----------------------------------------------------------------------------
// Purpose: Hides the game ui, in whatever state it's in
//-----------------------------------------------------------------------------
void CGameUI::OnGameUIHidden()
{
    GetBasePanel()->OnGameUIHidden();
}

//-----------------------------------------------------------------------------
// Purpose: paints all the vgui elements
//-----------------------------------------------------------------------------
void CGameUI::RunFrame()
{
    int wide, tall;
    vgui::surface()->GetScreenSize(wide, tall);
    staticPanel->SetSize(wide, tall);

    GetBasePanel()->RunFrame();

    // Play the start-up music the first time we run frame
    if (m_iPlayGameStartupSound > 0)
    {
        m_iPlayGameStartupSound--;
        if (!m_iPlayGameStartupSound)
        {
            PlayGameStartupSound();
        }
    }
}

Vector2D CGameUI::GetViewport() const
{
    int wide, tall;
    engine->GetScreenSize(wide, tall);
    return Vector2D(wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: activates the loading dialog on level load start
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingStarted(bool bShowProgressDialog)
{
    GetBasePanel()->OnLevelLoadingStarted();

    ShowLoadingBackgroundDialog();

    // Don't play the start game sound if this happens before we get to the first frame
    m_iPlayGameStartupSound = 0;
}

//-----------------------------------------------------------------------------
// Purpose: closes any level load dialog
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingFinished(bool bError, const char *failureReason, const char *extendedReason)
{
    HideLoadingBackgroundDialog();

    HideGameUI();

    GetBasePanel()->OnLevelLoadingFinished();
}

//-----------------------------------------------------------------------------
// Purpose: Updates progress bar
// Output : Returns true if screen should be redrawn
//-----------------------------------------------------------------------------
bool CGameUI::UpdateProgressBar(float progress, const char *statusText)
{
    bool bRedraw = false;

    if (ContinueProgressBar(progress))
    {
        bRedraw = true;
    }

    return bRedraw;
}

void CGameUI::GetLocalizedString(const char* pToken, wchar_t** pOut)
{
    if (pToken[0] == '#')
    {
        wchar_t *pLocalizedString = g_pVGuiLocalize->Find(pToken);
        if (pLocalizedString)
        {
            const size_t sizeInBytes = (Q_wcslen(pLocalizedString) + 1) * sizeof(wchar_t);
            *pOut = static_cast<wchar_t*>(calloc(1, sizeInBytes));
            Q_wcsncpy(*pOut, pLocalizedString, sizeInBytes);
        }
        else
        {
            g_pVGuiLocalize->ConvertUTF8ToUTF16(pToken, pOut);
        }
    }
    else
    {
        g_pVGuiLocalize->ConvertUTF8ToUTF16(pToken, pOut);
    }
}

bool CGameUI::ContinueProgressBar(float progressFraction)
{
    if (g_hLoadingBackgroundDialog == NULL)
        return false;

    KeyValues *pKV = new KeyValues("ProgressFraction");
    pKV->SetFloat("percent", progressFraction);
    vgui::ivgui()->PostMessage(g_hLoadingBackgroundDialog, pKV, NULL);
    return true;
}

bool CGameUI::IsInLevel()
{
    return engine->IsInGame() && !engine->IsLevelMainMenuBackground();
}

bool CGameUI::IsInBackgroundLevel()
{
    return (engine->IsInGame() && engine->IsLevelMainMenuBackground());
}

bool CGameUI::IsInMenu()
{
    return IsInBackgroundLevel() || GetBasePanel()->GetMenuBackgroundState() == BACKGROUND_DISCONNECTED;
}

bool CGameUI::IsInMultiplayer() { return (IsInLevel() && engine->GetMaxClients() > 1); }

void CGameUI::ShowLoadingBackgroundDialog()
{
    if (g_hLoadingBackgroundDialog)
    {
        vgui::ipanel()->SetParent(g_hLoadingBackgroundDialog, staticPanel->GetVPanel());
        vgui::ipanel()->SendMessage(g_hLoadingBackgroundDialog, new KeyValues("Activate"), staticPanel->GetVPanel());
    }
}

void CGameUI::HideLoadingBackgroundDialog()
{
    if (g_hLoadingBackgroundDialog)
    {
        vgui::ivgui()->PostMessage(g_hLoadingBackgroundDialog, new KeyValues("Deactivate", "loaded_into_game", engine->IsInGame()), staticPanel->GetVPanel());
    }
}

void CGameUI::SendMainMenuCommand(const char* pszCommand)
{
    GetBasePanel()->RunMenuCommand(pszCommand);
}

void CGameUI::OnConfirmQuit()
{
    GetBasePanel()->RunEngineCommand("quit\n");
}

bool CGameUI::IsMainMenuVisible()
{
    return GetBasePanel()->GetMainMenu() && GetBasePanel()->GetMainMenu()->IsVisible();
}