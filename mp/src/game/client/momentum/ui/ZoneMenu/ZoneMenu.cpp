#include "cbase.h"
#include "ZoneMenu.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "clientmode.h"
#include "mom_player_shared.h"
#include "util\mom_util.h"
#include "icliententitylist.h"

using namespace vgui;

ZoneMenu *g_pZoneMenu = nullptr;

CON_COMMAND(show_zonemenu, "Shows zoning menu") 
{
	if( !g_pZoneMenu )
	{
        g_pZoneMenu = new ZoneMenu(g_pClientMode->GetViewport());
	}
    g_pZoneMenu->Activate();
}

ZoneMenu::ZoneMenu(Panel* pParentPanel) : Frame(pParentPanel, "ZoneMenu")
{
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");

    m_bBindMouseToMark = false;

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

void ZoneMenu::FireGameEvent(IGameEvent* event)
{ Log("Event fired!"); }

void ZoneMenu::OnMousePressed( MouseCode code )
{
	if (code == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(false);
	}
}

void ZoneMenu::OnCreateNewZone()
{
    ConVarRef mom_zone_edit("mom_zone_edit");
    mom_zone_edit.SetValue(true);

    m_bBindMouseToMark = mom_zone_edit.GetBool();
	if (m_bBindMouseToMark)
    {
		// return control to game so they can start zoning immediately
        SetMouseInputEnabled(false);
	}
}

void ZoneMenu::OnDeleteZone()
{
    
}

void ZoneMenu::OnEditZone()
{

}