#include "cbase.h"
#include "mapzones.h"
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
	if (m_scale)
	{
		delete m_scale;
		m_scale = NULL;
	}
}

CMapzone::CMapzone(const int pType, Vector* pPos, Vector* pRot, Vector* pScale, const int pIndex)
{
	m_type = pType;
	m_pos = pPos;
	m_rot = pRot;
	m_scale = pScale;
	m_index = pIndex;
}

void CMapzone::SpawnZone()
{
	// TODO: Create trigger
}

void CMapzone::RemoveZone()
{
	// TODO: Remove trigger
}

CMapzoneData::CMapzoneData(const char *szMapName)
{
	// Generate file path for zone file
	char zoneFilePath[MAX_PATH];
	Q_strcpy(zoneFilePath, c_mapPath);
	Q_strcat(zoneFilePath, szMapName, MAX_PATH);
	Q_strncat(zoneFilePath, c_zoneFileEnding, MAX_PATH);

	KeyValues* zoneKV = new KeyValues(szMapName);
	if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "GAME"))
	{
		// Go through checkpoints
		for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
		{
			// Load position information (will default to 0 if the keys don't exist)
			Vector* pos = new Vector(cp->GetInt("xPos"), cp->GetInt("yPos"), cp->GetInt("zPos"));
			Vector* rot = new Vector(cp->GetInt("xRot"), cp->GetInt("yRot"), cp->GetInt("zRot"));
			Vector* scale = new Vector(cp->GetInt("xScale"), cp->GetInt("yScale"), cp->GetInt("zScale"));

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
			m_zones.AddToTail(new CMapzone(zoneType, pos, rot, scale, index));
		}
	}

	zoneKV->deleteThis();
	zoneKV = NULL;
}

CMapzoneData::~CMapzoneData()
{
	RemoveMapZones();
	m_zones.PurgeAndDeleteElements();
}

void CMapzoneData::SpawnMapZones()
{
	for (int i = 0; i < m_zones.Count(); i++)
	{
		if (m_zones[i])
			m_zones[i]->SpawnZone();
	}
}

void CMapzoneData::RemoveMapZones()
{
	for (int i = 0; i < m_zones.Count(); i++)
	{
		if (m_zones[i])
			m_zones[i]->RemoveZone();
	}
}