#pragma once

#define INFINITE_AMMO (-2)
#define MAX_WEAPON_STRING 80
#define MAX_WEAPON_PREFIX 16
#define WEAPON_PRINTNAME_MISSING "!!! Missing printname on weapon"

#define MOM_WEAPON_STICKYBOMB_COUNT 3

enum WeaponID_t
{
    WEAPON_NONE = 0,

    WEAPON_PISTOL,
    WEAPON_SHOTGUN,
    WEAPON_MACHINEGUN,
    WEAPON_SNIPER,
    WEAPON_GRENADE,
    WEAPON_CONCGRENADE,
    WEAPON_KNIFE,
    WEAPON_ROCKETLAUNCHER,
    WEAPON_STICKYLAUNCHER,

    WEAPON_MAX, // number of weapons weapon index

    WEAPON_FIRST = WEAPON_PISTOL,
    WEAPON_LAST = WEAPON_STICKYLAUNCHER,
};

static const char *const g_szWeaponNames[WEAPON_MAX] = 
{
    "weapon_none",
    "weapon_momentum_pistol",
    "weapon_momentum_shotgun",
    "weapon_momentum_machinegun",
    "weapon_momentum_sniper",
    "weapon_momentum_grenade",
    "weapon_momentum_concgrenade",
    "weapon_knife",
    "weapon_momentum_rocketlauncher",
    "weapon_momentum_stickylauncher"
};

enum WeaponHudResource_t
{
    HUD_RESOURCE_ACTIVE,
    HUD_RESOURCE_INACTIVE,
    HUD_RESOURCE_AMMO,
    HUD_RESOURCE_AMMO2,
    HUD_RESOURCE_CROSSHAIR,
    HUD_RESOURCE_ZOOMED_CROSSHAIR,
    HUD_RESOURCE_SMALL,

    HUD_RESOURCE_MAX
};

static const char * const g_szWeaponHudResourceNames[HUD_RESOURCE_MAX] =
{
    "weapon_s",
    "weapon",
    "ammo",
    "ammo2",
    "crosshair",
    "zoom",
    "weapon_small"
};

enum AmmoTracer_t
{
    TRACER_NONE,
    TRACER_LINE,
    TRACER_RAIL,
    TRACER_BEAM,
    TRACER_LINE_AND_WHIZ,
};

enum AmmoType_t
{
    AMMO_TYPE_NONE = -1,
    AMMO_TYPE_PISTOL,
    AMMO_TYPE_MACHINEGUN,
    AMMO_TYPE_SNIPER,
    AMMO_TYPE_SHOTGUN,
    AMMO_TYPE_GRENADE,
    AMMO_TYPE_PAINT,

    AMMO_TYPE_MAX
};