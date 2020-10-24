#include "cbase.h"

#include "MomentumSettingsDialog.h"

#include "SettingsPanel.h"
#include "InputSettingsPanel.h"
#include "AudioSettingsPanel.h"
#include "VideoSettingsPanel.h"
#include "OnlineSettingsPanel.h"
#include "GameplaySettingsPanel.h"
#include "HUDSettingsPanel.h"

#include "vgui/ISystem.h"

#include "vgui_controls/Frame.h"
#include "vgui_controls/ToggleButton.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/ScrollableEditablePanel.h"

#include "clientmode.h"
#include "ienginevgui.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class GroupPanel : public Panel
{
    DECLARE_CLASS_SIMPLE(GroupPanel, vgui::Panel);

    GroupPanel(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
    {
        m_iSpacingX = GetScaledVal(10);
    }

    void AddPanelToGroup(Panel *pPanel)
    {
        PHandle handle;
        handle = pPanel;
        m_vecPanels.AddToTail(handle);
    }

    void PerformLayout() override
    {
        BaseClass::PerformLayout();

        int wide, tall;
        GetSize(wide, tall);

        int totalWide = 0;
        FOR_EACH_VEC(m_vecPanels, i)
        {
            totalWide += m_vecPanels[i]->GetWide();
        }

        totalWide += m_iSpacingX * (m_vecPanels.Count() - 1);

        int x = (wide / 2) - (totalWide / 2);
        FOR_EACH_VEC(m_vecPanels, i)
        {
            auto pPanel = m_vecPanels[i];

            pPanel->SetPos(x, GetYPos() + (tall / 2) - (pPanel->GetTall() / 2));
            x += m_vecPanels[i]->GetWide() + m_iSpacingX;
        }
    }

private:
    CUtlVector<PHandle> m_vecPanels;

    int m_iSpacingX;
};

static MAKE_TOGGLE_CONVAR(mom_settings_remember_tab, "1", FCVAR_ARCHIVE, "Toggles remembering the last opened tab in settings. 0 = OFF, 1 = ON.\n");
static CMomentumSettingsDialog *g_pSettingsDialog = nullptr;

CMomentumSettingsDialog::CMomentumSettingsDialog() : BaseClass(nullptr, "SettingsDialog")
{
    g_pSettingsDialog = this;

    m_pCurrentSettingsPage = nullptr;

    SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    SetProportional(true);

    const auto settingsScheme = scheme()->LoadSchemeFromFile("resource/SettingsScheme.res", "SettingsScheme");
    SetScheme(settingsScheme ? settingsScheme : scheme()->GetDefaultScheme());

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);
    SetDeleteSelfOnClose(false);

    SetCloseButtonVisible(true);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonVisible(false);
    SetMinimizeToSysTrayButtonVisible(false);

    SetTitleBarVisible(true);
    SetMenuButtonResponsive(false);
    SetSysMenu(nullptr);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetMoveable(false);
    SetSizeable(false);

    m_pButtonGroup = new GroupPanel(this, "ButtonGroup");

    m_pScrollableSettingsPanel = new ScrollableEditablePanel(this, nullptr, "CurrentSettings");

    m_pInputButton = new Button(this, "InputButton", "#MOM_Settings_Input", this, "Input");
    m_pButtonGroup->AddPanelToGroup(m_pInputButton);

    m_pAudioButton = new Button(this, "AudioButton", "#MOM_Settings_Audio", this, "Audio");
    m_pButtonGroup->AddPanelToGroup(m_pAudioButton);

    m_pVideoButton = new Button(this, "VideoButton", "#MOM_Settings_Video", this, "Video");
    m_pButtonGroup->AddPanelToGroup(m_pVideoButton);

    m_pOnlineButton = new Button(this, "OnlineButton", "#MOM_Settings_Online", this, "Online");
    m_pButtonGroup->AddPanelToGroup(m_pOnlineButton);

    m_pGameplayButton = new Button(this, "GameplayButton", "#MOM_Settings_Gameplay", this, "Gameplay");
    m_pButtonGroup->AddPanelToGroup(m_pGameplayButton);

    m_pHUDButton = new Button(this, "HUDButton", "#MOM_Settings_HUD", this, "HUD");
    m_pButtonGroup->AddPanelToGroup(m_pHUDButton);

    LoadControlSettings("resource/ui/settings/SettingsDialog.res");
    SetVisible(false);

    m_pInputPage = new InputSettingsPanel(m_pScrollableSettingsPanel, m_pInputButton);
    AddActionSignalTarget(m_pInputPage);
    m_pAudioPage = new AudioSettingsPanel(m_pScrollableSettingsPanel, m_pAudioButton);
    AddActionSignalTarget(m_pAudioPage);
    m_pVideoPage = new VideoSettingsPanel(m_pScrollableSettingsPanel, m_pVideoButton);
    AddActionSignalTarget(m_pVideoPage);
    m_pOnlinePage = new OnlineSettingsPanel(m_pScrollableSettingsPanel, m_pOnlineButton);
    AddActionSignalTarget(m_pOnlinePage);
    m_pGameplayPage = new GameplaySettingsPanel(m_pScrollableSettingsPanel, m_pGameplayButton);
    AddActionSignalTarget(m_pGameplayPage);
    m_pHUDPage = new HUDSettingsPanel(m_pScrollableSettingsPanel, m_pHUDButton);
    AddActionSignalTarget(m_pHUDPage);

    SetActivePanel(m_pInputPage);
}

CMomentumSettingsDialog::~CMomentumSettingsDialog()
{
    g_pSettingsDialog = nullptr;
}

void CMomentumSettingsDialog::Init()
{
    if (g_pSettingsDialog)
        return;

    g_pSettingsDialog = new CMomentumSettingsDialog;
}

void CMomentumSettingsDialog::OnClose()
{
    if (g_pClientMode->GetViewportAnimationController())
        g_pClientMode->GetViewportAnimationController()->CancelAllAnimations();

    engine->ClientCmd_Unrestricted("exec userconfig.cfg\nhost_writeconfig\n");

    PostActionSignal(new KeyValues("MainDialogClosed"));

    BaseClass::OnClose();
}

void CMomentumSettingsDialog::Activate()
{
    BaseClass::Activate();

    SetActivePanel(mom_settings_remember_tab.GetBool() ? m_pCurrentSettingsPage : m_pInputPage);
}

void CMomentumSettingsDialog::OnThink()
{
    BaseClass::OnThink();

    if (g_pClientMode->GetViewport() && g_pClientMode->GetViewportAnimationController())
        g_pClientMode->GetViewportAnimationController()->UpdateAnimations(system()->GetFrameTime());
}

void CMomentumSettingsDialog::OnReloadControls()
{
    BaseClass::OnReloadControls();

    if (g_pClientMode->GetViewportAnimationController())
        g_pClientMode->GetViewportAnimationController()->CancelAllAnimations();

    SetActivePanel(m_pCurrentSettingsPage);
}

void CMomentumSettingsDialog::OnCommand(const char *command)
{
    if (FStrEq(command, "Input"))
    {
        SetActivePanel(m_pInputPage);
    }
    else if (FStrEq(command, "Audio"))
    {
        SetActivePanel(m_pAudioPage);
    }
    else if (FStrEq(command, "Video"))
    {
        SetActivePanel(m_pVideoPage);
    }
    else if (FStrEq(command, "Online"))
    {
        SetActivePanel(m_pOnlinePage);
    }
    else if (FStrEq(command, "Gameplay"))
    {
        SetActivePanel(m_pGameplayPage);
    }
    else if (FStrEq(command, "HUD"))
    {
        SetActivePanel(m_pHUDPage);
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void CMomentumSettingsDialog::SetActivePanel(SettingsPanel *pPanel)
{
    if (m_pCurrentSettingsPage && m_pCurrentSettingsPage != pPanel)
    {
        m_pCurrentSettingsPage->OnPageHide();
        m_pCurrentSettingsPage->SetVisible(false);
    }

    m_pScrollableSettingsPanel->SetChild(pPanel);
    pPanel->SetVisible(true);

    m_pCurrentSettingsPage = pPanel;
    m_pCurrentSettingsPage->OnPageShow();
}

CON_COMMAND_F(mom_settings_show, "Shows the settings panel.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN)
{
    if (!g_pSettingsDialog)
    {
        CMomentumSettingsDialog::Init();
    }

    g_pSettingsDialog->DoModal();
}