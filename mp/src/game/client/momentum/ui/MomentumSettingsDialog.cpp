//The following include files are necessary to allow your  the panel .cpp to compile.
#include "cbase.h"
#include "IMomentumSettingsPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/pch_vgui_controls.h>
#include <vgui_controls/CvarToggleCheckButton.h>

#include "momentum/mom_shareddefs.h"
#include "tier0/memdbgon.h"

class CMomentumSettingsPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CMomentumSettingsPanel, vgui::Frame);
    //CMomentumSettingsPanel : This Class / vgui::Frame : BaseClass

    CMomentumSettingsPanel(vgui::VPANEL parent); 	// Constructor
    ~CMomentumSettingsPanel() {};				// Destructor
    void Activate() override;

protected:
    //VGUI overrides:
    void OnTick() override;
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
    MESSAGE_FUNC_PTR(OnCheckboxChecked, "CheckButtonChecked", panel);
    MESSAGE_FUNC(OnApplyChanges, "ApplyChanges")
    {
        m_pApplyButton->SetEnabled(false);
    }

private:
    void LoadSettings();
    //Other used VGUI control Elements:
    ComboBox *m_pSpeedometerUnits, *m_pSyncType, *m_pSyncColorize;

    CvarToggleCheckButton<ConVarRef> *m_pSpeedometerShow, *m_pSpeedometerShowLastJump, *m_pSpeedometerShowVerticalVel, *m_pSpeedometerColorize,
        *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow,
        *m_pShowVersion;

    Button *m_pApplyButton;

};

// Constuctor: Initializes the Panel
CMomentumSettingsPanel::CMomentumSettingsPanel(vgui::VPANEL parent)
    : BaseClass(nullptr, "CMomentumSettingsPanel")
{
    SetParent(parent);
    SetPaintBackgroundType(1);
    SetRoundedCorners(PANEL_ROUND_CORNER_ALL);
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetProportional(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMoveable(true);
    SetVisible(false);
    
    m_pSpeedometerUnits = new ComboBox(this, "SpeedoUnits", 3, false);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSyncType = new ComboBox(this, "SyncType", 2, false);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = new ComboBox(this, "SyncColorize", 3, false);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

    m_pSpeedometerShow = new CvarToggleCheckButton<ConVarRef>(this, "SpeedoShow", "#MOM_Settings_Speedometer_Show", "mom_speedometer", false);
    m_pSpeedometerShow->AddActionSignalTarget(this);
    m_pSpeedometerShowLastJump = new CvarToggleCheckButton<ConVarRef>(this, "SpeedoShowJump", "#MOM_Settings_Speedometer_Show_Jump", 
        "mom_speedometer_showlastjumpvel", false);
    m_pSpeedometerShowLastJump->AddActionSignalTarget(this);
    m_pSpeedometerShowVerticalVel = new CvarToggleCheckButton<ConVarRef>(this, "ShowSpeedoHvel", "#MOM_Settings_Speedometer_Show_Hvel",
        "mom_speedometer_hvel", false);
    m_pSpeedometerShowVerticalVel->AddActionSignalTarget(this);
    m_pSpeedometerColorize = new CvarToggleCheckButton<ConVarRef>(this, "SpeedoShowColor", "#MOM_Settings_Speedometer_Show_Color", 
        "mom_speedometer_colorize", false);
    m_pSpeedometerColorize->AddActionSignalTarget(this);

    m_pSyncShow = new CvarToggleCheckButton<ConVarRef>(this, "SyncShow", "#MOM_Settings_Sync_Show", "mom_strafesync_draw", false);
    m_pSyncShow->AddActionSignalTarget(this);
    m_pSyncShowBar = new CvarToggleCheckButton<ConVarRef>(this, "SyncShowBar", "#MOM_Settings_Sync_Show_Bar", "mom_strafesync_drawbar", false);
    m_pSyncShowBar->AddActionSignalTarget(this);

    m_pButtonsShow = new CvarToggleCheckButton<ConVarRef>(this, "ButtonsShow", "#MOM_Settings_Buttons_Show", "mom_showkeypresses", false);
    m_pButtonsShow->AddActionSignalTarget(this);
    //MOM_TODO: have one for hud_versionwarn? 

    m_pApplyButton = new Button(this, "ApplyButton", "#GameUI_Apply", this);
    m_pApplyButton->SetCommand("ApplyChanges");

    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShow);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShowLastJump);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerShowVerticalVel);
    m_pApplyButton->AddActionSignalTarget(m_pSpeedometerColorize);
    m_pApplyButton->AddActionSignalTarget(m_pSyncShow);
    m_pApplyButton->AddActionSignalTarget(m_pSyncShowBar);
    m_pApplyButton->AddActionSignalTarget(m_pButtonsShow);
    
    SetScheme("ClientScheme");
    //SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme"));

    LoadControlSettings("resource/ui/MomentumSettingsPanel.res");
    LoadSettings();

    ivgui()->AddTickSignal(GetVPanel());
}

//Class: CMomentumSettingsPanelInterface Class. Used for construction.
class CMomentumSettingsPanelInterface : public MomentumSettingsPanel
{
private:
    CMomentumSettingsPanel *settings_panel;
public:
    CMomentumSettingsPanelInterface()
    {
        settings_panel = nullptr;
    }
    ~CMomentumSettingsPanelInterface()
    {
        settings_panel = nullptr;
    }
    void Create(vgui::VPANEL parent)
    {
        settings_panel = new CMomentumSettingsPanel(parent);
    }
    void Destroy()
    {
        if (settings_panel)
        {
            settings_panel->SetParent(nullptr);
            delete settings_panel;
        }
    }
    void Activate(void)
    {
        if (settings_panel)
        {
            settings_panel->Activate();
        }
    }
    void Close()
    {
        if (settings_panel)
        {
            settings_panel->Close();
        }
    }
};
static CMomentumSettingsPanelInterface g_SettingsPanel;
MomentumSettingsPanel* momentum_settings = (MomentumSettingsPanel*) &g_SettingsPanel;

// MOM_TODO: This is prob not necessary here
ConVar cl_showsettingspanel("cl_showsettingspanel", "0", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE 
    | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN, "Sets the state of settings panel.\n");

CON_COMMAND(mom_settings_show, "Shows the settings.\n")
{
    momentum_settings->Activate();
}

//Combobox changed a value
void CMomentumSettingsPanel::OnTextChanged(vgui::Panel* panel)
{
    ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"), sync_color("mom_strafesync_colorize");
    
    units.SetValue(m_pSpeedometerUnits->GetActiveItem() + 1);
    sync_type.SetValue(m_pSyncType->GetActiveItem() + 1);//Sync type needs +1 added to it before setting convar!
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
    vgui::GetAnimationController()->UpdateAnimations(system()->GetFrameTime());
}

void CMomentumSettingsPanel::Activate()
{
    LoadSettings();
    BaseClass::Activate();
}

void CMomentumSettingsPanel::OnCheckboxChecked(vgui::Panel* panel)
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