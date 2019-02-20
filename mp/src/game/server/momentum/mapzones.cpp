#include "cbase.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "mapzones_build.h"

#include "tier0/memdbgon.h"

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

    CBaseMomentumTrigger *m_pTrigger;
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
    m_pTrigger = dynamic_cast<CBaseMomentumTrigger *>(CreateEntityByName(classname));
    AssertMsg(m_pTrigger, "Unhandled zone type");

    if (m_pTrigger)
    {
        if (m_iType == MOMZONETYPE_START)
        {
            g_pMomentumTimer->SetStartTrigger(static_cast<CTriggerTimerStart *>(m_pTrigger));
        }

        bool success = m_pTrigger->LoadFromKeyValues(m_pZoneValues);
        if (!success)
        {
            Warning("Failed to load zone of type '%s' (Invalid zone data)", classname);
            Assert(false);
            return;
        }

        CMomBaseZoneBuilder* pBaseBuilder = CreateZoneBuilderFromKeyValues(m_pZoneValues);

        m_pTrigger->Spawn();

        pBaseBuilder->BuildZone();
        pBaseBuilder->FinishZone(m_pTrigger);

        m_pTrigger->Activate();

        delete pBaseBuilder;
    }
}

static void SaveZonFile(const char *szMapName)
{
    KeyValues *zoneKV = new KeyValues(szMapName);
    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_*");
    while (pEnt)
    {
        CBaseMomentumTrigger *pTrigger = dynamic_cast<CBaseMomentumTrigger *>(pEnt);
        if (!pTrigger)
        {
            AssertMsg(false, "Entity with classname trigger_momentum_* was not a momentum trigger");
            continue;
        }

        KeyValues *pSubKey = pTrigger->ToKeyValues();
        if (pSubKey)
        {
            CMomBaseZoneBuilder *pBuilder = CreateZoneBuilderFromExisting(pTrigger);

            if (!pBuilder->Save(pSubKey))
            {
                Warning("Failed to save zone to file!\n");
            }

            delete pBuilder;

            zoneKV->AddSubKey(pSubKey);
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_*");
    }
    if (zoneKV->GetFirstSubKey()) // not empty
    {
        char zoneFilePath[MAX_PATH];
        Q_strcpy(zoneFilePath, "maps/");
        Q_strcat(zoneFilePath, szMapName, MAX_PATH);
        Q_strncat(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
        zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
        zoneKV->deleteThis();
    }
}

CMapzoneData::CMapzoneData(const char *szMapName)
{
    if (!LoadFromFile(szMapName))
    {
        DevLog("No existing .zon file found!\n");
    }
}

static void CC_Mom_GenerateZoneFile() { SaveZonFile(gpGlobals->mapname.ToCStr()); }

static ConCommand mom_generate_zone_file("mom_zone_generate", CC_Mom_GenerateZoneFile, "Generates a zone file.");

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

    bool toReturn = false;

    KeyValues *zoneKV = new KeyValues(szMapName);
    if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "MOD"))
    {
        // Go through checkpoints
        for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
        {
            // Load position information (will default to 0 if the keys don't exist)
            // Do specific things for different types of checkpoints
            // 0 = start, 1 = checkpoint, 2 = end, 3 = Onehop, 4 = OnehopReset, 5 = Checkpoint_teleport, 6 = Multihop, 7
            // = stage
            int zoneType = MOMZONETYPE_INVALID;

            const char *name = cp->GetName();
            if (FStrEq(name, "start"))
            {
                zoneType = MOMZONETYPE_START;
            }
            else if (FStrEq(name, "checkpoint"))
            {
                zoneType = MOMZONETYPE_CP;
            }
            else if (FStrEq(name, "end"))
            {
                zoneType = MOMZONETYPE_STOP;
            }
            else if (FStrEq(name, "onehop"))
            {
                zoneType = MOMZONETYPE_ONEHOP;
            }
            else if (FStrEq(name, "resetonehop"))
            {
                zoneType = MOMZONETYPE_RESETONEHOP;
            }
            else if (FStrEq(name, "checkpoint_teleport"))
            {
                zoneType = MOMZONETYPE_CPTELE;
            }
            else if (FStrEq(name, "multihop"))
            {
                zoneType = MOMZONETYPE_MULTIHOP;
            }
            else if (FStrEq(name, "stage"))
            {
                zoneType = MOMZONETYPE_STAGE;
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
        toReturn = true;
    }
    zoneKV->deleteThis();
    return toReturn;
}

bool ZoneTypeToClass(int type, char *dest, int maxlen)
{
    switch (type)
    {
    case MOMZONETYPE_START:
        Q_strncpy(dest, "trigger_momentum_timer_start", maxlen);
        return true;
    case MOMZONETYPE_CP:
        Q_strncpy(dest, "trigger_momentum_timer_checkpoint", maxlen);
        return true;
    case MOMZONETYPE_STOP:
        Q_strncpy(dest, "trigger_momentum_timer_stop", maxlen);
        return true;
    case MOMZONETYPE_ONEHOP:
        Q_strncpy(dest, "trigger_momentum_onehop", maxlen);
        return true;
    case MOMZONETYPE_RESETONEHOP:
        Q_strncpy(dest, "trigger_momentum_timer_resetonehop", maxlen);
        return true;
    case MOMZONETYPE_CPTELE:
        Q_strncpy(dest, "trigger_momentum_teleport_checkpoint", maxlen);
        return true;
    case MOMZONETYPE_MULTIHOP:
        Q_strncpy(dest, "trigger_momentum_multihop", maxlen);
        return true;
    case MOMZONETYPE_STAGE:
        Q_strncpy(dest, "trigger_momentum_timer_stage", maxlen);
        return true;
    }

    return false;
}