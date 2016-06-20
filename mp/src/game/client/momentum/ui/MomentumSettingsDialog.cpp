#include "cbase.h"

#include "IMomentumSettingsPanel.h"
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/pch_vgui_controls.h>

#include "momentum/mom_shareddefs.h"
#include "tier0/memdbgon.h"

using namespace vgui;

class CMomentumSettingsPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CMomentumSettingsPanel, vgui::Frame);
    // CMomentumSettingsPanel : This Class / vgui::Frame : BaseClass

    CMomentumSettingsPanel(VPANEL parent); // Constructor
    ~CMomentumSettingsPanel(){};           // Destructor
    void Activate() override;

  protected:
    // VGUI overrides:
    void OnTick() override;
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel);
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges") { m_pApplyButton->SetEnabled(false); }

  private:
    void LoadSettings();
    // Other used VGUI control Elements:
    ComboBox *m_pSpeedometerUnits, *m_pSyncType, *m_pSyncColorize;

    CvarToggleCheckButton<ConVarRef> *m_pSpeedometerShow, *m_pSpeedometerShowLastJump, *m_pSpeedometerShowVerticalVel,
        *m_pSpeedometerColorize, *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion;

    Button *m_pApplyButton;
};

// Constuctor: Initializes the Panel
CMomentumSettingsPanel::CMomentumSettingsPanel(vgui::VPANEL parent) : BaseClass(nullptr, "CMomentumSettingsPanel")
{
    SetParent(parent);

    SetProportional(true);
    LoadControlSettings("resource/ui/MomentumSettingsPanel.res");

    SetPaintBackgroundType(1);
    SetRoundedCorners(PANEL_ROUND_CORNER_ALL);
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetMinimumSize(500, 500);

    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMoveable(true);
    SetVisible(false);

    m_pSpeedometerUnits = FindControl<ComboBox>("SpeedoUnits");
    m_pSpeedometerUnits->SetNumberOfEditLines(3);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSyncType = FindControl<ComboBox>("SyncType");
    m_pSyncType->SetNumberOfEditLines(2);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = FindControl<ComboBox>("SyncColorize");
    m_pSyncColorize->SetNumberOfEditLines(3);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

    m_pSpeedometerShow = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShow");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerShowLastJump = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShowJump");
    m_pSpeedometerShowLastJump->AddActionSignalTarget(this);

    m_pSpeedometerShowVerticalVel = FindControl<CvarToggleCheckButton<ConVarRef>>("ShowSpeedoHvel");
    m_pSpeedometerShowVerticalVel->AddActionSignalTarget(this);

    m_pSpeedometerColorize = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShowColor");
    m_pSpeedometerColorize->AddActionSignalTarget(this);

    m_pSyncShow = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShow");
    m_pSyncShow->AddActionSignalTarget(this);

    m_pSyncShowBar = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShowBar");
    m_pSyncShowBar->AddActionSignalTarget(this);

    m_pButtonsShow = FindControl<CvarToggleCheckButton<ConVarRef>>("ButtonsShow");
    m_pButtonsShow->AddActionSignalTarget(this);
    // MOM_TODO: have one for hud_versionwarn?

    m_pApplyButton = FindControl<Button>("ApplyButton");
    m_pApplyButton->AddActionSignalTarget(this);

    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShow);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShowLastJump);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShowVerticalVel);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerColorize);
    m_pApplyButton->AddActionSignalTarget(m_pSyncShow);
    m_pApplyButton->AddActionSignalTarget(m_pSyncShowBar);
    m_pApplyButton->AddActionSignalTarget(m_pButtonsShow);

    SetScheme("SourceScheme");
    //Okay so this is gross but ComboBoxes only use the "Default" font for the text inside them. IDK why.
    //Maybe in the future, we can update the scheme to not have grossly large default fonts (in ClientScheme)
    HFont comboBoxFont = scheme()->GetIScheme(scheme()->GetScheme("SourceScheme"))->GetFont("DefaultVerySmall", true);
    m_pSyncType->SetFont(comboBoxFont);
    m_pSyncColorize->SetFont(comboBoxFont);
    m_pSpeedometerUnits->SetFont(comboBoxFont);

    LoadSettings();

    ivgui()->AddTickSignal(GetVPanel());
}

// Class: CMyPanelInterface Class. Used for construction.
class CMomentumSettingsPanelInterface : public MomentumSettingsPanel
{
  private:
    CMomentumSettingsPanel *settings_panel;

  public:
    CMomentumSettingsPanelInterface() { settings_panel = nullptr; }
    ~CMomentumSettingsPanelInterface() { settings_panel = nullptr; }
    void Create(vgui::VPANEL parent) override { settings_panel = new CMomentumSettingsPanel(parent); }
    void Destroy() override
    {
        if (settings_panel)
        {
            settings_panel->SetParent(nullptr);
            delete settings_panel;
        }
    }
    void Activate(void) override
    {
        if (settings_panel)
        {
            settings_panel->Activate();
        }
    }
    void Close() override
    {
        if (settings_panel)
        {
            settings_panel->Close();
        }
    }
};
static CMomentumSettingsPanelInterface g_SettingsPanel;
MomentumSettingsPanel *momentum_settings = static_cast<MomentumSettingsPanel *>(&g_SettingsPanel);

CON_COMMAND_F(mom_settings_show, "Shows the settings panel.\n",
              FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN)
{
    momentum_settings->Activate();
}

// Combobox changed a value
void CMomentumSettingsPanel::OnTextChanged(vgui::Panel *panel)
{
    ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"), sync_color("mom_strafesync_colorize");

    units.SetValue(m_pSpeedometerUnits->GetActiveItem() + 1);
    sync_type.SetValue(m_pSyncType->GetActiveItem() + 1); // Sync type needs +1 added to it before setting convar!
    sync_color.SetValue(m_pSyncColorize->GetActiveItem());
}

void CMomentumSettingsPanel::LoadSettings()
{
    ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"), sync_color("mom_strafesync_colorize");
    m_pSpeedometerUnits->ActivateItemByRow(units.GetInt() - 1);
    m_pSyncType->ActivateItemByRow(sync_type.GetInt() - 1);
    m_pSyncColorize->ActivateItemByRow(sync_color.GetInt());
}

void CMomentumSettingsPanel::OnTick()
{
    BaseClass::OnTick();
    GetAnimationController()->UpdateAnimations(system()->GetFrameTime());
}

void CMomentumSettingsPanel::Activate()
{
    LoadSettings();
    BaseClass::Activate();
}

void CMomentumSettingsPanel::OnCheckboxChecked(vgui::Panel *panel)
{
    m_pApplyButton->SetEnabled(true);

    CheckButton *button = dynamic_cast<CheckButton *>(panel);
    if (button)
    {
        if (!Q_stricmp(button->GetName(), "SpeedoShow"))
        {
            m_pSpeedometerShowLastJump->SetEnabled(button->IsSelected());
            m_pSpeedometerShowVerticalVel->SetEnabled(button->IsSelected());
            m_pSpeedometerUnits->SetEnabled(button->IsSelected());
            m_pSpeedometerColorize->SetEnabled(button->IsSelected());
        }
        else if (!Q_stricmp(button->GetName(), "SyncShow"))
        {
            m_pSyncType->SetEnabled(button->IsSelected());
            m_pSyncShowBar->SetEnabled(button->IsSelected());
            m_pSyncColorize->SetEnabled(button->IsSelected());
        }
    }
}