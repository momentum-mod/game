#include "cbase.h"

#include "weapon_def.h"

#include "filesystem.h"
#include "fmtstr.h"
#include "mom_player_shared.h"

#ifdef CLIENT_DLL
#include "hud.h"
#else
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

WeaponScriptDefinition::WeaponScriptDefinition()
{
    szPrintName[0] = 0;
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

    pKVWeaponModels = nullptr;
    pKVWeaponParticles = nullptr;
    pKVWeaponSounds = nullptr;
}

WeaponScriptDefinition::~WeaponScriptDefinition()
{
    if (pKVWeaponModels)
        pKVWeaponModels->deleteThis();

    if (pKVWeaponSounds)
        pKVWeaponSounds->deleteThis();

    if (pKVWeaponParticles)
        pKVWeaponParticles->deleteThis();
}

void WeaponScriptDefinition::Parse(KeyValues *pKvInput)
{
    Q_strncpy(szPrintName, pKvInput->GetString("printname", WEAPON_PRINTNAME_MISSING), MAX_WEAPON_STRING);
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

    if (pKVWeaponModels)
    {
        pKVWeaponModels->deleteThis();
        pKVWeaponModels = nullptr;
    }
    pKVWeaponModels = pKvInput->FindKey("ModelData", true)->MakeCopy();

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
}

void WeaponScriptDefinition::Precache()
{
#ifdef GAME_DLL
    // Model bounds are rounded to the nearest integer, then extended by 1
    engine->ForceModelBounds(pKVWeaponModels->GetString("world"), Vector(-15, -12, -18), Vector(44, 16, 19));
#endif

    FOR_EACH_VALUE(pKVWeaponModels, pKvModel)
    {
        const auto pWepMdl = pKvModel->GetString();
        if (pWepMdl && pWepMdl[0])
            CBaseEntity::PrecacheModel(pWepMdl);
    }

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

#ifdef CLIENT_DLL
    ListenForGameEvent("reload_weapon_script");
#endif
}

void CWeaponDef::LoadWeaponDefinitions()
{
    if (!m_vecWeaponDefs.IsEmpty())
        return;

    m_vecWeaponDefs.EnsureCount(WEAPON_MAX);

    m_vecWeaponDefs[WEAPON_NONE] = new WeaponDefinition("none");
    m_vecWeaponDefs[WEAPON_PISTOL]          = ParseWeaponScript(g_szWeaponNames[WEAPON_PISTOL]);
    m_vecWeaponDefs[WEAPON_SHOTGUN]         = ParseWeaponScript(g_szWeaponNames[WEAPON_SHOTGUN]);
    m_vecWeaponDefs[WEAPON_MACHINEGUN]      = ParseWeaponScript(g_szWeaponNames[WEAPON_MACHINEGUN]);
    m_vecWeaponDefs[WEAPON_SNIPER]          = ParseWeaponScript(g_szWeaponNames[WEAPON_SNIPER]);
    m_vecWeaponDefs[WEAPON_GRENADE]         = ParseWeaponScript(g_szWeaponNames[WEAPON_GRENADE]);
    m_vecWeaponDefs[WEAPON_CONCGRENADE]     = ParseWeaponScript(g_szWeaponNames[WEAPON_CONCGRENADE]);
    m_vecWeaponDefs[WEAPON_KNIFE]           = ParseWeaponScript(g_szWeaponNames[WEAPON_KNIFE]);
    m_vecWeaponDefs[WEAPON_ROCKETLAUNCHER]  = ParseWeaponScript(g_szWeaponNames[WEAPON_ROCKETLAUNCHER]);
    m_vecWeaponDefs[WEAPON_STICKYLAUNCHER]  = ParseWeaponScript(g_szWeaponNames[WEAPON_STICKYLAUNCHER]);
}

void CWeaponDef::ReloadWeaponDefinitions()
{
    m_vecWeaponDefs.PurgeAndDeleteElements();
    LoadWeaponDefinitions();

#ifdef GAME_DLL
    FireWeaponDefinitionReloadedEvent(WEAPON_NONE);
#endif
}

void CWeaponDef::ReloadWeaponDefinition(const WeaponID_t id)
{
    AssertMsg(id != WEAPON_NONE, "Cannot reload weapon definition of WEAPON_NONE");

    delete m_vecWeaponDefs[id];
    m_vecWeaponDefs[id] = ParseWeaponScript(g_szWeaponNames[id]);

#ifdef GAME_DLL
    FireWeaponDefinitionReloadedEvent(id);
#endif
}

const char *CWeaponDef::GetWeaponParticle(WeaponID_t id, const char *pKey)
{
    return m_vecWeaponDefs[id]->m_WeaponScript.pKVWeaponParticles->GetString(pKey);
}

const char *CWeaponDef::GetWeaponModel(WeaponID_t id, const char *pKey)
{
    return m_vecWeaponDefs[id]->m_WeaponScript.pKVWeaponModels->GetString(pKey);
}

const char *CWeaponDef::GetWeaponSound(WeaponID_t id, const char *pKey)
{
    return m_vecWeaponDefs[id]->m_WeaponScript.pKVWeaponSounds->GetString(pKey);
}

WeaponDefinition *CWeaponDef::GetWeaponDefinition(const WeaponID_t id)
{
    return m_vecWeaponDefs[id];
}

WeaponScriptDefinition *CWeaponDef::GetWeaponScript(const WeaponID_t id)
{
    return &m_vecWeaponDefs[id]->m_WeaponScript;
}

#ifdef CLIENT_DLL
WeaponHUDResourceDefinition *CWeaponDef::GetWeaponHUDResource(const WeaponID_t id)
{
    return &m_vecWeaponDefs[id]->m_WeaponHUD;
}

void CWeaponDef::FireGameEvent(IGameEvent *pEvent)
{
    const auto iWpnID = pEvent->GetInt("id", -1);
    if (iWpnID > -1 && iWpnID < WEAPON_MAX)
    {
        if (iWpnID == WEAPON_NONE)
        {
            ReloadWeaponDefinitions();
        }
        else
        {
            ReloadWeaponDefinition(static_cast<WeaponID_t>(iWpnID));
        }
    }
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

#ifdef GAME_DLL
void CWeaponDef::FireWeaponDefinitionReloadedEvent(const WeaponID_t id)
{
    const auto pGameEvent = gameeventmanager->CreateEvent("reload_weapon_script");
    if (pGameEvent)
    {
        pGameEvent->SetInt("id", id);
        gameeventmanager->FireEvent(pGameEvent);
    }
}

CON_COMMAND(weapon_reload_scripts, "Reloads all weapon scripts.\n")
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && !pPlayer->IsObserver() && !g_pMomentumTimer->IsRunning())
    {
        g_pWeaponDef->ReloadWeaponDefinitions();

        CUtlVector<WeaponID_t> vecCurrentWeps;
        pPlayer->GetCurrentWeaponIDs(vecCurrentWeps);

        if (!vecCurrentWeps.IsEmpty())
        {
            const auto iActiveWpn = pPlayer->GetActiveWeapon()->GetWeaponID();

            pPlayer->RemoveAllWeapons();

            FOR_EACH_VEC(vecCurrentWeps, i)
            {
                pPlayer->GiveWeapon(vecCurrentWeps[i]);
            }

            pPlayer->Weapon_Switch(pPlayer->GetWeapon(iActiveWpn));
        }

        Msg("Weapon scripts reloaded.\n");
    }
    else
    {
        Warning("Cannot reload weapon scripts if you are in a run or spectating!\n");
    }
}

CON_COMMAND(weapon_reload_script_current, "Reloads weapon script for the current weapon.\n")
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && !pPlayer->IsObserver() && !g_pMomentumTimer->IsRunning())
    {
        const auto pWeapon = pPlayer->GetActiveWeapon();
        if (!pWeapon)
        {
            Warning("Cannot reload script if you are not wielding a weapon!\n");
            return;
        }

        pPlayer->MomentumWeaponDrop(pWeapon);

        const auto iWpnID = pWeapon->GetWeaponID();
        g_pWeaponDef->ReloadWeaponDefinition(iWpnID);

        const auto pName = g_szWeaponNames[iWpnID];
        pPlayer->GiveWeapon(iWpnID);
        pPlayer->Weapon_Switch(pPlayer->GetWeapon(iWpnID));

        Msg("Successfully reloaded weapon script for weapon %s.\n", pName);
    }
    else
    {
        Warning("Cannot reload weapon scripts if you are in a run or spectating!\n");
    }
}
#endif

static CWeaponDef s_WeaponDef;
CWeaponDef *g_pWeaponDef = &s_WeaponDef;