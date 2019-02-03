#include "cbase.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "mapzones_build.h"

#include "tier0/memdbgon.h"

#define NO_LOOK -190.0f

CMapzone::CMapzone(const int iType, const Vector &vPos, const QAngle &vRot, const Vector &vScaleMins, const Vector &vScaleMaxs,
                    const int iIndex, const bool bShouldStop,
                    const bool bShouldTilt, const float flHoldTime,
                    const bool bLimitSpeed, const float flBhopLeaveSpeed,
                    const float flYaw, const string_t szLinkedEnt,
                    const bool bCheckOnlyXY, const int iStartZoneNumber,
                    const int iEndZoneNumber, const int iLimitType,
                    const bool bStartJump,
                    const CUtlVector<Vector> &points, float flHeight)
{
    m_iType = iType;
    m_vecPos = vPos;
    m_angRot = vRot;
    m_vecScaleMins = vScaleMins;
    m_vecScaleMaxs = vScaleMaxs;
    m_iIndex = iIndex;
    m_bShouldStopOnTeleport = bShouldStop;
    m_bShouldResetAngles = bShouldTilt;
    m_flHoldTimeBeforeTeleport = flHoldTime;
    m_bLimitingSpeed = bLimitSpeed;
    m_flBhopLeaveSpeed = flBhopLeaveSpeed;
    m_flYaw = flYaw;
    m_szLinkedEnt = szLinkedEnt;
    m_bOnlyXYCheck = bCheckOnlyXY;
    m_iStartZoneNumber = iStartZoneNumber;
    m_iEndZoneNumber = iEndZoneNumber;
    m_iLimitType = iLimitType;
    m_bLimitBhop = false;
    m_flMaxLeaveSpeed = 0.0f;
    m_bStartOnJump = bStartJump;
    m_pTrigger = nullptr;
    m_vZonePoints.CopyArray(points.Base(), points.Count());
    m_flPointZoneHeight = flHeight;
}

void CMapzone::SpawnZone()
{
    switch (m_iType)
    {
    case MOMZONETYPE_START:
    {
        auto zone = (CTriggerTimerStart *)CreateEntityByName("trigger_momentum_timer_start");

        zone->SetIsLimitingSpeed(m_bLimitingSpeed);
        zone->SetMaxLeaveSpeed(m_flBhopLeaveSpeed);
        zone->SetZoneNumber(m_iStartZoneNumber);
        zone->LimitSpeedType() = m_iLimitType;
        zone->StartOnJump() = m_bStartOnJump;
        if (m_flYaw != NO_LOOK)
        {
            zone->SetHasLookAngles(true);
            zone->SetLookAngles(QAngle(0, m_flYaw, 0));
        }
        else
        {
            zone->SetHasLookAngles(false);
        }
        zone->SetName(MAKE_STRING("Start Trigger"));
        g_pMomentumTimer->SetStartTrigger(zone);

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_CP:
    {
        auto zone = (CTriggerCheckpoint *)CreateEntityByName("trigger_momentum_timer_checkpoint");

        zone->SetCheckpointNumber(m_iIndex);
        zone->SetName(MAKE_STRING("Checkpoint Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_STOP:
    {
        auto zone = (CTriggerTimerStop *)CreateEntityByName("trigger_momentum_timer_stop");

        zone->SetZoneNumber(m_iEndZoneNumber);
        zone->SetName(MAKE_STRING("End Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_ONEHOP:
    {
        auto zone = (CTriggerOnehop *)CreateEntityByName("trigger_momentum_onehop");

        zone->m_target = m_szLinkedEnt;
        // zone->SetDestinationIndex(m_destinationIndex);
        // zone->SetDestinationName(m_linkedtrigger);
        zone->SetHoldTeleportTime(m_flHoldTimeBeforeTeleport);
        zone->SetShouldStopPlayer(m_bShouldStopOnTeleport);
        zone->SetShouldResetAngles(m_bShouldResetAngles);
        zone->SetName(MAKE_STRING("Onehop Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_RESETONEHOP:
    {
        auto zone = (CTriggerResetOnehop *)CreateEntityByName("trigger_momentum_resetonehop");

        zone->SetName(MAKE_STRING("ResetOnehop Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_CPTELE:
    {
        auto zone = (CTriggerTeleportCheckpoint *)CreateEntityByName("trigger_momentum_teleport_checkpoint");

        zone->m_target = m_szLinkedEnt;
        // zone->SetDestinationCheckpointNumber(m_destinationIndex);
        // zone->SetDestinationCheckpointName(m_linkedtrigger);
        zone->SetShouldStopPlayer(m_bShouldStopOnTeleport);
        zone->SetShouldResetAngles(m_bShouldResetAngles);
        zone->SetName(MAKE_STRING("TeleportToCheckpoint Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_MULTIHOP:
    {
        auto zone = (CTriggerMultihop *)CreateEntityByName("trigger_momentum_onehop");

        zone->m_target = m_szLinkedEnt;
        // zone->SetDestinationIndex(m_destinationIndex);
        // zone->SetDestinationName(m_szLinkedEnt);
        zone->SetHoldTeleportTime(m_flHoldTimeBeforeTeleport);
        zone->SetShouldStopPlayer(m_bShouldStopOnTeleport);
        zone->SetShouldResetAngles(m_bShouldResetAngles);
        m_pTrigger->SetName(MAKE_STRING("Mutihop Trigger"));

        m_pTrigger = zone;
        break;
    }
    case MOMZONETYPE_STAGE:
    {
        auto zone = (CTriggerStage *)CreateEntityByName("trigger_momentum_timer_stage");

        zone->SetStageNumber(m_iIndex);
        zone->SetName(MAKE_STRING("Stage Trigger"));

        m_pTrigger = zone;
        break;
    }
    // MOM_TODO: add trigger_momentum_teleport, and momentum_trigger_userinput
    default:
    {
        AssertMsg(false, "Unhandled zone type");
        break;
    }
    }

    if (m_pTrigger)
    {
        auto pMomTrigger = static_cast<CBaseMomentumTrigger *>(m_pTrigger);


        // HACK - STREAMLINE ME
        CMomBaseZoneBuilder* pBaseBuilder;

        if (m_vZonePoints.Count() > 0)
        {
            auto pBuilder = new CMomPointZoneBuilder;

            pBuilder->CopyPoints(m_vZonePoints);
            pBuilder->SetHeight(m_flPointZoneHeight);

            pBaseBuilder = pBuilder;
        }
        else
        {
            auto pBuilder = new CMomBoxZoneBuilder;

            pBuilder->SetBounds(m_vecPos, m_vecScaleMins, m_vecScaleMaxs);

            pBaseBuilder = pBuilder;
        }

        m_pTrigger->Spawn();

        pBaseBuilder->BuildZone();
        pBaseBuilder->FinishZone(pMomTrigger);

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
        KeyValues *subKey = nullptr;
        if (pEnt->ClassMatches("trigger_momentum_timer_start"))
        {
            CTriggerTimerStart *pTrigger = dynamic_cast<CTriggerTimerStart *>(pEnt);
            subKey = new KeyValues("start");
            if (pTrigger)
            {
                subKey->SetFloat("bhopleavespeed", pTrigger->GetMaxLeaveSpeed());
                subKey->SetBool("limitingspeed", pTrigger->IsLimitingSpeed());
                subKey->SetBool("StartOnJump", pTrigger->StartOnJump());
                subKey->SetInt("LimitSpeedType", pTrigger->LimitSpeedType());
                subKey->SetInt("ZoneNumber", pTrigger->GetZoneNumber());
                if (pTrigger->HasLookAngles())
                    subKey->SetFloat("yaw", pTrigger->GetLookAngles()[YAW]);
            }
        }
        else if (pEnt->ClassMatches("trigger_momentum_timer_stop"))
        {
            CTriggerTimerStop *pTrigger = dynamic_cast<CTriggerTimerStop *>(pEnt);
            subKey = new KeyValues("end");
            subKey->SetInt("ZoneNumber", pTrigger->GetZoneNumber());
        }
        else if (pEnt->ClassMatches("trigger_momentum_timer_checkpoint"))
        {
            CTriggerCheckpoint *pTrigger = dynamic_cast<CTriggerCheckpoint *>(pEnt);
            if (pTrigger)
            {
                subKey = new KeyValues("checkpoint");
                subKey->SetInt("number", pTrigger->GetCheckpointNumber());
            }
        }
        else if (pEnt->ClassMatches("trigger_momentum_onehop"))
        {
            CTriggerOnehop *pTrigger = dynamic_cast<CTriggerOnehop *>(pEnt);
            if (pTrigger)
            {
                subKey = new KeyValues("onehop");
                // subKey->SetInt("destination", pTrigger->GetDestinationIndex());
                subKey->SetBool("stop", pTrigger->ShouldStopPlayer());
                subKey->SetBool("resetang", pTrigger->ShouldResetAngles());
                subKey->SetFloat("hold", pTrigger->GetHoldTeleportTime());
                subKey->SetString("destinationname", pTrigger->m_target.ToCStr());
            }
        }
        else if (pEnt->ClassMatches("trigger_momentum_resetonehop"))
        {
            subKey = new KeyValues("resetonehop");
        }
        else if (pEnt->ClassMatches("trigger_momentum_teleport_checkpoint"))
        {

            CTriggerTeleportCheckpoint *pTrigger = dynamic_cast<CTriggerTeleportCheckpoint *>(pEnt);
            if (pTrigger)
            {
                subKey = new KeyValues("checkpoint_teleport");
                // subKey->SetInt("destination", pTrigger->GetDestinationCheckpointNumber());
                subKey->SetBool("stop", pTrigger->ShouldStopPlayer());
                subKey->SetBool("resetang", pTrigger->ShouldResetAngles());
                subKey->SetString("destinationname", pTrigger->m_target.ToCStr());
            }
        }
        else if (pEnt->ClassMatches("trigger_momentum_multihop"))
        {
            CTriggerMultihop *pTrigger = dynamic_cast<CTriggerMultihop *>(pEnt);
            if (pTrigger)
            {
                subKey = new KeyValues("multihop");
                // subKey->SetInt("destination", pTrigger->GetDestinationIndex());
                subKey->SetBool("stop", pTrigger->ShouldStopPlayer());
                subKey->SetFloat("hold", pTrigger->GetHoldTeleportTime());
                subKey->SetBool("resetang", pTrigger->ShouldResetAngles());
                subKey->SetString("destinationname", pTrigger->m_target.ToCStr());
            }
        }
        else if (pEnt->ClassMatches("trigger_momentum_timer_stage"))
        {
            CTriggerStage *pTrigger = dynamic_cast<CTriggerStage *>(pEnt);
            if (pTrigger)
            {
                subKey = new KeyValues("stage");
                subKey->SetInt("number", pTrigger->GetStageNumber());
            }
        }
        if (subKey)
        {
            auto pMomTrigger = static_cast<CBaseMomentumTrigger*>(pEnt);

            CMomBaseZoneBuilder *pBuilder = CreateZoneBuilderFromExisting(pMomTrigger);

            if (!pBuilder->Save(subKey))
            {
                Warning("Failed to save zone to file!\n");
            }

            delete pBuilder;

            zoneKV->AddSubKey(subKey);
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
        Log("Unable to find map zones! Trying to create them...\n");
        SaveZonFile(szMapName); // try making the zon file if the map has the entities
        LoadFromFile(szMapName);
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

bool CMapzoneData::MapZoneSpawned(CMapzone *pZone)
{
    if (!pZone)
        return false;

    char name[128];
    if (!ZoneTypeToClass(pZone->GetType(), name))
        return false;

    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, name);
    while (pEnt)
    {
        if (pEnt->GetAbsOrigin() == pZone->GetPosition() && pEnt->GetAbsAngles() == pZone->GetRotation())
        {
            // Only check WorldAlignMaxs/Mins if collision prop bounds are not in entity space, to avoid assertions
            if (pEnt->CollisionProp()->IsBoundsDefinedInEntitySpace() ||
                (pEnt->WorldAlignMaxs() == pZone->GetScaleMaxs() && pEnt->WorldAlignMins() == pZone->GetScaleMins()))
            {
                DevLog("Already found a %s spawned on the map! Not spawning it from zone file...\n", name);
                return true;
            }
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, name);
    }

    return false;
}

void CMapzoneData::SpawnMapZones()
{
    int count = m_zones.Count();
    for (int i = 0; i < count; i++)
    {
        if (m_zones[i])
        {
            // if the zone already exists (placed in map by Hammer), don't spawn it
            if (!MapZoneSpawned(m_zones[i]))
                m_zones[i]->SpawnZone();
        }
    }
}

bool CMapzoneData::LoadFromFile(const char *szMapName)
{
    bool toReturn = false;
    char zoneFilePath[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
    V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
    DevLog("Looking for zone file: %s \n", zoneFilePath);
    KeyValues *zoneKV = new KeyValues(szMapName);
    if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "MOD"))
    {
        // Go through checkpoints
        for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
        {
            // HACK - STREAMLINE ME
            CUtlVector<Vector> points;
            float pointzoneheight = 0.0f;
            if (cp->FindKey("point_points"))
            {
                auto pBuilder = new CMomPointZoneBuilder();
                pBuilder->Load(cp);

                pointzoneheight = pBuilder->GetHeight();
                points.CopyArray(pBuilder->GetPoints().Base(), pBuilder->GetPoints().Count());

                delete pBuilder;
            }

            // Load position information (will default to 0 if the keys don't exist)
            Vector pos(cp->GetFloat("xPos"), cp->GetFloat("yPos"), cp->GetFloat("zPos"));
            QAngle rot(cp->GetFloat("xRot"), cp->GetFloat("yRot"), cp->GetFloat("zRot"));
            Vector scaleMins(cp->GetFloat("xScaleMins"), cp->GetFloat("yScaleMins"), cp->GetFloat("zScaleMins"));
            Vector scaleMaxs(cp->GetFloat("xScaleMaxs"), cp->GetFloat("yScaleMaxs"), cp->GetFloat("zScaleMaxs"));

            // Do specific things for different types of checkpoints
            // 0 = start, 1 = checkpoint, 2 = end, 3 = Onehop, 4 = OnehopReset, 5 = Checkpoint_teleport, 6 = Multihop, 7
            // = stage
            int zoneType = -1;
            int index = -1;
            bool shouldStop = false;
            bool shouldTilt = true;
            float holdTime = 1.0f;
            // int destinationIndex = -1;
            bool limitingspeed = true;
            bool checkonlyxy = true;
            float bhopleavespeed = 250.0f;
            const char *linkedtrigger = nullptr;

            float start_yaw = NO_LOOK;
            int ilimittype = 0;
            int istartzonenumber = 0;
            int iendzonenumber = 0;
            bool bstartonjump = false;

            if (Q_strcmp(cp->GetName(), "start") == 0)
            {
                zoneType = MOMZONETYPE_START;
                limitingspeed = cp->GetBool("limitingspeed");
                bhopleavespeed = cp->GetFloat("bhopleavespeed");
                start_yaw = cp->GetFloat("yaw", NO_LOOK);
                ilimittype = cp->GetInt("LimitSpeedType");
                istartzonenumber = cp->GetInt("ZoneNumber");
                bstartonjump = cp->GetBool("StartOnJump");
            }
            else if (Q_strcmp(cp->GetName(), "checkpoint") == 0)
            {
                zoneType = MOMZONETYPE_CP;
                index = cp->GetInt("number", -1);
            }
            else if (Q_strcmp(cp->GetName(), "end") == 0)
            {
                iendzonenumber = cp->GetInt("ZoneNumber");
                zoneType = MOMZONETYPE_STOP;
            }
            else if (Q_strcmp(cp->GetName(), "onehop") == 0)
            {
                zoneType = MOMZONETYPE_ONEHOP;
                shouldStop = cp->GetBool("stop", false);
                shouldTilt = cp->GetBool("resetang", true);
                holdTime = cp->GetFloat("hold", 1);
                // destinationIndex = cp->GetInt("destination", 1);
                linkedtrigger = cp->GetString("destinationname", nullptr);
            }
            else if (Q_strcmp(cp->GetName(), "resetonehop") == 0)
            {
                zoneType = MOMZONETYPE_RESETONEHOP;
            }
            else if (Q_strcmp(cp->GetName(), "checkpoint_teleport") == 0)
            {
                zoneType = MOMZONETYPE_CPTELE;
                // destinationIndex = cp->GetInt("destination", -1);
                shouldStop = cp->GetBool("stop", false);
                shouldTilt = cp->GetBool("resetang", true);
                linkedtrigger = cp->GetString("destinationname", nullptr);
            }
            else if (Q_strcmp(cp->GetName(), "multihop") == 0)
            {
                zoneType = MOMZONETYPE_MULTIHOP;
                shouldStop = cp->GetBool("stop", false);
                shouldTilt = cp->GetBool("resetang", true);
                holdTime = cp->GetFloat("hold", 1);
                // destinationIndex = cp->GetInt("destination", 1);
                linkedtrigger = cp->GetString("destinationname", nullptr);
            }
            else if (!Q_strcmp(cp->GetName(), "stage") || !Q_strcmp(cp->GetName(), "zone"))
            {
                zoneType = MOMZONETYPE_STAGE;
                index = cp->GetInt("number", 0);
            }
            else
            {
                Warning("Error while reading zone file: Unknown mapzone type %s!\n", cp->GetName());
                continue;
            }

            // Add element
            m_zones.AddToTail(new CMapzone(zoneType, pos, rot, scaleMins, scaleMaxs, index, shouldStop, shouldTilt,
                                           holdTime, limitingspeed, bhopleavespeed, start_yaw,
                                           MAKE_STRING(linkedtrigger), checkonlyxy, istartzonenumber, iendzonenumber,
                                           ilimittype,bstartonjump, points, pointzoneheight));
        }
        DevLog("Successfully loaded map zone file %s!\n", zoneFilePath);
        toReturn = true;
    }
    zoneKV->deleteThis();
    return toReturn;
}

bool ZoneTypeToClass(int type, char *dest)
{
    switch (type)
    {
    case MOMZONETYPE_START:
        Q_strcpy(dest, "trigger_momentum_timer_start");
        return true;
    case MOMZONETYPE_CP:
        Q_strcpy(dest, "trigger_momentum_timer_checkpoint");
        return true;
    case MOMZONETYPE_STOP:
        Q_strcpy(dest, "trigger_momentum_timer_stop");
        return true;
    case MOMZONETYPE_ONEHOP:
        Q_strcpy(dest, "trigger_momentum_onehop");
        return true;
    case MOMZONETYPE_RESETONEHOP:
        Q_strcpy(dest, "trigger_momentum_timer_resetonehop");
        return true;
    case MOMZONETYPE_CPTELE:
        Q_strcpy(dest, "trigger_momentum_teleport_checkpoint");
        return true;
    case MOMZONETYPE_MULTIHOP:
        Q_strcpy(dest, "trigger_momentum_multihop");
        return true;
    case MOMZONETYPE_STAGE:
        Q_strcpy(dest, "trigger_momentum_timer_stage");
        return true;
    }

    return false;
}