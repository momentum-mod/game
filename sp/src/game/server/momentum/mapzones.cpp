#include "cbase.h"
#include "mapzones.h"
#include "Timer.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "TimerTriggers.h"

#include "tier0/memdbgon.h"

CMapzone::~CMapzone()
{
	if (m_pos)
	{
		delete m_pos;
		m_pos = NULL;
	}
	if (m_rot)
	{
		delete m_rot;
		m_rot = NULL;
	}
	if (m_scaleMins)
	{
		delete m_scaleMins;
		m_scaleMins = NULL;
	}
	if (m_scaleMaxs)
	{
		delete m_scaleMaxs;
		m_scaleMaxs = NULL;
	}
}

CMapzone::CMapzone(const int pType, Vector* pPos, QAngle* pRot, Vector* pScaleMins, 
    Vector* pScaleMaxs, const int pIndex, const bool pShouldStop, 
    const float pHoldTime, const int pDestinationIndex)
{
	m_type = pType;
	m_pos = pPos;
	m_rot = pRot;
	m_scaleMins = pScaleMins;
	m_scaleMaxs = pScaleMaxs;
	m_index = pIndex;
	m_shouldStopOnTeleport = pShouldStop;
	m_holdTimeBeforeTeleport = pHoldTime;
	m_destinationIndex = pDestinationIndex;
}

void CMapzone::SpawnZone()
{
	switch (m_type)
	{
	case 0://start
		m_trigger = (CTriggerTimerStart *)CreateEntityByName("trigger_timer_start");
		m_trigger->SetName(MAKE_STRING("Start Trigger"));
		g_Timer.SetStartTrigger((CTriggerTimerStart *)m_trigger);
		break;
	case 1://checkpoint
		m_trigger = (CTriggerCheckpoint *)CreateEntityByName("trigger_timer_checkpoint");
		m_trigger->SetName(MAKE_STRING("Checkpoint Trigger"));
		((CTriggerCheckpoint *)m_trigger)->SetCheckpointNumber(m_index);
		break;
	case 2://end
		m_trigger = (CTriggerTimerStop *)CreateEntityByName("trigger_timer_stop");
		m_trigger->SetName(MAKE_STRING("Ending Trigger"));
		break;
	case 3://onehop
		m_trigger = (CTriggerOnehop *)CreateEntityByName("trigger_timer_onehop");
		m_trigger->SetName(MAKE_STRING("Onehop Trigger"));
		((CTriggerOnehop *)m_trigger)->SetDestinationIndex(m_destinationIndex);
		((CTriggerOnehop *)m_trigger)->SetHoldTeleportTime(m_holdTimeBeforeTeleport);
		((CTriggerOnehop *)m_trigger)->SetShouldStopPlayer(m_shouldStopOnTeleport);
		break;
	case 4://resetonehop
		m_trigger = (CTriggerResetOnehop *)CreateEntityByName("trigger_timer_resetonehop");
		m_trigger->SetName(MAKE_STRING("ResetOnehop Trigger"));
		break;
	case 5://checkpoint_teleport
		m_trigger = (CTriggerTeleportCheckpoint *)CreateEntityByName("trigger_timer_checkpoint_teleport");
		m_trigger->SetName(MAKE_STRING("TeleportToCheckpoint Trigger"));
		((CTriggerTeleportCheckpoint *)m_trigger)->SetDestinationCheckpointNumber(m_destinationIndex);
		((CTriggerTeleportCheckpoint *)m_trigger)->SetShouldStopPlayer(m_shouldStopOnTeleport);
	default:
		break;
	}

	if (m_trigger)
	{
		m_trigger->Spawn();
		m_trigger->Activate();
		m_trigger->SetAbsOrigin(*m_pos);
		m_trigger->SetSize(*m_scaleMins, *m_scaleMaxs);
		m_trigger->SetAbsAngles(*m_rot);
		m_trigger->SetSolid(SOLID_BBOX);
	}
}

static void saveZonFile(const char* szMapName)
{
	KeyValues* zoneKV = new KeyValues(szMapName);
	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "trigger_timer_*");
	while (pEnt)
	{
		KeyValues* subKey = NULL;
		if (pEnt->ClassMatches("trigger_timer_start"))
		{
			subKey = new KeyValues("start");
		}
		else if (pEnt->ClassMatches("trigger_timer_stop"))
		{
			subKey = new KeyValues("end");
		}
		else if (pEnt->ClassMatches("trigger_timer_checkpoint"))
		{
			CTriggerCheckpoint* pTrigger = dynamic_cast<CTriggerCheckpoint*>(pEnt);
			if (pTrigger)
			{
				subKey = new KeyValues("checkpoint");
				subKey->SetInt("number", pTrigger->GetCheckpointNumber());
			}
		}
		else if (pEnt->ClassMatches("trigger_timer_onehop"))
		{
			CTriggerOnehop* pTrigger = dynamic_cast<CTriggerOnehop*>(pEnt);
			if (pTrigger)
			{
				subKey = new KeyValues("onehop");
				subKey->SetInt("destination", pTrigger->GetDestinationIndex());
				subKey->SetBool("stop", pTrigger->GetShouldStopPlayer());
				subKey->SetFloat("hold", pTrigger->GetHoldTeleportTime());
			}
		}
		else if (pEnt->ClassMatches("trigger_timer_resetonehop"))
		{
			subKey = new KeyValues("resetonehop");
		}
		else if (pEnt->ClassMatches("trigger_timer_checkpoint_teleport"))
		{
			
			CTriggerTeleportCheckpoint* pTrigger = dynamic_cast<CTriggerTeleportCheckpoint*>(pEnt);
			if (pTrigger)
			{
				subKey = new KeyValues("checkpoint_teleport");
				subKey->SetInt("destination", pTrigger->GetDestinationCheckpointNumber());
				subKey->SetBool("stop", pTrigger->GetShouldStopPlayer());
			}
		}
		if (subKey)
		{
			subKey->SetFloat("xPos", pEnt->GetAbsOrigin().x);
			subKey->SetFloat("yPos", pEnt->GetAbsOrigin().y);
			subKey->SetFloat("zPos", pEnt->GetAbsOrigin().z);
			subKey->SetFloat("xRot", pEnt->GetAbsAngles().x);
			subKey->SetFloat("yRot", pEnt->GetAbsAngles().y);
			subKey->SetFloat("zRot", pEnt->GetAbsAngles().z);
			subKey->SetFloat("xScaleMins", pEnt->WorldAlignMins().x);
			subKey->SetFloat("yScaleMins", pEnt->WorldAlignMins().y);
			subKey->SetFloat("zScaleMins", pEnt->WorldAlignMins().z);
			subKey->SetFloat("xScaleMaxs", pEnt->WorldAlignMaxs().x);
			subKey->SetFloat("yScaleMaxs", pEnt->WorldAlignMaxs().y);
			subKey->SetFloat("zScaleMaxs", pEnt->WorldAlignMaxs().z);
			zoneKV->AddSubKey(subKey);
		}
		pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_timer_*");
	}
	if (zoneKV->GetFirstSubKey())//not empty 
	{
		char zoneFilePath[MAX_PATH];
		Q_strcpy(zoneFilePath, "maps/");
		Q_strcat(zoneFilePath, szMapName, MAX_PATH);
		Q_strncat(zoneFilePath, ".zon", MAX_PATH);
		zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
		zoneKV->deleteThis();
	}
}

CMapzoneData::CMapzoneData(const char *szMapName)
{
    if (!LoadFromFile(szMapName))
    {
        Log("Unable to find map zones! Trying to create them...\n");
        saveZonFile(szMapName);//try making the zon file if the map has the entities
        LoadFromFile(szMapName);
    }
}

//MOM_TODO: Get rid of the following method and ConCommand
static void saveZonFile_f() {
	saveZonFile(gpGlobals->mapname.ToCStr());
}

static ConCommand mom_generate_zone_file("mom_generate_zone_file", saveZonFile_f, "Generates a zone file.");

CMapzoneData::~CMapzoneData()
{
	if (!m_zones.IsEmpty())
	{
		m_zones.PurgeAndDeleteElements();
	}	
}

bool CMapzoneData::MapZoneSpawned(CMapzone *mZone)
{
    bool toReturn = false;
    if (!mZone) return false;

    char name[128];
    switch (mZone->GetType())
    {
    case 0:
        Q_strcpy(name, "trigger_timer_start");
        break;
    case 1:
        Q_strcpy(name, "trigger_timer_checkpoint");
        break;
    case 2:
        Q_strcpy(name, "trigger_timer_stop");
        break;
	case 3:
		Q_strcpy(name, "trigger_timer_onehop");
		break;
	case 4:
		Q_strcpy(name, "trigger_timer_resetonehop");
		break;
	case 5:
		Q_strcpy(name, "trigger_timer_checkpoint_teleport");
		break;
    default:
        return false;
    }

    CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, name);
    while (pEnt)
    {
        if (pEnt->GetAbsOrigin() == *mZone->GetPosition()
            && pEnt->GetAbsAngles() == *mZone->GetRotation()
            && pEnt->WorldAlignMaxs() == *mZone->GetScaleMaxs()
            && pEnt->WorldAlignMins() == *mZone->GetScaleMins())
        {
            DevLog("Already found a %s spawned on the map! Not spawning it from zone file...\n", name);
            toReturn = true;
            break;
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, name);
    }

    return toReturn;
}

void CMapzoneData::SpawnMapZones()
{
    int count = m_zones.Count();
	for (int i = 0; i < count; i++)
	{
        if (m_zones[i])
        {
            //if the zone already exists (placed in map by Hammer), don't spawn it
            if (!MapZoneSpawned(m_zones[i]))
                m_zones[i]->SpawnZone();
        }	
	}
}

bool CMapzoneData::LoadFromFile(const char *szMapName)
{
    bool toReturn = false;
    char zoneFilePath[MAX_PATH];
    Q_strcpy(zoneFilePath, c_mapPath);
    Q_strcat(zoneFilePath, szMapName, MAX_PATH);
    Q_strncat(zoneFilePath, c_zoneFileEnding, MAX_PATH);
    DevLog("Looking for zone file: %s \n", zoneFilePath);
    KeyValues* zoneKV = new KeyValues(szMapName);
    if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "MOD"))
    {
        // Go through checkpoints
        for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
        {
            // Load position information (will default to 0 if the keys don't exist)
            Vector* pos = new Vector(cp->GetFloat("xPos"), cp->GetFloat("yPos"), cp->GetFloat("zPos"));
            QAngle* rot = new QAngle(cp->GetFloat("xRot"), cp->GetFloat("yRot"), cp->GetFloat("zRot"));
            Vector* scaleMins = new Vector(cp->GetFloat("xScaleMins"), cp->GetFloat("yScaleMins"), cp->GetFloat("zScaleMins"));
            Vector* scaleMaxs = new Vector(cp->GetFloat("xScaleMaxs"), cp->GetFloat("yScaleMaxs"), cp->GetFloat("zScaleMaxs"));

            // Do specific things for different types of checkpoints
			// 0 = start, 1 = checkpoint, 2 = end, 3 = Onehop, 4 = OnehopReset, 5 = Checkpoint_teleport
            int zoneType = -1;
            int index = -1;
			bool shouldStop = false;
			float holdTime = 1.0f;
			int destinationIndex = -1;

            if (Q_strcmp(cp->GetName(), "start") == 0)
            {
                zoneType = 0;
            }
            else if (Q_strcmp(cp->GetName(), "checkpoint") == 0)
            {
                zoneType = 1;
                index = cp->GetInt("index");
            }
            else if (Q_strcmp(cp->GetName(), "end") == 0)
            {
                zoneType = 2;
			}
			else if (Q_strcmp(cp->GetName(), "onehop") == 0)
			{
				zoneType = 3;
				shouldStop = cp->GetBool("stop", false);
				holdTime = cp->GetFloat("hold", 1);
				destinationIndex = cp->GetInt("destination", 1);
			}
			else if (Q_strcmp(cp->GetName(), "resetonehop") == 0)
			{
				zoneType = 4;
			}
			else if (Q_strcmp(cp->GetName(), "checkpoint_teleport") == 0)
			{
				zoneType = 5;
				destinationIndex = cp->GetInt("destination", -1);
				shouldStop = cp->GetBool("stop", false);
			}
            else
            {
                Warning("Error while reading zone file: Unknown mapzone type %s!\n",cp->GetName());
                continue;
            }

            // Add element
			m_zones.AddToTail(new CMapzone(zoneType, pos, rot, scaleMins, scaleMaxs, index, shouldStop, 
                holdTime, destinationIndex));
        }
        DevLog("Successfully loaded map zone file %s!\n", zoneFilePath);
        toReturn = true;
    }
    zoneKV->deleteThis();
    return toReturn;
}