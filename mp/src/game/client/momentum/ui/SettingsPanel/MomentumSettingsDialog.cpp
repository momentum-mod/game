#include "cbase.h"

#include "IMomentumSettingsPanel.h"
#include "SettingsPage.h"
#include "HudSettingsPage.h"
#include "GameplaySettingsPage.h"
#include "ComparisonsSettingsPage.h"
#include "AppearanceSettingsPage.h"
#include "OnlineSettingsPage.h"
#include "RocketJumpSettingsPage.h"
#include "StickyJumpSettingsPage.h"
#include "CrosshairSettingsPage.h"
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/pch_vgui_controls.h>
#include "clientmode.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class CMomentumSettingsDialog : public PropertyDialog
{
    DECLARE_CLASS_SIMPLE(CMomentumSettingsDialog, PropertyDialog);

    CMomentumSettingsDialog(VPANEL parent);
    ~CMomentumSettingsDialog();

    void OnClose() OVERRIDE;
    void Activate() OVERRIDE;

  protected:
    // VGUI overrides:
    void OnThink() OVERRIDE;

  private:
    SettingsPage *m_pHudSettings, *m_pControlsSettings, *m_pCompareSettings, *m_pAppearanceSettings,
     *m_pOnlineSettings, *m_pRocketJumpSettings, *m_pStickyJumpSettings, *m_pCrosshairSettings;
};

CMomentumSettingsDialog::CMomentumSettingsDialog(VPANEL parent) : BaseClass(nullptr, "CMomentumSettingsPanel")
{
    SetParent(parent);
    SetProportional(true);
    SetScheme("SourceScheme");
    LoadControlSettings("resource/ui/SettingsPanel_Base.res");
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetMinimumSize(GetScaledVal(400), GetScaledVal(260));
    SetSize(GetScaledVal(400), GetScaledVal(260));
    SetApplyButtonVisible(true);
    SetTitleBarVisible(true);
    SetMenuButtonResponsive(false);
    SetSysMenu(nullptr);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetMoveable(true);
    SetVisible(false);
    SetSizeable(true);
    GetPropertySheet()->SetSmallTabs(true);

    //Create the pages here
    m_pControlsSettings = new GameplaySettingsPage(this);
    m_pHudSettings = new HudSettingsPage(this);
    m_pCompareSettings = new ComparisonsSettingsPage(this);
    m_pAppearanceSettings = new AppearanceSettingsPage(this);
    m_pOnlineSettings = new OnlineSettingsPage(this);
    m_pRocketJumpSettings = new RocketJumpSettingsPage(this);
    m_pStickyJumpSettings = new StickyJumpSettingsPage(this);
    m_pCrosshairSettings = new CrosshairSettingsPage(this);

    //Note: we're adding the scroll panels here, because we want to be able to scroll.
    AddPage(m_pControlsSettings->GetScrollPanel(), "#MOM_Settings_Tab_Gameplay");
    AddPage(m_pHudSettings->GetScrollPanel(), "#MOM_Settings_Tab_HUD");
    AddPage(m_pCompareSettings->GetScrollPanel(), "#MOM_Settings_Tab_Comparisons");
    AddPage(m_pAppearanceSettings->GetScrollPanel(), "#MOM_Settings_Tab_Appearance");
    AddPage(m_pOnlineSettings->GetScrollPanel(), "#MOM_Settings_Tab_Online");
    AddPage(m_pRocketJumpSettings->GetScrollPanel(), "#MOM_Settings_Tab_RJ");
    AddPage(m_pStickyJumpSettings->GetScrollPanel(), "#MOM_Settings_Tab_SJ");
    AddPage(m_pCrosshairSettings->GetScrollPanel(), "#MOM_Settings_Tab_Crosshair");
}

CMomentumSettingsDialog::~CMomentumSettingsDialog()
{
}

void CMomentumSettingsDialog::OnClose()
{
    BaseClass::OnClose();

    //Let the comparisons settings page/Replay model panel know so they can fade too
    if (GetActivePage())
        PostMessage(GetActivePage(), new KeyValues("OnMainDialogClosed"));
}

void CMomentumSettingsDialog::Activate()
{
    BaseClass::Activate();

    MoveToCenterOfScreen();
    //Let the comparisons settings page/replay model panel know so they can show back up
    if (GetActivePage())
        PostMessage(GetActivePage(), new KeyValues("OnMainDialogShow"));
}

class CMomentumSettingsPanelInterface : public IMomentumSettingsPanel
{
  private:
    CMomentumSettingsDialog *settings_panel;

  public:
    CMomentumSettingsPanelInterface() { settings_panel = nullptr; }
    ~CMomentumSettingsPanelInterface() { settings_panel = nullptr; }
    void Create(VPANEL parent) OVERRIDE { settings_panel = new CMomentumSettingsDialog(parent); }
    void Destroy() OVERRIDE
    {
        if (settings_panel)
        {
            settings_panel->SetParent(nullptr);
            settings_panel->DeletePanel();
        }
        settings_panel = nullptr;
    }
    void Activate(void) OVERRIDE
    {
        if (settings_panel)
        {
            settings_panel->Activate();
        }
    }
    void Close() OVERRIDE
    {
        if (settings_panel)
        {
            settings_panel->Close();
        }
    }
};

//Expose this interface to the DLL
static CMomentumSettingsPanelInterface g_SettingsPanel;
IMomentumSettingsPanel *momentum_settings = static_cast<IMomentumSettingsPanel *>(&g_SettingsPanel);

CON_COMMAND_F(mom_settings_show, "Shows the settings panel.\n",
              FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN)
{
    momentum_settings->Activate();
}

void CMomentumSettingsDialog::OnThink()
{
    BaseClass::OnThink();

    if (g_pClientMode->GetViewport() && g_pClientMode->GetViewportAnimationController())
        g_pClientMode->GetViewportAnimationController()->UpdateAnimations(system()->GetFrameTime());
}
