#pragma once

#include "cbase.h"

struct RunCompare_t
{
    // Name of the comparison.
    char runName[32]; // MOM_TODO: determine a good size for this array.
    // Note: we're using CUtlVectors here so we don't have to parse the stage/checkpoint number from the .tim file!
    CUtlVector<float> overallSplits, // Zone enter times (overall times)
        zoneSplits,                  // Times spent on zones (zone time)
        zoneAvgVels[2],              // Average velocities for zones, 0 = 3D vels, 1 = horizontal vels
        zoneMaxVels[2],              // Maximum velocities for zones, 0 = 3D vels, 1 = horizontal vels
        zoneEnterVels[2], // Velocity with which you enter a zone (exit a zone start trigger), 0 = 3D vels, 1 =
                          // horizontal vels
        zoneExitVels[2], // Velocity with which you leave a zone (one stage -> next), 0 = 3D vels, 1 = horizontal vels
        zoneAvgSync1,    // Average zone sync1
        zoneAvgSync2;    // Average zone sync2
    CUtlVector<int> zoneJumps, // Number of jumps on this zone
        zoneStrafes;           // Number of strafes on this zone
};

enum ComparisonString_t
{
    TIME_OVERALL = 1, // Zone enter times (overall times)
    ZONE_TIME,        // Times spent on zones (zone time)
    VELOCITY_AVERAGE, // Average velocities for zones
    VELOCITY_MAX,     // Maximum velocities for zones
    VELOCITY_ENTER,   // Velocity with which you enter a zone (exit a zone start trigger)
    VELOCITY_EXIT,    // Velocity with which you leave a zone (staged maps only)
    ZONE_SYNC1,       // Average zone sync1
    ZONE_SYNC2,       // Average zone sync2
    ZONE_JUMPS,       // Number of jumps on this zone
    ZONE_STRAFES      // Number of strafes on this zone
};