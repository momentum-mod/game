#include "cbase.h"
#include "ZoneMenu.h"
#include "clientmode.h"
#include "icliententitylist.h"
#include "mom_player_shared.h"
#include "util\mom_util.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarTextEntry.h>

using namespace vgui;

CMomZoneMenu *g_pZoneMenu = nullptr;

CON_COMMAND(mom_show_zonemenu, "Shows zoning menu")
{
    if (!g_pZoneMenu)
    {
        g_pZoneMenu = new CMomZoneMenu(g_pClientMode->GetViewport());
    }
    g_pZoneMenu->Activate();
}

CMomZoneMenu::CMomZoneMenu(Panel *pParentPanel) : Frame(pParentPanel, "ZoneMenu")
{
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");

    m_bBindKeys = false;
    m_iCurrentZone = -1;

    SetSize(600, 250);
    SetPos(20, 180);
    SetTitle("Zoning Menu", true);
    SetSizeable(false);
    SetMoveable(true);

    m_pEditorTitleLabel = new Label(this, "ZoneMenuEditorLabel", "Editor Controls");
    m_pEditorTitleLabel->SetPos(30, 50);
    m_pEditorTitleLabel->SetWide(200);

    m_pCreateNewZoneButton = new Button(this, "CreateNewZone", "Create a new Zone", this);
    m_pCreateNewZoneButton->SetPos(30, 70);
    m_pCreateNewZoneButton->SetWide(200);
    m_pDeleteZoneButton = new Button(this, "DeleteZone", "Delete Zone", this);
    m_pDeleteZoneButton->SetPos(30, 90);
    m_pDeleteZoneButton->SetWide(200);
    m_pEditZoneButton = new Button(this, "EditZone", "Edit Zone", this);
    m_pEditZoneButton->SetPos(30, 110);
    m_pEditZoneButton->SetWide(200);
    m_pCancelZoneButton = new Button(this, "CancelZone", "Cancel Zone", this);
    m_pCancelZoneButton->SetPos(30, 150);
    m_pCancelZoneButton->SetWide(200);

    m_pCreateNewZoneButton->SetCommand(new KeyValues("CreateNewZone"));
    m_pDeleteZoneButton->SetCommand(new KeyValues("DeleteZone"));
    m_pEditZoneButton->SetCommand(new KeyValues("EditZone"));
    m_pCancelZoneButton->SetCommand(new KeyValues("CancelZone"));

	m_pZoneTypeLabel = new Label(this, "ZoneTypeLabel", "Zone Type:");
    m_pZoneTypeLabel->SetPos(260, 50);
    m_pZoneTypeLabel->SetWide(200);
    m_pZoneTypeCombo = new ComboBox(this, "ZoneTypeCombo", 7, false);
    m_pZoneTypeCombo->SetPos(260, 70);
    m_pZoneTypeCombo->SetWide(180);
    m_pZoneTypeCombo->SetEditable(false);
    m_pZoneTypeCombo->AddItem("Auto",        new KeyValues("vals", "zone_type", "auto",  "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Start",       new KeyValues("vals", "zone_type", "start", "bonus", "0"));
    m_pZoneTypeCombo->AddItem("End",         new KeyValues("vals", "zone_type", "end",   "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Bonus Start", new KeyValues("vals", "zone_type", "start", "bonus", "1"));
    m_pZoneTypeCombo->AddItem("Bonus End",   new KeyValues("vals", "zone_type", "end",   "bonus", "1"));
    m_pZoneTypeCombo->AddItem("Stage",       new KeyValues("vals", "zone_type", "stage", "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Checkpoint",  new KeyValues("vals", "zone_type", "cp",    "bonus", "0"));
    m_pZoneTypeCombo->ActivateItemByRow(0);

    m_pGridSizeLabel = new Label(this, "GridSizeLabel", "Grid Size:");
    m_pGridSizeLabel->SetPos(260, 100);
    m_pGridSizeLabel->SetWide(200);
    m_pGridSizeSlider = new CvarSlider(this, "GridSizeSlider", "Grid Size", 1.0f, 64.0f, "mom_zone_grid");
    m_pGridSizeSlider->SetPos(260, 120);
    m_pGridSizeSlider->SetWide(180);
    m_pGridSizeSlider->SetTall(50);
    m_pGridSizeTextEntry = new CvarTextEntry(this, "GridSizeTextEntry", "mom_zone_grid");
    m_pGridSizeTextEntry->SetPos(450, 120);
    m_pGridSizeTextEntry->SetWide(50);
    m_bUpdateGridSizeSlider = false;

    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}

int CMomZoneMenu::HandleKeyInput(int down, ButtonCode_t keynum)
{
    if (keynum == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(true);
        SetKeyBoardInputEnabled(true);
        return true;
    }
    else if (ShouldBindKeys() && down)
    {
        if (keynum == MOUSE_LEFT)
        {
            engine->ExecuteClientCmd("mom_zone_mark");
            return true;
        }
        else if (keynum == KEY_DELETE)
        {
            engine->ExecuteClientCmd("mom_zone_back");
            return true;
        }
        else if (keynum == KEY_ENTER)
        {
            engine->ExecuteClientCmd("mom_zone_create");
            return true;
        }
    }

    return false;
}

void CMomZoneMenu::CancelZoning()
{
    ConVarRef mom_zone_edit("mom_zone_edit");

    engine->ExecuteClientCmd("mom_zone_cancel");
    mom_zone_edit.SetValue(false);
    m_bBindKeys = false;
}

void CMomZoneMenu::FireGameEvent(IGameEvent *event)
{
    if (Q_strcmp(event->GetName(), "zone_enter") == 0)
    {
        m_iCurrentZone = event->GetInt("zone_ent", -1);
    }
    else // zone_exit
    {
        m_iCurrentZone = -1;
    }
}

void CMomZoneMenu::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }

    BaseClass::OnMousePressed(code);
}

void CMomZoneMenu::OnClose()
{
    CancelZoning();

    BaseClass::OnClose();
}

void CMomZoneMenu::OnControlModified(Panel* pPanel)
{
    if (pPanel == m_pGridSizeSlider)
    {
        // Don't retrigger the cvar change if it was already updated by textentry
        if (!m_bUpdateGridSizeSlider)
        {
            m_bUpdateGridSizeSlider = true;
            return;
        }

        // Round val to whole number, because no one wants to align to 6.1238765426
        float flVal = roundf(m_pGridSizeSlider->GetSliderValue());
        m_pGridSizeSlider->SetSliderValue(flVal);
        m_pGridSizeSlider->ApplyChanges();
        // update textentry control
        char szVal[32];
        Q_snprintf(szVal, sizeof(szVal), "%.0f", flVal);
        m_pGridSizeTextEntry->SetText(szVal);
    }
}

void CMomZoneMenu::OnTextChanged(Panel *pPanel)
{
    if (pPanel == m_pGridSizeTextEntry)
    {
        m_bUpdateGridSizeSlider = false;
        m_pGridSizeTextEntry->ApplyChanges();
    }
    else if (pPanel == m_pZoneTypeCombo)
    {
        static ConVarRef mom_zone_type("mom_zone_type");
        static ConVarRef mom_zone_bonus("mom_zone_bonus");

        KeyValues *pData = m_pZoneTypeCombo->GetActiveItemUserData();
        mom_zone_type.SetValue(pData->GetString("zone_type"));
        mom_zone_bonus.SetValue(pData->GetInt("bonus"));
    }
}

void CMomZoneMenu::OnCreateNewZone()
{
    ConVarRef mom_zone_edit("mom_zone_edit");
    ConVarRef mom_zone_usenewmoethod("mom_zone_usenewmethod");

    mom_zone_usenewmoethod.SetValue(true);
    mom_zone_edit.SetValue(true);

    // return control to game so they can start zoning immediately
    m_bBindKeys = true;
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}

void CMomZoneMenu::OnDeleteZone()
{
    ConVarRef mom_zone_edit("mom_zone_edit");

    if (m_iCurrentZone > -1)
    {
        mom_zone_edit.SetValue(true);

        char cmd[128];
        Q_snprintf(cmd, sizeof(cmd), "mom_zone_delete %i", m_iCurrentZone);
        engine->ExecuteClientCmd(cmd);

        mom_zone_edit.SetValue(false);
    }
    else
    {
        Warning("You must be standing in a zone to delete it");
    }
}

void CMomZoneMenu::OnEditZone()
{
    ConVarRef mom_zone_edit("mom_zone_edit");

    if (m_iCurrentZone > -1)
    {
        mom_zone_edit.SetValue(true);

        char cmd[128];
        Q_snprintf(cmd, sizeof(cmd), "mom_zone_edit_existing %i", m_iCurrentZone);
        engine->ExecuteClientCmd(cmd);

        // return control to game so they can start zoning immediately
        m_bBindKeys = true;
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }
    else
    {
        Warning("You must be standing in a zone to edit it");
    }
}

void CMomZoneMenu::OnCancelZone() { CancelZoning(); }