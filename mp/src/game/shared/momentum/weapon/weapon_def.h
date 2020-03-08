#pragma once

#include "weapon/weapon_shareddefs.h"
#include "igamesystem.h"

#ifdef CLIENT_DLL
#include "GameEventListener.h"
#endif

class CHudTexture;

struct WeaponScriptDefinition
{
    char szPrintName[MAX_WEAPON_STRING];       // Name for showing in HUD
    char szAnimationPrefix[MAX_WEAPON_PREFIX]; // Prefix of the animations that should be used by the player carrying this weapon
    int iSlot;                                 // inventory slot.
    int iPosition;                             // position in the inventory slot.
    int iMaxClip1;                             // max primary clip size (-1 if no clip)
    int iMaxClip2;                             // max secondary clip size (-1 if no clip)
    int iDefaultClip1;                         // amount of primary ammo in the gun when it's created
    int iDefaultClip2;                         // amount of secondary ammo in the gun when it's created
    int iWeight;          // this value used to determine this weapon's importance in autoselection.
    bool bAutoSwitchTo;   // whether this weapon should be considered for autoswitching to
    bool bAutoSwitchFrom; // whether this weapon can be autoswitched away from when picking up another weapon or ammo
    bool bMeleeWeapon; // Melee weapons can always "fire" regardless of ammo.

    // This tells if the weapon was built right-handed (defaults to true).
    // This helps cl_righthand make the decision about whether to flip the model or not.
    bool bBuiltRightHanded;
    bool bAllowFlipping; // False to disallow flipping the model, regardless of whether it is built left or right handed.

    // TF2 specific
    bool bShowUsageHint; // if true, then when you receive the weapon, show a hint about it

    // Mom specific
    int iCrosshairMinDistance;
    int iCrosshairDeltaDistance;

    // Weapon models
    KeyValues *pKVWeaponModels;
    // Particle effects of weapons
    KeyValues *pKVWeaponParticles;
    // Sound Data
    KeyValues *pKVWeaponSounds;

    WeaponScriptDefinition();
    ~WeaponScriptDefinition();
    void Parse(KeyValues *pKvInput);
    void Precache();
};

#ifdef CLIENT_DLL
struct WeaponHUDResourceDefinition
{
    CUtlVector<CHudTexture *> m_vecResources;

    WeaponHUDResourceDefinition();
    void Parse(KeyValues *pKvInput);
};
#endif

struct WeaponDefinition
{
    char szClassName[MAX_WEAPON_STRING];

    WeaponScriptDefinition m_WeaponScript;   // Models, sounds, and particles for the gun

#ifdef CLIENT_DLL
    WeaponHUDResourceDefinition m_WeaponHUD; // HUD icons (weapon selection)
#endif

    WeaponDefinition(const char *pWeaponClassname);
    void Parse(KeyValues *pKvInput);
    void Precache();
};

class CWeaponDef : public CAutoGameSystem
#ifdef CLIENT_DLL
, public CGameEventListener
#endif
{
public:
    CWeaponDef();

    void PostInit() override;

    void LoadWeaponDefinitions();
    void ReloadWeaponDefinitions();
    void ReloadWeaponDefinition(WeaponID_t id);

    // Convenience methods
    const char *GetWeaponParticle(WeaponID_t id, const char *pKey);
    const char *GetWeaponModel(WeaponID_t id, const char *pKey);
    const char *GetWeaponSound(WeaponID_t id, const char *pKey);

    WeaponDefinition *GetWeaponDefinition(WeaponID_t id);
    WeaponScriptDefinition *GetWeaponScript(WeaponID_t id);

#ifdef CLIENT_DLL
    WeaponHUDResourceDefinition *GetWeaponHUDResource(WeaponID_t id);
protected:
    void FireGameEvent(IGameEvent *event) override;
#else
private:
    void FireWeaponDefinitionReloadedEvent(WeaponID_t id);
#endif

private:
    WeaponDefinition *ParseWeaponScript(const char *pWeaponName);

    CUtlVector<WeaponDefinition *> m_vecWeaponDefs;
};

extern CWeaponDef *g_pWeaponDef;