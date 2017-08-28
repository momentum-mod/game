//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
//
// valid data key types are:
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit
//   local  : any data, but not networked to clients
//
// following key names are reserved:
//   local      : if set to 1, event is not networked to clients
//   unreliable : networked, but unreliable
//   suppress   : never fire this event
//   time	: firing server time
//   eventid	: holds the event ID

"modevents"
{
	"zone_enter"//When the player/ghost enters a checkpoint/stage trigger
	{
	}
    "zone_exit"//When the player exits the start trigger for the stage
    {
    }
    "run_upload"
    {
        "run_posted" "bool"
        "web_msg" "string"//MOM_TODO: fill this with more stuff?
    }
	"timer_state"
	{
        "ent" "short"
		"is_running"	"bool"
	}
    "map_init"
    {
        "is_linear" "bool"
        "num_zones" "byte"
    }
    "spec_target_updated" // Used by the spectator GUI
    {
    }
    "mapfinished_panel_closed"
    {
        "restart" "bool"
    }
	"replay_save"
	{
		"filename" "string"
        "save" "bool"
	}
    "weapon_fire"
    {
        "userid" "long"
        "weapon" "string"
    }
    "bullet_impact"
    {
        "userid" "long"
        "x" "float"
        "y" "float"
        "z" "float"
    }
    "gravity_change"
    {
        "newgravity" "float"
    }
}
