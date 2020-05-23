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
    "paintgun_panel"
    {
        "show" "bool"
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
}
