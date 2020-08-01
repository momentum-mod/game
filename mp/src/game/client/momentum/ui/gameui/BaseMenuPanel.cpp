#include "cbase.h"

#include "BaseMenuPanel.h"
#include "mainmenu/MainMenu.h"
#include "loadingscreen/LoadingScreen.h"

#include "gameui/GameUIUtil.h"

#include "vgui/ISurface.h"
#include "tier0/icommandline.h"
#include "ienginevgui.h"

#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

bool GameUIUtil::IsInLevel()
{
    return engine->IsInGame() && !engine->IsLevelMainMenuBackground();
}

bool GameUIUtil::IsInBackgroundLevel()
{
    return engine->IsInGame() && engine->IsLevelMainMenuBackground();
}

bool GameUIUtil::IsInMenu()
{
    return IsInBackgroundLevel() || g_pBasePanel->GetMenuBackgroundState() == BACKGROUND_DISCONNECTED;
}

CBaseMenuPanel *g_pBasePanel = nullptr;

CBaseMenuPanel::CBaseMenuPanel() : BaseClass(nullptr, "BaseGameUIPanel")
{
    g_pBasePanel = this;

    SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));

    SetBounds(0, 0, 640, 480);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(true);
    SetPaintEnabled(true);
    SetVisible(true);

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);

    m_iBackgroundImageID = 0;
    m_iLoadingImageID = 0;
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

    SetZPos(-100);
    m_pMainMenu = new MainMenu(this);

    m_pLoadingScreen = new CLoadingScreen(this);

    OnGameUIActivated(); // Done here because normally it was done earlier than construction of this panel
}

CBaseMenuPanel::~CBaseMenuPanel()
{
    g_pBasePanel = nullptr;
}

void CBaseMenuPanel::Init()
{
    if (g_pBasePanel)
        return;

    g_pBasePanel = new CBaseMenuPanel;
}

void CBaseMenuPanel::OnLevelLoadingStarted(KeyValues *pKvData)
{
    m_bLevelLoading = true;

    m_pLoadingScreen->Activate();
}

void CBaseMenuPanel::OnLevelLoadingFinished(KeyValues *pKvData)
{
    m_bLevelLoading = false;

    m_pLoadingScreen->Deactivate();
}

void CBaseMenuPanel::OnLoadProgress(float percent)
{
    m_pLoadingScreen->ProgressUpdate(percent);
}

bool CBaseMenuPanel::IsInLoading() const
{
    return m_bLevelLoading;
}

void CBaseMenuPanel::OnGameUIActivated()
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

        m_bEverActivated = true;
    }

    m_pMainMenu->SetVisible(true);

    if (GameUIUtil::IsInLevel())
    {
        OnCommand("OpenPauseMenu");
    }
}

void CBaseMenuPanel::OnGameUIHidden()
{
    m_pMainMenu->SetVisible(false);
}

Panel* CBaseMenuPanel::GetMainMenu()
{
    return m_pMainMenu;
}

void CBaseMenuPanel::SetMenuAlpha(int alpha)
{
    m_pMainMenu->SetAlpha(alpha);
}

void CBaseMenuPanel::UpdateBackgroundState()
{
    if (GameUIUtil::IsInLevel())
    {
        SetBackgroundRenderState(BACKGROUND_LEVEL);
    }
    else if (GameUIUtil::IsInBackgroundLevel() && !m_bLevelLoading)
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
    bool bIsInLevel = GameUIUtil::IsInLevel();
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

void CBaseMenuPanel::DrawBackgroundImage()
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

void CBaseMenuPanel::OnThink()
{
    BaseClass::OnThink();

    UpdateBackgroundState();
    
    if (ipanel())
    {
        int screenWide, screenTall;
        engine->GetScreenSize(screenWide, screenTall);
        SetBounds(0, 0, screenWide, screenTall);
    }
}

void CBaseMenuPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_flFrameFadeInTime = Q_atof(pScheme->GetResourceString("Frame.TransitionEffectTime"));

    // work out current focus - find the topmost panel
    SetBgColor(Color(0, 0, 0, 0));

    m_BackdropColor = pScheme->GetColor("mainmenu.backdrop", Color(0, 0, 0, 128));

    int screenWide, screenTall;
    surface()->GetScreenSize(screenWide, screenTall);
    float aspectRatio = (float) screenWide / (float) screenTall;
    bool bIsWidescreen = aspectRatio >= 1.5999f;

    // work out which background image to use
    // pc uses blurry backgrounds based on the background level
    char background[MAX_PATH];
    engine->GetMainMenuBackgroundName(background, sizeof(background));

    char filename[MAX_PATH];
    Q_snprintf(filename, sizeof(filename), "console/%s%s", background, (bIsWidescreen ? "_widescreen" : ""));

    if (m_iBackgroundImageID)
    {
        surface()->DestroyTextureID(m_iBackgroundImageID);
    }
    m_iBackgroundImageID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iBackgroundImageID, filename, false, false);

    // load the loading icon
    if (m_iLoadingImageID)
    {
        surface()->DestroyTextureID(m_iLoadingImageID);
    }
    m_iLoadingImageID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iLoadingImageID, "console/startup_loading", false, false);
}

void CBaseMenuPanel::PaintBackground()
{
    if (!GameUIUtil::IsInLevel() || m_pLoadingScreen->IsVisible())
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

void CBaseMenuPanel::RunEngineCommand(const char* command)
{
    engine->ClientCmd_Unrestricted(command);
}

void CBaseMenuPanel::RunMenuCommand(const char* command)
{
    if (FStrEq(command, "OpenGameMenu"))
    {
        if (m_pMainMenu)
        {
            // MOM_TODO: Do we even need to do this?
            PostMessage(m_pMainMenu, new KeyValues("Command", "command", "Open"));
        }
    }
    else if (FStrEq(command, "Quit") || FStrEq(command, "QuitNoConfirm"))
    {
        // hide everything while we quit
        SetVisible(false);
        surface()->RestrictPaintToSinglePanel(GetVPanel());
        engine->ClientCmd_Unrestricted("quit\n");
    }
    else if (FStrEq(command, "ResumeGame"))
    {
        engine->ExecuteClientCmd("gameui_hide");
    }
    else if (FStrEq(command, "Disconnect"))
    {
        engine->ClientCmd_Unrestricted("disconnect");
    }
    else if (FStrEq(command, "DisconnectNoConfirm"))
    {
        // Leave our current session, if we have one
    }
    else if (FStrEq(command, "ReleaseModalWindow"))
    {
        surface()->RestrictPaintToSinglePanel(NULL);
    }
    else if (Q_stristr(command, "engine "))
    {
        const char *engineCMD = Q_strstr(command, "engine ") + Q_strlen("engine ");
        if (Q_strlen(engineCMD) > 0)
        {
            engine->ClientCmd_Unrestricted(engineCMD);
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

bool CBaseMenuPanel::RequestInfo(KeyValues* data)
{
    if (FStrEq(data->GetName(), "menu_visible"))
    {
        data->SetBool("response", m_pMainMenu->IsVisible());
        return true;
    }

    return BaseClass::RequestInfo(data);
}

void CBaseMenuPanel::SetBackgroundRenderState(EBackgroundState state)
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
