#include "cbase.h"

#include "MainMenu.h"
#include "MainMenuButton.h"
#include "gameui/BaseMenuPanel.h"
#include "gameui/GameUIUtil.h"

#include "vgui/ISurface.h"

#include "filesystem.h"
#include "KeyValues.h"
#include <steam/isteamuser.h>

#include "mom_shareddefs.h"
#include "fmtstr.h"

#include "vgui/ILocalize.h"

#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Tooltip.h"

#include "controls/ModelPanel.h"
#include "controls/UserComponent.h"
#include "drawer/MenuDrawer.h"

#include "viewpostprocess.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static MainMenu *g_pMainMenu = nullptr;

CON_COMMAND(reload_menu, "Reloads the main menu\n")
{
    g_pMainMenu->CreateMenu();
}

CON_COMMAND(reload_menu_controls, "Reloads controls for the menu\n")
{
    g_pMainMenu->PostMessage(g_pMainMenu, new KeyValues("ReloadControls"));
    g_pMainMenu->InvalidateLayout();
}

static ConVar mom_menu_model_path("mom_menu_model_path", "models/custom_props/hd_logo.mdl", FCVAR_ARCHIVE, "Changes the model used in the main menu model panel.\n", [](IConVar *pVar, const char *pOldVal, float fOldVal)
{
   g_pMainMenu->SetModelPanelModel(ConVarRef(pVar).GetString()); 
});

static MAKE_TOGGLE_CONVAR_C(mom_menu_model_enable, "1", FCVAR_ARCHIVE, "Toggles the visibility of the main menu model panel. 0 = OFF, 1 = ON\n", [](IConVar *pVar, const char *pOldVal, float fOldVal)
{
   g_pMainMenu->SetModelPanelEnabled(ConVarRef(pVar).GetBool());
});

static MAKE_CONVAR_C(mom_menu_model_rotation_speed, "10", FCVAR_ARCHIVE, "Controls how fast the main menu model spins.\n", -1000, 1000, [](IConVar *pVar, const char *pOldVal, float fOldVal)
{
   g_pMainMenu->SetModelPanelRotationSpeed(ConVarRef(pVar).GetInt()); 
});

MainMenu::MainMenu(CBaseMenuPanel *pParent) : BaseClass(pParent, "MainMenu")
{
    m_pBasePanel = pParent;
    g_pMainMenu = this;

    MakePopup(false);
    SetProportional(true);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    AddActionSignalTarget(this);
    SetZPos(0);

    // Set our initial size
    int screenWide, screenTall;
    surface()->GetScreenSize(screenWide, screenTall);
    SetBounds(0, 0, screenWide, screenTall);

    // Listen for game events
    ListenForGameEvent("lobby_leave");
    ListenForGameEvent("lobby_join");
    ListenForGameEvent("spec_start");
    ListenForGameEvent("spec_stop");
    ListenForGameEvent("site_auth");

    if (!GetAnimationController()->SetScriptFile(GetVPanel(), "scripts/HudAnimations.txt"))
        AssertMsg(0, "Couldn't load the animations!");

    SetScheme(scheme()->LoadSchemeFromFile("resource/schememainmenu.res", "SchemeMainMenu"));

    m_bFocused = true;
    m_bNeedSort = false;
    m_nSortFlags = FL_SORT_SHARED;

    Q_wcsncpy(m_logoLeft, g_pVGuiLocalize->FindSafe("#MOM_Momentum"), sizeof(m_logoLeft));
    Q_wcsncpy(m_logoRight, g_pVGuiLocalize->FindSafe("#GameUI2_LogoRight"), sizeof(m_logoRight));

    m_pLogoImage = nullptr;

    CreateMenu();

    m_pMenuDrawer = new MenuDrawerPanel(this);

    m_pUserComponent = new UserComponent(this);
    m_pUserComponent->SetClickable(false);
    m_pUserComponent->AddActionSignalTarget(this);
    if (SteamUser())
    {
        m_pUserComponent->SetUser(SteamUser()->GetSteamID().ConvertToUint64());
    }

    m_pVersionLabel = new Button(this, "VersionLabel", "v" MOM_CURRENT_VERSION, this, "ShowVersion");
    m_pVersionLabel->SetPaintBackgroundEnabled(false);
    m_pVersionLabel->SetPaintBorderEnabled(false);
    m_pVersionLabel->SetAutoWide(true);
    m_pVersionLabel->SetAutoTall(true);
    
    m_pModelPanel = new CRenderPanel(this, "MainMenuModel");
    m_pModelPanel->SetVisible(mom_menu_model_enable.GetBool());
    m_pModelPanel->SetPaintBackgroundEnabled(false);
    m_pModelPanel->SetPaintBorderEnabled(true);
    m_pModelPanel->SetShouldAutoRotateModel(true);
    m_pModelPanel->SetZPos(-50);
    m_pModelPanel->SetAllowedDragModes(RDRAG_ROTATE);
    m_pModelPanel->SetAllowedRotateModes(ROTATE_YAW);
    m_pModelPanel->SetCameraDefaults(40, 0.0f, 180.0f, 200);
    m_pModelPanel->SetDefaultLightAngle(QAngle(90.0f, 0, 0));
    m_pModelPanel->SetAutoModelRotationSpeed(QAngle(0, mom_menu_model_rotation_speed.GetInt(), 0));
    m_pModelPanel->LoadModel(mom_menu_model_path.GetString());

    // MOM_TODO: LoadControlSettings("resource/ui/MainMenuLayout.res");

    m_PostProcessParameters.m_flParameters[PPPN_VIGNETTE_START] = 0.0f;
    m_PostProcessParameters.m_flParameters[PPPN_VIGNETTE_END] = 0.0f;
    m_PostProcessParameters.m_flParameters[PPPN_VIGNETTE_BLUR_STRENGTH] = 1.0f;

    CheckVersion();

    MakeReadyForUse();
    InvalidateLayout(true);
    RequestFocus();
}

MainMenu::~MainMenu()
{
}

void MainMenu::OnThink()
{
    BaseClass::OnThink();

    surface()->MovePopupToBack(GetVPanel());
}

bool MainMenu::IsVisible()
{
    return !m_pBasePanel->IsInLoading() && BaseClass::IsVisible();
}

void MainMenu::OnUserComponentClicked()
{
    m_pMenuDrawer->OpenDrawerTo(DRAWER_TAB_USER);
}

void MainMenu::OnMenuButtonCommand(KeyValues* pKv)
{
    const char *pNormalCommand = pKv->GetString("command", nullptr);
    const char *pEngineCommand = pKv->GetString("EngineCommand", nullptr);
    if (pNormalCommand)
    {
        m_pBasePanel->RunMenuCommand(pNormalCommand);
    }
    else if (pEngineCommand)
    {
        CCommand args;
        args.Tokenize(pEngineCommand);

        ConCommand *pCommand = g_pCVar->FindCommand(args[0]);
        if (pCommand) // can we directly call this command?
        {
            pCommand->Dispatch(args);
        }
        else // fallback to old code
        {
            engine->ClientCmd(pEngineCommand);
        }
    }
}

void MainMenu::OnCommand(char const *cmd)
{
    if (FStrEq(cmd, "ShowVersion"))
    {
        m_pMenuDrawer->OpenDrawerTo(DRAWER_TAB_CHANGELOG);
        GetAnimationController()->StartAnimationSequence(this, "VersionPulseStop");
    }

    m_pBasePanel->RunMenuCommand(cmd);

    BaseClass::OnCommand(cmd);
}

void MainMenu::FireGameEvent(IGameEvent* event)
{
    if (FStrEq(event->GetName(), "lobby_leave"))
    {
        m_pMenuDrawer->OnLobbyLeave();
    }
    else if (FStrEq(event->GetName(), "lobby_join"))
    {
        m_pMenuDrawer->OnLobbyEnter();
    }
    else if (FStrEq(event->GetName(), "spec_start"))
    {
        m_pMenuDrawer->OnSpecStart();
    }
    else if (FStrEq(event->GetName(), "spec_stop"))
    {
        m_pMenuDrawer->OnSpecStop();
    }
    else if (FStrEq(event->GetName(), "site_auth"))
    {
        m_pMenuDrawer->OnSiteAuth();
    }

    InvalidateLayout();
}

void MainMenu::CreateMenu()
{
    m_pButtons.PurgeAndDeleteElements();

    KeyValuesAD datafile("MainMenu");
    datafile->UsesEscapeSequences(true);
    if (datafile->LoadFromFile(g_pFullFileSystem, "resource/mainmenu.res", "GAME"))
    {
        FOR_EACH_SUBKEY(datafile, dat)
        {
            MainMenuButton *button = new MainMenuButton(this);
            button->SetName(dat->GetName());
            button->SetPriority(dat->GetInt("priority", 1));
            button->SetText(dat->GetString("text", "no text"));
            button->SetBlank(dat->GetBool("blank"));
            button->SetEnabled(dat->GetBool("enabled"));

            button->SetCommand(dat->GetString("command", nullptr));
            button->SetEngineCommand(dat->GetString("EngineCommand", nullptr));

            // a bit hacky, show informational tooltip
            if (!Q_strcasecmp("ShowMapSelectionPanel", dat->GetString("EngineCommand", "")))
            {
                button->GetTooltip()->SetText("The online map selector is not available in this build, use the map command from the console instead!");
            }

            const char *specifics = dat->GetString("specifics", "shared");
            if (!Q_strcasecmp(specifics, "ingame"))
            {
                button->SetButtonType(IN_GAME);
            }
            else if (!Q_strcasecmp(specifics, "mainmenu"))
            {
                button->SetButtonType(MAIN_MENU);
            }

            m_pButtons.AddToTail(button);
        }
    }
}

int32 __cdecl ButtonsPositionBottom(MainMenuButton *const *s1, MainMenuButton *const *s2)
{
    return ((*s1)->GetPriority() > (*s2)->GetPriority());
}

int32 __cdecl ButtonsPositionTop(MainMenuButton *const *s1, MainMenuButton *const *s2)
{
    return ((*s1)->GetPriority() < (*s2)->GetPriority());
}

void MainMenu::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_iButtonsSpace = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.Space")));
    m_iButtonsOffsetX = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.OffsetX")));
    m_iButtonsOffsetY = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Buttons.OffsetY")));
    m_iLogoOffsetX = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.OffsetX")));
    m_iLogoOffsetY = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.OffsetY")));
    m_cLogoLeft = GetSchemeColor("MainMenu.Logo.Left", pScheme);
    m_cLogoRight = GetSchemeColor("MainMenu.Logo.Right", pScheme);
    m_bLogoAttachToMenu = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.AttachToMenu"));
    m_bLogoText = Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Text"));
    if (!m_bLogoText)
    {
        if (m_pLogoImage)
        {
            m_pLogoImage->EvictImage();
        }
        else
        {
            m_pLogoImage = new ImagePanel(this, "GameLogo");
        }

        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        m_iLogoWidth = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width")));
        m_iLogoHeight = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height")));
        // Size and pos are handled in Paint()
    }

    m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    const auto hVersionLabelFont = pScheme->GetFont("MainMenu.VersionLabel.Font", true);
    if (m_pVersionLabel && hVersionLabelFont)
    {
        m_pVersionLabel->SetFont(hVersionLabelFont);
    }

    Q_strncpy(m_pszMenuOpenSound, pScheme->GetResourceString("MainMenu.Sound.Open"), sizeof(m_pszMenuOpenSound));
    Q_strncpy(m_pszMenuCloseSound, pScheme->GetResourceString("MainMenu.Sound.Close"), sizeof(m_pszMenuCloseSound));
}

inline MainMenuButton *GetNextVisible(CUtlVector<MainMenuButton *> *vec, int start)
{
    for (int i = start + 1; i < vec->Count(); i++)
    {
        MainMenuButton *pButton = vec->Element(i);
        if (pButton->IsVisible())
            return pButton;
    }
    return nullptr;
}

void MainMenu::DrawMainMenu()
{
    for (int8 i = 0; i < m_pButtons.Count(); i++)
    {
        MainMenuButton *pButton = m_pButtons[i];
        switch (pButton->GetButtonType())
        {
        default:
        case SHARED:
            pButton->SetVisible(GameUIUtil::IsInLevel() || GameUIUtil::IsInMenu());
            break;
        case IN_GAME:
            pButton->SetVisible(GameUIUtil::IsInLevel());
            break;
        case MAIN_MENU:
            pButton->SetVisible(GameUIUtil::IsInMenu());
            break;
        }
    }

    if (GameUIUtil::IsInLevel())
    {
        SetPostProcessParams(&m_PostProcessParameters, true);

        m_nSortFlags &= ~FL_SORT_MENU;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_INGAME));
        m_nSortFlags |= FL_SORT_INGAME;
    }
    else if (GameUIUtil::IsInMenu())
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
        MainMenuButton *pNextVisible = GetNextVisible(&m_pButtons, i);
        if (pNextVisible)
        {
            int32 x0, y0;
            pNextVisible->GetPos(x0, y0);
            m_pButtons[i]->SetPos(m_iButtonsOffsetX, y0 - (m_pButtons[i]->GetHeight() + m_iButtonsSpace));
        }
        else
        {
            m_pButtons[i]->SetPos(m_iButtonsOffsetX,
                ScreenHeight() - (m_iButtonsOffsetY + m_pButtons[i]->GetHeight()));
        }
    }
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
    }
}

void MainMenu::CheckVersion()
{
    KeyValuesAD pVersionKV("Version");
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

void MainMenu::PerformLayout()
{
    BaseClass::PerformLayout();

    int screenWide, screenTall;
    surface()->GetScreenSize(screenWide, screenTall);

    m_pVersionLabel->SetPos(screenWide - m_pVersionLabel->GetWide() - m_pMenuDrawer->GetDrawerButtonWidth() - m_iButtonsOffsetX, GetScaledVal(2));

    m_pUserComponent->SetBounds(screenWide - m_pUserComponent->GetWide() - m_iButtonsOffsetX - m_pMenuDrawer->GetDrawerButtonWidth(),
                                screenTall - m_pUserComponent->GetTall() - m_iButtonsOffsetY,
                                GetScaledVal(200),
                                GetScaledVal(50));

    m_pModelPanel->SetBounds((screenWide - m_pMenuDrawer->GetDrawerButtonWidth() - m_iButtonsOffsetX) / 2, 0, 
                             (screenWide - m_pMenuDrawer->GetDrawerButtonWidth() + m_iButtonsOffsetX) / 2, screenTall);
}

void MainMenu::SetModelPanelEnabled(bool bEnabled)
{
    m_pModelPanel->SetVisible(bEnabled);
}

void MainMenu::SetModelPanelModel(const char *pModel)
{
    m_pModelPanel->LoadModel(pModel);
}

void MainMenu::SetModelPanelRotationSpeed(int iSpeed)
{
    m_pModelPanel->SetAutoModelRotationSpeed(QAngle(0, iSpeed, 0));
}

void MainMenu::SetVisible(bool state)
{
    if (!state)
        SetPostProcessParams(&m_PostProcessParameters, false);

    BaseClass::SetVisible(state);
}

void MainMenu::OnConfirmDisconnect()
{
    engine->DisconnectInternal();
}

void MainMenu::OnConfirmMapChange()
{
    engine->ClientCmd_Unrestricted("__map_change_ok 1\n");
}

void MainMenu::OnConfirmQuit()
{
    engine->ClientCmd_Unrestricted("__game_quit_ok 1; quit\n");
}