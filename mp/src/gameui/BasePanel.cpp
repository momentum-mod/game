#include "BasePanel.h"
#include "GameUI_Interface.h"
#include "MainMenu.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "tier0/icommandline.h"
#include "OptionsDialog.h"
#include "steam/steam_api.h"
#include "EngineInterface.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_controls/AnimationController.h"
#include "LoadingDialog.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern DHANDLE<CLoadingDialog> g_hLoadingDialog;

static CBasePanel *g_pBasePanel;
CBasePanel *GetBasePanel() { return g_pBasePanel; }

CON_COMMAND(reload_menu, "Reloads the menu\n")
{
    GetBasePanel()->GetMainMenu()->CreateMenu();
}

//-----------------------------------------------------------------------------
// Purpose: hack function to give the module loader access to the main panel handle
//			only used in VguiSystemModuleLoader
//-----------------------------------------------------------------------------
VPANEL GetGameUIBasePanel()
{
    return GetBasePanel()->GetVPanel();
}

CBasePanel::CBasePanel() : BaseClass(nullptr, "BaseGameUIPanel")
{
    g_pBasePanel = this;

    m_bLevelLoading = false;
    m_eBackgroundState = BACKGROUND_INITIAL;
    m_flTransitionStartTime = 0.0f;
    m_flTransitionEndTime = 0.0f;
    m_flFrameFadeInTime = 0.5f;
    m_bRenderingBackgroundTransition = false;
    m_bFadingInMenus = false;
    m_bEverActivated = false;
    m_bHaveDarkenedBackground = false;
    m_BackdropColor = Color(0, 0, 0, 128);

    g_pVGuiLocalize->AddFile("resource/momentum_%language%.txt");

    m_pMainMenu = new MainMenu(this);
    SETUP_PANEL(m_pMainMenu);
}

CBasePanel::~CBasePanel()
{
    g_pBasePanel = nullptr;
}

void CBasePanel::OnLevelLoadingStarted()
{
    m_bLevelLoading = true;
}

void CBasePanel::OnLevelLoadingFinished()
{
    m_bLevelLoading = false;
}

bool CBasePanel::IsInLoading() const
{
    return m_bLevelLoading;
}

void CBasePanel::RunFrame()
{
    InvalidateLayout();
    GetAnimationController()->UpdateAnimations(engine->Time());

    UpdateBackgroundState();
}

void CBasePanel::FadeToBlackAndRunEngineCommand(const char* engineCommand)
{
    KeyValues *pKV = new KeyValues("RunEngineCommand", "command", engineCommand);

    // execute immediately, with no delay
    PostMessage(this, pKV, 0);
}

void CBasePanel::OnGameUIActivated()
{
    // If the load failed, we're going to bail out here
    if (engine->MapLoadFailed())
    {
        // Don't display this again until it happens again
        engine->SetMapLoadFailed(false);
    }

    if (!m_bEverActivated)
    {
        // Layout the first time to avoid focus issues (setting menus visible will grab focus)
        //UpdateGameMenus();
        // MOM_TODO do something with the HTML menu here?

        m_bEverActivated = true;
    }

    m_pMainMenu->SetVisible(true);

    if (GameUI().IsInLevel())
    {
        OnCommand("OpenPauseMenu");

        if (m_hAchievementsDialog.Get())
        {
            // Achievement dialog refreshes it's data if the player looks at the pause menu
            m_hAchievementsDialog->OnCommand("OnGameUIActivated");
        }
    }
}

void CBasePanel::OnGameUIHidden()
{
    if (m_hOptionsDialog.Get())
    {
        PostMessage(m_hOptionsDialog.Get(), new KeyValues("GameUIHidden"));
    }

    m_pMainMenu->SetVisible(false);

    // HACKISH: Force this dialog closed so it gets data updates upon reopening.
    Frame* pAchievementsFrame = m_hAchievementsDialog.Get();
    if (pAchievementsFrame)
    {
        pAchievementsFrame->Close();
    }
}

void CBasePanel::OnOpenOptionsDialog()
{
    if (!m_hOptionsDialog.Get())
    {
        m_hOptionsDialog = new COptionsDialog(this);
        PositionDialog(m_hOptionsDialog);
    }

    m_hOptionsDialog->Activate();
}

void CBasePanel::OnOpenAchievementsDialog()
{
    Warning("MOM_TODO: Implement the achievements dialog!\n");
#if 0
    if (!m_hAchievementsDialog.Get())
    {
        m_hAchievementsDialog = new CAchievementsDialog(this);
        PositionDialog(m_hAchievementsDialog);
    }
    m_hAchievementsDialog->Activate();
#endif
}

void CBasePanel::PositionDialog(PHandle dlg)
{
    if (!dlg.Get())
        return;

    int x, y, ww, wt, wide, tall;
    surface()->GetWorkspaceBounds(x, y, ww, wt);
    dlg->GetSize(wide, tall);

    // Center it, keeping requested size
    dlg->SetPos(x + ((ww - wide) / 2), y + ((wt - tall) / 2));
}

void CBasePanel::ApplyOptionsDialogSettings()
{
    if (m_hOptionsDialog.Get())
    {
        m_hOptionsDialog->ApplyChanges();
    }
}

void CBasePanel::SetMenuAlpha(int alpha)
{
    if (m_pMainMenu)
        m_pMainMenu->SetAlpha(alpha);
}

void CBasePanel::UpdateBackgroundState()
{
    if (GameUI().IsInLevel())
    {
        SetBackgroundRenderState(BACKGROUND_LEVEL);
    }
    else if (GameUI().IsInBackgroundLevel() && !m_bLevelLoading)
    {
        // level loading is truly completed when the progress bar is gone, then transition to main menu
        SetBackgroundRenderState(BACKGROUND_MAINMENU);
    }
    else if (m_bLevelLoading)
    {
        SetBackgroundRenderState(BACKGROUND_LOADING);
    }
    else if (m_bEverActivated)
    {
        SetBackgroundRenderState(BACKGROUND_DISCONNECTED);
    }

    // check for background fill
    // fill over the top if we have any dialogs up
    int i;
    bool bHaveActiveDialogs = false;
    bool bIsInLevel = GameUI().IsInLevel();
    int childCount = GetChildCount();
    for (i = 0; i < childCount; ++i)
    {
        VPANEL child = ipanel()->GetChild(GetVPanel(), i);
        if (child
            && ipanel()->IsVisible(child)
            && ipanel()->IsPopup(child)
            && child != m_pMainMenu->GetVPanel())
        {
            bHaveActiveDialogs = true;
        }
    }
    // see if the base gameui panel has dialogs hanging off it (engine stuff, console, bug reporter)
    VPANEL parent = GetVParent();
    childCount = ipanel()->GetChildCount(parent);
    for (i = 0; i < childCount; ++i)
    {
        VPANEL child = ipanel()->GetChild(parent, i);
        if (child
            && ipanel()->IsVisible(child)
            && ipanel()->IsPopup(child)
            && child != GetVPanel())
        {
            bHaveActiveDialogs = true;
        }
    }

    // check to see if we need to fade in the background fill
    bool bNeedDarkenedBackground = (bHaveActiveDialogs || bIsInLevel);
    if (m_bHaveDarkenedBackground != bNeedDarkenedBackground)
    {
        // fade in faster than we fade out
        float targetAlpha, duration;
        if (bNeedDarkenedBackground)
        {
            // fade in background tint
            targetAlpha = m_BackdropColor[3];
            duration = m_flFrameFadeInTime;
        }
        else
        {
            // fade out background tint
            targetAlpha = 0.0f;
            duration = 2.0f;
        }

        m_bHaveDarkenedBackground = bNeedDarkenedBackground;
        GetAnimationController()->RunAnimationCommand(this, "m_flBackgroundFillAlpha", targetAlpha, 0.0f, duration, AnimationController::INTERPOLATOR_LINEAR);
    }
}

void CBasePanel::DrawBackgroundImage()
{
    int wide, tall;
    GetSize(wide, tall);

    float frametime = engine->Time();

    // a background transition has a running map underneath it, so fade image out
    // otherwise, there is no map and the background image stays opaque
    int alpha = 255;
    if (m_bRenderingBackgroundTransition)
    {
        // goes from [255..0]
        alpha = static_cast<int>((m_flTransitionEndTime - frametime) / (m_flTransitionEndTime - m_flTransitionStartTime) * 255.0f);
        alpha = clamp(alpha, 0, 255);
    }

    surface()->DrawSetColor(255, 255, 255, alpha);
    surface()->DrawSetTexture(m_iBackgroundImageID);
    surface()->DrawTexturedRect(0, 0, wide, tall);

    // 360 always use the progress bar, TCR Requirement, and never this loading plaque
    if (m_bRenderingBackgroundTransition || m_eBackgroundState == BACKGROUND_LOADING)
    {
        // draw the loading image over the top
        surface()->DrawSetColor(255, 255, 255, alpha);
        surface()->DrawSetTexture(m_iLoadingImageID);
        int twide, ttall;
        surface()->DrawGetTextureSize(m_iLoadingImageID, twide, ttall);
        surface()->DrawTexturedRect(wide - twide, tall - ttall, wide, tall);
    }

    // update the menu alpha
    if (m_bFadingInMenus)
    {
        // goes from [0..255]
        alpha = (frametime - m_flFadeMenuStartTime) / (m_flFadeMenuEndTime - m_flFadeMenuStartTime) * 255;
        alpha = clamp(alpha, 0, 255);
        m_pMainMenu->SetAlpha(alpha);
        if (alpha == 255)
        {
            m_bFadingInMenus = false;
        }
    }
}

void CBasePanel::OnThink()
{
    BaseClass::OnThink();

    if (ipanel())
        SetBounds(0, 0, GameUI().GetViewport().x, GameUI().GetViewport().y);
}

void CBasePanel::PaintBlurMask()
{
    BaseClass::PaintBlurMask();

    if (GameUI().IsInLevel())
    {
        surface()->DrawSetColor(Color(255, 255, 255, 255));
        surface()->DrawFilledRect(0, 0, GameUI().GetViewport().x, GameUI().GetViewport().y);
    }
}

void CBasePanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_flFrameFadeInTime = atof(pScheme->GetResourceString("Frame.TransitionEffectTime"));

    // work out current focus - find the topmost panel
    SetBgColor(Color(0, 0, 0, 0));

    m_BackdropColor = pScheme->GetColor("mainmenu.backdrop", Color(0, 0, 0, 128));

    char filename[MAX_PATH];


    int screenWide, screenTall;
    surface()->GetScreenSize(screenWide, screenTall);
    float aspectRatio = (float) screenWide / (float) screenTall;
    bool bIsWidescreen = aspectRatio >= 1.5999f;

    // work out which background image to use
    // pc uses blurry backgrounds based on the background level
    char background[MAX_PATH];
    engine->GetMainMenuBackgroundName(background, sizeof(background));
    Q_snprintf(filename, sizeof(filename), "console/%s%s", background, (bIsWidescreen ? "_widescreen" : ""));

    m_iBackgroundImageID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iBackgroundImageID, filename, false, false);

    // load the loading icon
    m_iLoadingImageID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iLoadingImageID, "console/startup_loading", false, false);
}

void CBasePanel::PaintBackground()
{
    if (!GameUI().IsInLevel() || g_hLoadingDialog.Get())
    {
        // not in the game or loading dialog active or exiting, draw the ui background
        DrawBackgroundImage();
    }

    if (m_flBackgroundFillAlpha)
    {
        int swide, stall;
        surface()->GetScreenSize(swide, stall);
        surface()->DrawSetColor(0, 0, 0, m_flBackgroundFillAlpha);
        surface()->DrawFilledRect(0, 0, swide, stall);
    }
}

void CBasePanel::RunEngineCommand(const char* command)
{
    engine->ClientCmd_Unrestricted(command);
}

void CBasePanel::RunMenuCommand(const char* command)
{
    if (!Q_stricmp(command, "OpenGameMenu"))
    {
        if (m_pMainMenu)
        {
            // MOM_TODO: Do we even need to do this?
            PostMessage(m_pMainMenu, new KeyValues("Command", "command", "Open"));
        }
    }
    else if (!Q_stricmp(command, "OpenOptionsDialog"))
    {
        OnOpenOptionsDialog();
    }
    else if (!Q_stricmp(command, "OpenAchievementsDialog"))
    {
        if (!GameUI().GetSteamContext()->SteamUser() || !GameUI().GetSteamContext()->SteamUser()->BLoggedOn())
        {
            MessageBox *pMessageBox = new MessageBox("#GameUI_Achievements_SteamRequired_Title", "#GameUI_Achievements_SteamRequired_Message");
            pMessageBox->DoModal();
            return;
        }
        OnOpenAchievementsDialog();
    }
    else if (!Q_stricmp(command, "Quit") || !Q_stricmp(command, "QuitNoConfirm"))
    {
        // hide everything while we quit
        SetVisible(false);
        surface()->RestrictPaintToSinglePanel(GetVPanel());
        engine->ClientCmd_Unrestricted("quit\n");
    }
    else if (!Q_stricmp(command, "ResumeGame"))
    {
        GameUI().HideGameUI();
    }
    else if (!Q_stricmp(command, "Disconnect"))
    {
        engine->ClientCmd_Unrestricted("disconnect");
    }
    else if (!Q_stricmp(command, "DisconnectNoConfirm"))
    {
        // Leave our current session, if we have one
    }
    else if (!Q_stricmp(command, "ReleaseModalWindow"))
    {
        surface()->RestrictPaintToSinglePanel(NULL);
    }
    else if (Q_stristr(command, "engine "))
    {
        const char *engineCMD = strstr(command, "engine ") + strlen("engine ");
        if (strlen(engineCMD) > 0)
        {
            engine->ClientCmd_Unrestricted(const_cast<char *>(engineCMD));
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void CBasePanel::SetBackgroundRenderState(EBackgroundState state)
{
    if (state == m_eBackgroundState)
    {
        return;
    }

    // apply state change transition
    float frametime = engine->Time();

    m_bRenderingBackgroundTransition = false;
    m_bFadingInMenus = false;

    if (state == BACKGROUND_DISCONNECTED || state == BACKGROUND_MAINMENU)
    {
        // menu fading
        // make the menus visible
        m_bFadingInMenus = true;
        m_flFadeMenuStartTime = frametime;
        m_flFadeMenuEndTime = frametime + 3.0f;

        if (state == BACKGROUND_MAINMENU)
        {
            // fade background into main menu
            m_bRenderingBackgroundTransition = true;
            m_flTransitionStartTime = frametime;
            m_flTransitionEndTime = frametime + 3.0f;
        }
    }
    else if (state == BACKGROUND_LOADING)
    {
        // hide the menus
        SetMenuAlpha(0);
    }
    else if (state == BACKGROUND_LEVEL)
    {
        // show the menus
        SetMenuAlpha(255);
    }

    m_eBackgroundState = state;
}
