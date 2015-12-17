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

CMapzone::CMapzone(const int pType, Vector* pPos, QAngle* pRot, Vector* pScaleMins, Vector* pScaleMaxs, const int pIndex)
{
	m_type = pType;
	m_pos = pPos;
	m_rot = pRot;
	m_scaleMins = pScaleMins;
	m_scaleMaxs = pScaleMaxs;
	m_index = pIndex;
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
	// Generate file path for zone file
	bool recursive = false;
	// MOM_TODO: Do we really need gotos? Spaghetti code will end civilizations
	top:
	char zoneFilePath[MAX_PATH];
	Q_strcpy(zoneFilePath, c_mapPath);
	Q_strcat(zoneFilePath, szMapName, MAX_PATH);
	Q_strncat(zoneFilePath, c_zoneFileEnding, MAX_PATH);
	Log("Looking for zone file: %s \n",zoneFilePath);
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

			// MOM_TODO: Load zone file and store info in memory
			// Do specific things for different types of checkpoints
			int zoneType = -1;
			int index = -1;

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
			else
			{
				Log("Error while reading zone file! Unknown checkpoint type.\n");
			}

			// Add element
			m_zones.AddToTail(new CMapzone(zoneType, pos, rot, scaleMins, scaleMaxs, index));
		}
	}
	else
	{
		if (!recursive) 
		{
			Log("Unable to find map zones! Creating it...\n");
			saveZonFile(szMapName);
			zoneKV->deleteThis();
			zoneKV = NULL;
			recursive = true;
			goto top;
		}	
	}

	zoneKV->deleteThis();
	zoneKV = NULL;
}

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

void CMapzoneData::SpawnMapZones()
{
	for (int i = 0; i < m_zones.Count(); i++)
	{
		if (m_zones[i])
			m_zones[i]->SpawnZone();
	}
}