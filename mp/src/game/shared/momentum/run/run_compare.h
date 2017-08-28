#pragma once

#include "cbase.h"
#include "run_stats.h"

struct RunCompare_t
{
    // Name of the comparison.
    char runName[32]; // MOM_TODO: determine a good size for this array.
    CMomRunStats runStats;
    CMomRunStats::data runStatsData;
    
    RunCompare_t() : runStats(&runStatsData)
    {
        runName[0] = '\0';
    }
};

enum ComparisonString_t
{
    TIME_OVERALL = (1 << 0),     // Zone enter times (overall times)
    ZONE_TIME = (1 << 1),        // Times spent on zones (zone time)
    VELOCITY_AVERAGE = (1 << 2), // Average velocities for zones
    VELOCITY_MAX = (1 << 3),     // Maximum velocities for zones
    VELOCITY_ENTER = (1 << 4),   // Velocity with which you enter a zone (exit a zone start trigger)
    VELOCITY_EXIT = (1 << 5),    // Velocity with which you leave a zone (staged maps only)
    ZONE_SYNC1 = (1 << 6),       // Average zone sync1
    ZONE_SYNC2 = (1 << 7),       // Average zone sync2
    ZONE_JUMPS = (1 << 8),       // Number of jumps on this zone
    ZONE_STRAFES = (1 << 9),     // Number of strafes on this zone

    //The below are used only in a bogus hud_comparisons, for the settings panel
    ZONE_LABELS = (1 << 10),     //The "Stage/Checkpoint ###" labels 
    ZONE_LABELS_COMP = (1 << 11) //The (+/- XX:XX.XX) next to the above label
};