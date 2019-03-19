#include "cbase.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "mapzones_build.h"

#include "tier0/memdbgon.h"

static void SaveZonFile(const char *pMapName);
static void CC_Mom_GenerateZoneFile() { SaveZonFile(gpGlobals->mapname.ToCStr()); }
static ConCommand mom_generate_zone_file("mom_zone_generate", CC_Mom_GenerateZoneFile, "Generates a zone file.");

class CMapzone
{
  public:
    CMapzone(const int type, const KeyValues *values);
    ~CMapzone();

    void SpawnZone();

    int GetType() const { return m_iType; }

  private:
    int m_iType;
    // KeyValues containing all the values describing the zone
    KeyValues *m_pZoneValues;

    CBaseMomZoneTrigger *m_pTrigger;
};

CMapzone::CMapzone(const int type, const KeyValues *values)
{
    m_iType = type;
    m_pZoneValues = values->MakeCopy();
    m_pTrigger = nullptr;
}

CMapzone::~CMapzone()
{
    m_pZoneValues->deleteThis();
}

void CMapzone::SpawnZone()
{
    char classname[64];
    ZoneTypeToClass(m_iType, classname, sizeof(classname));
    m_pTrigger = dynamic_cast<CBaseMomZoneTrigger *>(CreateEntityByName(classname));
    AssertMsg(m_pTrigger, "Unhandled zone type");

    if (m_pTrigger)
    {
        if (m_iType == ZONE_TYPE_START)
        {
            g_pMomentumTimer->SetStartTrigger(static_cast<CTriggerTimerStart *>(m_pTrigger));
        }

        const bool success = m_pTrigger->LoadFromKeyValues(m_pZoneValues);
        if (!success)
        {
            Warning("Failed to load zone of type '%s' (Invalid zone data)", classname);
            Assert(false);
            return;
        }

        CMomBaseZoneBuilder* pBaseBuilder = CreateZoneBuilderFromKeyValues(m_pZoneValues);

        pBaseBuilder->BuildZone();
        m_pTrigger->Spawn();
        pBaseBuilder->FinishZone(m_pTrigger);

        m_pTrigger->Activate();

        delete pBaseBuilder;
    }
}

static void SaveZonFile(const char *szMapName)
{
    KeyValuesAD zoneKV(szMapName);
    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_*");
    while (pEnt)
    {
        CBaseMomZoneTrigger *pTrigger = dynamic_cast<CBaseMomZoneTrigger*>(pEnt);
        if (!pTrigger)
        {
            AssertMsg(false, "Entity with classname trigger_momentum_* was not a momentum trigger");
            continue;
        }

        KeyValues *pSubKey = zoneKV->CreateNewKey();
        if (pTrigger->ToKeyValues(pSubKey))
        {
            CMomBaseZoneBuilder *pBuilder = CreateZoneBuilderFromExisting(pTrigger);

            if (!pBuilder->Save(pSubKey))
            {
                Warning("Failed to save zone to file!\n");
            }

            delete pBuilder;
        }
        else
        {
            zoneKV->RemoveSubKey(pSubKey);
            pSubKey->deleteThis();
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_*");
    }
    if (zoneKV->GetFirstSubKey()) // not empty
    {
        char zoneFilePath[MAX_PATH];
        V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
        V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
        zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
    }
}

CMapzoneData::CMapzoneData(const char *szMapName)
{
    if (!LoadFromFile(szMapName))
    {
        DevLog("No existing .zon file found!\n");
    }
}

CMapzoneData::~CMapzoneData()
{
    if (!m_zones.IsEmpty())
    {
        m_zones.PurgeAndDeleteElements();
    }
}

void CMapzoneData::SpawnMapZones()
{
    int count = m_zones.Count();
    for (int i = 0; i < count; i++)
    {
        m_zones[i]->SpawnZone();
    }
}

bool CMapzoneData::LoadFromFile(const char *szMapName)
{
    char zoneFilePath[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
    V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
    DevLog("Looking for zone file: %s \n", zoneFilePath);

    KeyValuesAD zoneKV(szMapName);
    if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "GAME"))
    {
        // Go through checkpoints
        for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
        {
            // Load position information (will default to 0 if the keys don't exist)
            int zoneType = ZONE_TYPE_INVALID;

            const char *name = cp->GetName();
            if (FStrEq(name, "start"))
            {
                zoneType = ZONE_TYPE_START;
            }
            else if (FStrEq(name, "checkpoint"))
            {
                zoneType = ZONE_TYPE_CHECKPOINT;
            }
            else if (FStrEq(name, "end"))
            {
                zoneType = ZONE_TYPE_STOP;
            }
            else if (FStrEq(name, "stage"))
            {
                zoneType = ZONE_TYPE_STAGE;
            }
            else
            {
                Warning("Error while reading zone file: Unknown mapzone type %s!\n", cp->GetName());
                continue;
            }

            // Add element
            m_zones.AddToTail(new CMapzone(zoneType, cp));
        }

        DevLog("Successfully loaded map zone file %s!\n", zoneFilePath);
        return true;
    }
    return false;
}

bool ZoneTypeToClass(int type, char *dest, int maxlen)
{
    switch (type)
    {
    case ZONE_TYPE_START:
        Q_strncpy(dest, "trigger_momentum_timer_start", maxlen);
        return true;
    case ZONE_TYPE_CHECKPOINT:
        Q_strncpy(dest, "trigger_momentum_timer_checkpoint", maxlen);
        return true;
    case ZONE_TYPE_STOP:
        Q_strncpy(dest, "trigger_momentum_timer_stop", maxlen);
        return true;
    case ZONE_TYPE_STAGE:
        Q_strncpy(dest, "trigger_momentum_timer_stage", maxlen);
        return true;
    default:
        return false;
    }
}