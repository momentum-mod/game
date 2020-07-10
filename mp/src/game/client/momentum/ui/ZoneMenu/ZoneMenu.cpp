#include "cbase.h"

#include <usermessages.h>
#include "ZoneMenu.h"
#include "clientmode.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/Label.h>
#include "vgui_controls/CvarToggleCheckButton.h"

#include "fmtstr.h"

#include "tier0/memdbgon.h"

using namespace vgui;

C_MomZoneMenu *g_pZoneMenu = nullptr;

CON_COMMAND_F(mom_zone_showmenu, "Shows zoning menu", FCVAR_MAPPING)
{
    if (!g_pZoneMenu)
    {
        g_pZoneMenu = new C_MomZoneMenu();
    }
    g_pZoneMenu->Activate();
}

C_MomZoneMenu::C_MomZoneMenu() : Frame(g_pClientMode->GetViewport(), "ZoneMenu")
{
    usermessages->HookMessage("ZoneInfo", OnZoneInfoThunk);
    SetProportional(true);
    SetBounds(9, 96, 245, 135);
    m_bBindKeys = false;
    m_eZoneAction = ZONEACTION_NONE;

    m_pToggleZoneEdit = new CvarToggleCheckButton(this, "ToggleZoneEdit", "#MOM_ZoneMenu_ToggleEdit", "mom_zone_edit");
    m_pToggleZoneEdit->AddActionSignalTarget(this);

    m_pToggleUsePointMethod = new CvarToggleCheckButton(this, "TogglePointMethod", "#MOM_ZoneMenu_TogglePointMethod", "mom_zone_usenewmethod");
    m_pToggleUsePointMethod->AddActionSignalTarget(this);

    m_pCreateNewZoneButton = new Button(this, "CreateNewZoneButton", "#MOM_ZoneMenu_CreateNewZone", this, "CreateNewZone");
    m_pDeleteZoneButton = new Button(this, "DeleteZoneButton", "#MOM_ZoneMenu_DeleteZone", this, "DeleteZone");
    m_pEditZoneButton = new Button(this, "EditZoneButton", "#MOM_ZoneMenu_EditZone", this, "EditZone");
    m_pCancelZoneButton = new Button(this, "CancelZoneButton", "#MOM_ZoneMenu_CancelZone", this, "CancelZone");
    m_pSaveZonesButton = new Button(this, "SaveZonesButton", "#MOM_ZoneMenu_SaveZones", this, "SaveZones");

    m_pTrackNumberLabel = new Label(this, "TrackNumberLabel", "#MOM_ZoneMenu_TrackNumber");
    m_pTrackNumberEntry = new CvarTextEntry(this, "TrackNumberEntry", "mom_zone_track", 0);
    m_pTrackNumberEntry->SetAllowNumericInputOnly(true);

    m_pZoneNumberLabel = new Label(this, "ZoneNumberLabel", "#MOM_ZoneMenu_ZoneNumber");
    m_pZoneNumberEntry = new CvarTextEntry(this, "ZoneNumberEntry", "mom_zone_zonenum", 0);
    m_pZoneNumberEntry->SetAllowNumericInputOnly(true);

    m_pZoneTypeLabel = new Label(this, "ZoneTypeLabel", "#MOM_ZoneMenu_ZoneType");
    m_pZoneTypeCombo = new ComboBox(this, "ZoneTypeCombo", 7, false);
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_Auto", new KeyValues("vals", "zone_type", "auto"));
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_Start", new KeyValues("vals", "zone_type", "start"));
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_End", new KeyValues("vals", "zone_type", "end"));
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_Stage", new KeyValues("vals", "zone_type", "stage"));
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_Checkpoint", new KeyValues("vals", "zone_type", "cp"));
    m_pZoneTypeCombo->AddItem("#MOM_ZoneMenu_ZoneType_Trick", new KeyValues("vals", "zone_type", "trick"));
    m_pZoneTypeCombo->GetMenu()->SetTypeAheadMode(Menu::NO_TYPE_AHEAD_MODE); // Disable the annoying type ahead
    m_pZoneTypeCombo->ActivateItemByRow(0);

    m_pGridSizeLabel = new Label(this, "GridSizeLabel", "");
    m_pGridSizeSlider = new CvarSlider(this, "GridSizeSlider", "mom_zone_grid", 0, true);
    m_pGridSizeTextEntry = new CvarTextEntry(this, "GridSizeTextEntry", "mom_zone_grid", 0);
    m_pGridSizeTextEntry->SetAllowNumericInputOnly(true);

    LoadControlSettingsAndUserConfig("resource/ui/ZoneMenu.res");

    SetSizeable(false);
    SetClipToParent(true);
}

int C_MomZoneMenu::HandleKeyInput(int down, ButtonCode_t keynum)
{
    if (keynum == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(true);
        SetKeyBoardInputEnabled(true);
        return true;
    }
    if (m_bBindKeys && down)
    {
        if (keynum == MOUSE_LEFT)
        {
            engine->ExecuteClientCmd("mom_zone_mark");
            return true;
        }
        if (keynum == KEY_DELETE)
        {
            m_eZoneAction = ZONEACTION_DELETE;
            engine->ExecuteClientCmd("mom_zone_info");
            return true;
        }
        if (keynum == KEY_BACKSPACE)
        {
            engine->ExecuteClientCmd("mom_zone_back");
            return true;
        }
        if (keynum == KEY_END)
        {
            engine->ExecuteClientCmd("mom_zone_cancel");
            return true;
        }
        if (keynum == KEY_ENTER || keynum == KEY_PAD_ENTER)
        {
            engine->ExecuteClientCmd("mom_zone_create");
            return true;
        }
        if (keynum == KEY_RALT)
        {
            static ConVarRef mom_zone_auto_make_stage("mom_zone_auto_make_stage");
            mom_zone_auto_make_stage.SetValue(!mom_zone_auto_make_stage.GetBool());
            return true;
        }
    }

    return false;
}

void C_MomZoneMenu::OnZoneInfoThunk(bf_read &msg) { g_pZoneMenu->OnZoneInfo(msg); }

void C_MomZoneMenu::OnZoneInfo(bf_read &msg)
{
    if (m_eZoneAction == ZONEACTION_NONE)
    {
        // Nothing to do :/
        return;
    }

    static ConVarRef mom_zone_edit("mom_zone_edit");
    int zoneidx = msg.ReadLong();
    int zonetype = msg.ReadLong();
    (void)zonetype;

    if (zoneidx == -1)
    {
        // No zone found
        Warning("No zones found\n");
        return;
    }

    switch (m_eZoneAction)
    {
    case ZONEACTION_DELETE:
    {
        bool old = mom_zone_edit.GetBool();
        mom_zone_edit.SetValue(true);

        engine->ExecuteClientCmd(CFmtStr("mom_zone_delete %i", zoneidx));

        mom_zone_edit.SetValue(old);

        break;
    }
    case ZONEACTION_EDIT:
    {
        mom_zone_edit.SetValue(true);

        engine->ExecuteClientCmd(CFmtStr("mom_zone_edit_existing %i", zoneidx));

        break;
    }
    default:
    {
        AssertMsg(false, "Unhandled zone action (%i)", m_eZoneAction);
        break;
    }
    }

    // Clear the command
    m_eZoneAction = ZONEACTION_NONE;
}

void C_MomZoneMenu::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }

    BaseClass::OnMousePressed(code);
}

void C_MomZoneMenu::OnClose()
{
    engine->ExecuteClientCmd("mom_zone_edit 0");

    BaseClass::OnClose();
}

void C_MomZoneMenu::OnTextChanged(Panel *pPanel)
{
    if (pPanel == m_pZoneTypeCombo)
    {
        static ConVarRef mom_zone_type("mom_zone_type");

        KeyValues *pData = m_pZoneTypeCombo->GetActiveItemUserData();
        mom_zone_type.SetValue(pData->GetString("zone_type"));
    }
}

void C_MomZoneMenu::OnButtonChecked(Panel *pPanel, int state)
{
    if (pPanel == m_pToggleZoneEdit)
    {
        m_pToggleUsePointMethod->SetEnabled(state);
        m_pCreateNewZoneButton->SetEnabled(state);
        m_pEditZoneButton->SetEnabled(state);
        m_pCancelZoneButton->SetEnabled(state);
        m_pDeleteZoneButton->SetEnabled(state);
        m_pSaveZonesButton->SetEnabled(state);

        m_pTrackNumberEntry->SetEnabled(state);
        m_pTrackNumberLabel->SetEnabled(state);

        m_pZoneNumberEntry->SetEnabled(state);
        m_pZoneNumberLabel->SetEnabled(state);

        m_pZoneTypeLabel->SetEnabled(state);
        m_pZoneTypeCombo->SetEnabled(state);

        m_pGridSizeLabel->SetEnabled(state);
        m_pGridSizeSlider->SetEnabled(state);
        m_pGridSizeTextEntry->SetEnabled(state);

        m_bBindKeys = state;
    }
    else if (pPanel == m_pToggleUsePointMethod)
    {
        engine->ExecuteClientCmd("mom_zone_cancel");
    }
}


void C_MomZoneMenu::OnCommand(const char *command)
{
    if (FStrEq(command, "CreateNewZone"))
    {
        // return control to game so they can start zoning immediately
        m_bBindKeys = true;
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }
    else if (FStrEq(command, "DeleteZone"))
    {
        m_eZoneAction = ZONEACTION_DELETE;
        engine->ExecuteClientCmd("mom_zone_info");
    }
    else if (FStrEq(command, "EditZone"))
    {
        m_eZoneAction = ZONEACTION_EDIT;
        engine->ExecuteClientCmd("mom_zone_info");
    }
    else if (FStrEq(command, "CancelZone"))
    {
        engine->ExecuteClientCmd("mom_zone_cancel");
    }
    else if (FStrEq(command, "SaveZones"))
    {
        engine->ExecuteClientCmd("mom_zone_generate");
    }
    else
        BaseClass::OnCommand(command);
}
