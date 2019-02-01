#include "cbase.h"
#include "ZoneMenu.h"
#include "clientmode.h"
#include "icliententitylist.h"
#include "mom_player_shared.h"
#include "util\mom_util.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"

using namespace vgui;

ZoneMenu *g_pZoneMenu = nullptr;

CON_COMMAND(show_zonemenu, "Shows zoning menu")
{
    if (!g_pZoneMenu)
    {
        g_pZoneMenu = new ZoneMenu(g_pClientMode->GetViewport());
    }
    g_pZoneMenu->Activate();
}

ZoneMenu::ZoneMenu(Panel *pParentPanel) : Frame(pParentPanel, "ZoneMenu")
{
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");

    m_bBindKeys = false;
    m_iCurrentZone = -1;

    SetSize(450, 250);
    SetPos(20, 180);
    SetTitle("Zoning Menu", true);
    SetSizeable(false);
    SetMoveable(true);

    m_pEditorTitleLabel = new Label(this, "ZoneMenuEditorLabel", "Editor Controls");
    m_pEditorTitleLabel->SetPos(30, 50);
    m_pEditorTitleLabel->SetWide(200);
    m_pZoneInfoLabel = new Label(this, "ZoneMenuInfoLabel", "Zone Info");
    m_pZoneInfoLabel->SetPos(250, 50);
    m_pZoneInfoLabel->SetVisible(false);

    m_pCreateNewZoneButton = new Button(this, "CreateNewZone", "Create a new Zone", this);
    m_pCreateNewZoneButton->SetPos(30, 70);
    m_pCreateNewZoneButton->SetWide(200);
    m_pDeleteZoneButton = new Button(this, "DeleteZone", "Delete Zone", this);
    m_pDeleteZoneButton->SetPos(30, 90);
    m_pDeleteZoneButton->SetWide(200);
    m_pEditZoneButton = new Button(this, "EditZone", "Edit Zone", this);
    m_pEditZoneButton->SetPos(30, 110);
    m_pEditZoneButton->SetWide(200);

    m_pCreateNewZoneButton->SetCommand(new KeyValues("CreateNewZone"));
    m_pDeleteZoneButton->SetCommand(new KeyValues("DeleteZone"));
    m_pEditZoneButton->SetCommand(new KeyValues("EditZone"));

    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}

int ZoneMenu::HandleKeyInput(int down, ButtonCode_t keynum)
{
    if (keynum == MOUSE_RIGHT)
    {
        engine->ExecuteClientCmd("mom_zone_cancel");
        ConVarRef mom_zone_edit("mom_zone_edit");
        mom_zone_edit.SetValue(false);
        g_pZoneMenu->SetMouseInputEnabled(true);
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

void ZoneMenu::FireGameEvent(IGameEvent *event)
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

void ZoneMenu::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_RIGHT)
    {
        m_bBindKeys = false;
        SetMouseInputEnabled(false);
    }
}

void ZoneMenu::OnCreateNewZone()
{
    engine->ExecuteClientCmd("mom_zone_usenewmethod 1");
    ConVarRef mom_zone_edit("mom_zone_edit");
    mom_zone_edit.SetValue(true);

    // return control to game so they can start zoning immediately
    m_bBindKeys = true;
    SetMouseInputEnabled(false);
}

void ZoneMenu::OnDeleteZone()
{
    if (m_iCurrentZone > -1)
    {
        ConVarRef mom_zone_edit("mom_zone_edit");
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

void ZoneMenu::OnEditZone()
{
    if (m_iCurrentZone > -1)
    {
        ConVarRef mom_zone_edit("mom_zone_edit");
        mom_zone_edit.SetValue(true);

        char cmd[128];
        Q_snprintf(cmd, sizeof(cmd), "mom_zone_edit_existing %i", m_iCurrentZone);
        engine->ExecuteClientCmd(cmd);

        // return control to game so they can start zoning immediately
        m_bBindKeys = true;
        SetMouseInputEnabled(false);
    }
    else
    {
        Warning("You must be standing in a zone to edit it");
    }
}