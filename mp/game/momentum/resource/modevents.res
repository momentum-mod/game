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
	"timer_stopped"
	{
		"avg_sync"	"float"
		"avg_sync2"	"float"
		"num_strafes"	"short"
		"num_jumps"	"short"
        
        "avg_vel"	"float"
		"max_vel"	"float"
		"start_vel"	"float"
		"end_vel"	"float"
        "avg_vel_2D"	"float"
		"max_vel_2D"	"float"
		"start_vel_2D"	"float"
		"end_vel_2D"	"float"
	}
	"stage_enter"
	{
		"stage_num"	"byte"
		"stage_enter_time"	"float" //time is in seconds
		"avg_sync"	"float"
		"avg_sync2"	"float"
		"num_strafes"	"short"
		"num_jumps"	"short"
        
        "avg_vel"	"float"
		"max_vel"	"float"
		"stage_exit_vel"	"float"//previous stage's exit velocity
        //we save both XY and XYZ, so we can look at both if need be...
        "avg_vel_2D"	"float"
		"max_vel_2D"	"float"
		"stage_exit_vel_2D"	"float"//previous stage's horizontal exit velocity
	}
    "stage_exit"//When the player exits the start trigger for the stage
    {
        "stage_num" "byte"
        "stage_enter_vel"    "float"//velocity in which the player starts the stage (exits the stage trigger)
        "stage_enter_vel_2D"    "float"
    }
	"run_save"
	{
		"run_saved"	"bool"
	}
    "run_upload"
    {
        "run_posted" "bool"
        "web_msg" "string"//MOM_TODO: fill this with more stuff?
    }
	"timer_state"
	{
		"is_running"	"bool"
	}
	"keypress"
	{
		"num_jumps"	"short"
		"num_strafes"	"short"
	}
    "map_init"
    {
        "is_linear" "bool"
        "num_checkpoints" "byte"
    }
}
