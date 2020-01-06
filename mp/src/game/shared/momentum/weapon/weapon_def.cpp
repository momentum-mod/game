#include "cbase.h"

#include "weapon_def.h"

#include "filesystem.h"
#include "fmtstr.h"

#ifdef CLIENT_DLL
#include "hud.h"
#endif

#include "tier0/memdbgon.h"

WeaponScriptDefinition::WeaponScriptDefinition()
{
    szPrintName[0] = 0;
    szViewModel[0] = 0;
    szWorldModel[0] = 0;
    szAnimationPrefix[0] = 0;

    iSlot = 0;
    iPosition = 0;
    iMaxClip1 = 0;
    iMaxClip2 = 0;
    iDefaultClip1 = 0;
    iDefaultClip2 = 0;
    iWeight = 0;
    bAutoSwitchTo = false;
    bAutoSwitchFrom = false;
    bMeleeWeapon = false;
    bShowUsageHint = false;
    bAllowFlipping = true;
    bBuiltRightHanded = true;
    iCrosshairDeltaDistance = 0;
    iCrosshairMinDistance = 0;

    pKVWeaponParticles = nullptr;
    pKVWeaponSounds = nullptr;
}

WeaponScriptDefinition::~WeaponScriptDefinition()
{
    if (pKVWeaponSounds)
        pKVWeaponSounds->deleteThis();

    if (pKVWeaponParticles)
        pKVWeaponParticles->deleteThis();
}

void WeaponScriptDefinition::Parse(KeyValues *pKvInput)
{
    Q_strncpy(szPrintName, pKvInput->GetString("printname", WEAPON_PRINTNAME_MISSING), MAX_WEAPON_STRING);
    Q_strncpy(szViewModel, pKvInput->GetString("viewmodel"), MAX_WEAPON_STRING);
    Q_strncpy(szWorldModel, pKvInput->GetString("playermodel"), MAX_WEAPON_STRING);
    Q_strncpy(szAnimationPrefix, pKvInput->GetString("anim_prefix"), MAX_WEAPON_PREFIX);
    iSlot = pKvInput->GetInt("bucket", 0);
    iPosition = pKvInput->GetInt("bucket_position", 0);
    iMaxClip1 = pKvInput->GetInt("clip_size", WEAPON_NOCLIP); // Max primary clips gun can hold (assume they don't use clips by default)
    iMaxClip2 = pKvInput->GetInt("clip2_size", WEAPON_NOCLIP); // Max secondary clips gun can hold (assume they don't use clips by default)
    iDefaultClip1 = pKvInput->GetInt("default_clip", iMaxClip1); // amount of primary ammo placed in the primary clip when it's picked up
    iDefaultClip2 = pKvInput->GetInt("default_clip2", iMaxClip2); // amount of secondary ammo placed in the secondary clip when it's picked up
    iWeight = pKvInput->GetInt("weight", 0);

    bShowUsageHint = pKvInput->GetBool("showusagehint");
    bAutoSwitchTo = pKvInput->GetBool("autoswitchto", true);
    bAutoSwitchFrom = pKvInput->GetBool("autoswitchfrom", true);
    bBuiltRightHanded = pKvInput->GetBool("BuiltRightHanded", true);
    bAllowFlipping = pKvInput->GetBool("AllowFlipping", true);
    bMeleeWeapon = pKvInput->GetBool("MeleeWeapon");
    iCrosshairMinDistance = pKvInput->GetInt("CrosshairMinDistance", 4);
    iCrosshairDeltaDistance = pKvInput->GetInt("CrosshairDeltaDistance", 3);

    if (pKVWeaponSounds)
    {
        pKVWeaponSounds->deleteThis();
        pKVWeaponSounds = nullptr;
    }
    pKVWeaponSounds = pKvInput->FindKey("SoundData", true)->MakeCopy();

    if (pKVWeaponParticles)
    {
        pKVWeaponParticles->deleteThis();
        pKVWeaponParticles = nullptr;
    }
    pKVWeaponParticles = pKvInput->FindKey("ParticleData", true)->MakeCopy();

#ifdef GAME_DLL
    // Model bounds are rounded to the nearest integer, then extended by 1
    engine->ForceModelBounds(szWorldModel, Vector(-15, -12, -18), Vector(44, 16, 19));
#endif
}

void WeaponScriptDefinition::Precache()
{
    FOR_EACH_VALUE(pKVWeaponSounds, pKvSound)
    {
        const auto pSoundStr = pKvSound->GetString();
        if (pSoundStr && pSoundStr[0])
            CBaseEntity::PrecacheScriptSound(pSoundStr);
    }

    FOR_EACH_VALUE(pKVWeaponParticles, pKvParticle)
    {
        const auto pParticleSystemStr = pKvParticle->GetString();
        if (pParticleSystemStr && pParticleSystemStr[0])
            PrecacheParticleSystem(pParticleSystemStr);
    }
}

#ifdef CLIENT_DLL
WeaponHUDResourceDefinition::WeaponHUDResourceDefinition() { m_vecResources.EnsureCount(HUD_RESOURCE_MAX); }

void WeaponHUDResourceDefinition::Parse(KeyValues *pKvInput)
{
    CUtlDict<CHudTexture *> weaponResources;
    LoadHudTextures(weaponResources, pKvInput);

    for (auto hudResource = 0; hudResource < HUD_RESOURCE_MAX; hudResource++)
    {
        const auto pResourceName = g_szWeaponHudResourceNames[hudResource];
        const auto iFound = weaponResources.Find(pResourceName);
        if (weaponResources.IsValidIndex(iFound))
        {
            m_vecResources[hudResource] = gHUD.AddUnsearchableHudIconToList(*weaponResources[iFound]);
        }
    }

    weaponResources.PurgeAndDeleteElements();
}

#endif

WeaponDefinition::WeaponDefinition(const char *pWeaponClassname)
{
    Q_strncpy(szClassName, pWeaponClassname, sizeof(szClassName));
}

void WeaponDefinition::Parse(KeyValues *pKvInput)
{
    m_WeaponScript.Parse(pKvInput);

#ifdef CLIENT_DLL
    m_WeaponHUD.Parse(pKvInput);
#endif
}

void WeaponDefinition::Precache() { m_WeaponScript.Precache(); }


CWeaponDef::CWeaponDef() : CAutoGameSystem("CWeaponDef")
{

}

void CWeaponDef::PostInit()
{
    LoadWeaponDefinitions();
}

void CWeaponDef::LoadWeaponDefinitions()
{
    if (!m_vecWeaponDefs.IsEmpty())
        return;

    m_vecWeaponDefs.EnsureCount(WEAPON_MAX);

    m_vecWeaponDefs[WEAPON_NONE] = new WeaponDefinition("none");
    m_vecWeaponDefs[WEAPON_PISTOL]          = ParseWeaponScript(g_szWeaponNames[WEAPON_PISTOL]);
    m_vecWeaponDefs[WEAPON_RIFLE]           = ParseWeaponScript(g_szWeaponNames[WEAPON_RIFLE]);
    m_vecWeaponDefs[WEAPON_SHOTGUN]         = ParseWeaponScript(g_szWeaponNames[WEAPON_SHOTGUN]);
    m_vecWeaponDefs[WEAPON_SMG]             = ParseWeaponScript(g_szWeaponNames[WEAPON_SMG]);
    m_vecWeaponDefs[WEAPON_SNIPER]          = ParseWeaponScript(g_szWeaponNames[WEAPON_SNIPER]);
    m_vecWeaponDefs[WEAPON_LMG]             = ParseWeaponScript(g_szWeaponNames[WEAPON_LMG]);
    m_vecWeaponDefs[WEAPON_GRENADE]         = ParseWeaponScript(g_szWeaponNames[WEAPON_GRENADE]);
    m_vecWeaponDefs[WEAPON_KNIFE]           = ParseWeaponScript(g_szWeaponNames[WEAPON_KNIFE]);
    m_vecWeaponDefs[WEAPON_PAINTGUN]        = ParseWeaponScript(g_szWeaponNames[WEAPON_PAINTGUN]);
    m_vecWeaponDefs[WEAPON_ROCKETLAUNCHER]  = ParseWeaponScript(g_szWeaponNames[WEAPON_ROCKETLAUNCHER]);
}

void CWeaponDef::RemoveWeaponDefinitions()
{
    m_vecWeaponDefs.PurgeAndDeleteElements();
}

WeaponDefinition *CWeaponDef::GetWeaponDefinition(const CWeaponID &id)
{
    return m_vecWeaponDefs[id];
}

WeaponScriptDefinition *CWeaponDef::GetWeaponScript(const CWeaponID &id)
{
    return &m_vecWeaponDefs[id]->m_WeaponScript;
}

#ifdef CLIENT_DLL
WeaponHUDResourceDefinition *CWeaponDef::GetWeaponHUDResource(const CWeaponID &id)
{
    return &m_vecWeaponDefs[id]->m_WeaponHUD;
}
#endif

WeaponDefinition *CWeaponDef::ParseWeaponScript(const char *pWeaponName)
{
    CFmtStr filePath("scripts/%s.txt", pWeaponName);
    KeyValuesAD weaponManifest(pWeaponName);
    if (weaponManifest->LoadFromFile(g_pFullFileSystem, filePath.Get(), "GAME"))
    {
        auto pWeaponDef = new WeaponDefinition(pWeaponName);
        pWeaponDef->Parse(weaponManifest);
        return pWeaponDef;
    }

    Error("Failed to read weapon script for %s, try validating your game cache.\n", pWeaponName);
    return nullptr;
}

static CWeaponDef s_WeaponDef;
CWeaponDef *g_pWeaponDef = &s_WeaponDef;