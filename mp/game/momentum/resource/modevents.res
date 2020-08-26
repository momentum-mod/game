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
//   time       : firing server time
//   eventid    : holds the event ID

"modevents"
{
    "zone_enter" // When the player/ghost enters a zone trigger
    {
        "ent" "short"
        "zone_ent" "short"
        "num" "short"
    }
    "zone_exit" // When the player exits the start trigger for the stage
    {
        "ent" "short"
        "zone_ent" "short"
        "num" "short"
    }
    "run_submit"
    {
        "state" "byte"
    }
    "run_upload"
    {
        "run_posted" "bool"
        "cos_xp" "long"
        "rank_xp" "long"
        "lvl_gain" "byte"
    }
    "timer_event" // Fired when timer starts/stops/fails to start
    {
        "ent" "short"
        "type" "byte"
    }
    "saveloc_upd8"
    {
        "using" "bool"
        "count" "long"
        "current" "long"
    }
    "practice_mode"
    {
        "enabled" "bool"
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
        "filename" "string" // the file name itself with extension
        "filepath" "string" // the full path + file name, used for file writing
        "save"     "bool"
        "time"     "long"   // time in milliseconds
    }
    "gravity_change"
    {
        "newgravity" "float"
    }
    "lobby_leave"
    {
    }
    "lobby_join"
    {
    }
    "spec_start"
    {
    }
    "spec_stop"
    {
    }
    "achievement_earned"
    {
        "player"      "byte"    // entindex of the player
        "achievement" "short"   // achievement ID
    }
    "invalid_mdl_cache"
    {
    }
    "site_auth"
    {
        "success" "bool"
    }
    "reload_weapon_script"
    {
        "id" "byte"
    }
    "lobby_update_msg"
    {
        "type" "byte"
        "id" "string"
    }
    "lobby_spec_update_msg"
    {
        "type" "byte"
        "id" "string"     // Person speccing
        "target" "string" // Spec target
    }
    "mapcache_map_load"
    {
        "map" "string"
    }
    "player_jumped"
    {
    }
    "ramp_board"
    {
        "speed" "float"
    }
    "ramp_leave"
    {
        "speed" "float"
    }
    "player_explosive_hit"
    {
        "speed" "float"
    }
    "trick_data_loaded"
    {
        
    }
    "tricks_tracking"
    {
        "type" "byte"
        "num" "short"
    }
    hud_chat_opened
    {
    }
	"instructor_server_hint_create" // Originally ported from the Alien Swarm SDK
	{
		"hint_name"					"string"	// what to name the hint. For referencing it again later (e.g. a kill command for the hint instead of a timeout)
		"hint_replace_key"			"string"	// type name so that messages of the same type will replace each other
		"hint_target"				"long"		// entity id that the hint should display at
		"hint_activator_userid"		"short"		// userid id of the activator
		"hint_timeout"				"short"	 	// how long in seconds until the hint automatically times out, 0 = never
		"hint_icon_onscreen"		"string"	// the hint icon to use when the hint is onscreen. e.g. "icon_alert_red"
		"hint_icon_offscreen"		"string"	// the hint icon to use when the hint is offscreen. e.g. "icon_alert"
		"hint_caption"				"string"	// the hint caption. e.g. "#ThisIsDangerous"
		"hint_activator_caption"	"string"	// the hint caption that only the activator sees e.g. "#YouPushedItGood"
		"hint_color"				"string"	// the hint color in "r,g,b" format where each component is 0-255
		"hint_icon_offset"			"float"		// how far on the z axis to offset the hint from entity origin
		"hint_range"				"float"		// range before the hint is culled
		"hint_flags"				"long"		// hint flags
		"hint_binding"				"string"	// bindings to use when use_binding is the onscreen icon
		"hint_allow_nodraw_target"	"bool"		// if false, the hint will dissappear if the target entity is invisible
		"hint_nooffscreen"			"bool"		// if true, the hint will not show when outside the player view
		"hint_forcecaption"			"bool"		// if true, the hint caption will show even if the hint is occluded
		"hint_local_player_only"	"bool"		// if true, only the local player will see the hint
		"hint_start_sound"			"string"	// Mapbase - the sound to play when the hint is opened
		"hint_target_pos"			"short"		// Mapbase - where the icon should be when there's a hint target
	}
	"instructor_server_hint_stop" //destroys a server/map created hint
	{
		"hint_name"					"string"	// The hint to stop. Will stop ALL hints with this name
	}
}
