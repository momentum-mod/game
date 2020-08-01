#include "GameUI_Interface.h"

#include <tier0/dbg.h>

#include "filesystem.h"
#include "tier0/icommandline.h"

// interface to engine
#include "cdll_int.h"
#include "IGameUIFuncs.h"
#include "game/client/IGameClientExports.h"

// vgui2 interface
// note that GameUI project uses ..\vgui2\include, not ..\utils\vgui\include
#include "matsys_controls/matsyscontrols.h"
#include "tier1/KeyValues.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IVEngineClient *engine = nullptr;
IGameUIFuncs *gameuifuncs = nullptr;
static IGameClientExports *g_pGameClientExports = nullptr;

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
    m_hBasePanel = 0;
    m_bIsInLoading = false;
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

    vgui::VGui_InitInterfacesList("GameUI", &factory, 1);
    vgui::VGui_InitMatSysInterfacesList("GameUI", &factory, 1);

    // load localization file
    g_pVGuiLocalize->AddFile("resource/gameui_%language%.txt", "GAME", true);
    g_pVGuiLocalize->AddFile("resource/momentum_%language%.txt", "GAME", true);

    // load localization file for kb_act.lst
    g_pVGuiLocalize->AddFile("resource/valve_%language%.txt", "GAME", true);

    engine = static_cast<IVEngineClient *>(factory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
    gameuifuncs = static_cast<IGameUIFuncs *>(factory(VENGINE_GAMEUIFUNCS_VERSION, nullptr));

    if (!engine || !gameuifuncs)
    {
        Error("CGameUI::Initialize() failed to get necessary interfaces\n");
    }
}

void CGameUI::PostInit()
{
}

//-----------------------------------------------------------------------------
// Purpose: connects to client interfaces
//-----------------------------------------------------------------------------
void CGameUI::Connect(CreateInterfaceFn gameFactory)
{
    g_pGameClientExports = static_cast<IGameClientExports *>(gameFactory(GAMECLIENTEXPORTS_INTERFACE_VERSION, nullptr));

    if (!g_pGameClientExports)
    {
        Error("CGameUI::Initialize() failed to get necessary interfaces\n");
    }

    m_hBasePanel = g_pGameClientExports->GetBasePanel();
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
    if (m_hBasePanel)
    {
        const auto pKv = new KeyValues("GameUIActivated");
        vgui::ivgui()->PostMessage(m_hBasePanel, pKv, NULL);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Hides the game ui, in whatever state it's in
//-----------------------------------------------------------------------------
void CGameUI::OnGameUIHidden()
{
    if (m_hBasePanel)
    {
        const auto pKv = new KeyValues("GameUIHidden");
        vgui::ivgui()->PostMessage(m_hBasePanel, pKv, NULL);
    }
}

//-----------------------------------------------------------------------------
// Purpose: paints all the vgui elements
//-----------------------------------------------------------------------------
void CGameUI::RunFrame()
{
    vgui::GetAnimationController()->UpdateAnimations(engine->Time());

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

//-----------------------------------------------------------------------------
// Purpose: activates the loading dialog on level load start
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingStarted(bool bShowProgressDialog)
{
    m_bIsInLoading = true;

    const auto pKv = new KeyValues("LevelLoadStarted");
    vgui::ivgui()->PostMessage(m_hBasePanel, pKv, NULL);
    
    // Don't play the start game sound if this happens before we get to the first frame
    m_iPlayGameStartupSound = 0;
}

//-----------------------------------------------------------------------------
// Purpose: closes any level load dialog
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingFinished(bool bError, const char *failureReason, const char *extendedReason)
{
    m_bIsInLoading = false;

    HideGameUI();

    const auto pKv = new KeyValues("LevelLoadFinished");
    pKv->SetBool("error", bError);
    pKv->SetString("failureReason", failureReason);
    pKv->SetString("extendedReason", extendedReason);
    vgui::ivgui()->PostMessage(m_hBasePanel, pKv, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Updates progress bar
// Output : Returns true if screen should be redrawn
//-----------------------------------------------------------------------------
bool CGameUI::UpdateProgressBar(float progress, const char *statusText)
{
    const auto pKV = new KeyValues("ProgressFraction");
    pKV->SetFloat("percent", progress);
    vgui::ivgui()->PostMessage(m_hBasePanel, pKV, NULL);

    return true;
}

void CGameUI::SendMainMenuCommand(const char* pszCommand)
{
    vgui::ivgui()->PostMessage(m_hBasePanel, new KeyValues("RunMenuCommand", "command", pszCommand), NULL);
}

void CGameUI::OnConfirmQuit()
{
    SendMainMenuCommand("engine quit\n");
}

bool CGameUI::IsMainMenuVisible()
{
    KeyValuesAD basePanelInfo("menu_visible");
    vgui::ipanel()->RequestInfo(m_hBasePanel, basePanelInfo);
    return basePanelInfo->GetBool("response");
}