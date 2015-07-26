#include "cbase.h"
#include "mapzones.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "TimerTriggers.h"

#include "tier0/memdbgon.h"

CMapzone::CMapzone()
{
	type = -1;
}

CMapzone::~CMapzone()
{

}

CMapzone::CMapzone(const int pType, const Vector pPos, const Vector pRot, const Vector pScale)
{
	type = pType;
	pos = pPos;
	rot = pRot;
	scale = pScale;
}

CMapzone::CMapzone(const int pType, const Vector pPos, const Vector pRot, const Vector pScale, int pIndex)
{
	type = pType;
	pos = pPos;
	rot = pRot;
	scale = pScale;
	index = pIndex;
}

void CMapzone::SpawnZone()
{
	// TODO: Create trigger
}

void CMapzone::RemoveZone()
{
	// TODO: Remove trigger
}

CMapzoneData::CMapzoneData(const char *pMapName)
{
	// Generate file path for zone file
	char zoneFilePath[MAX_PATH];
	Q_strcpy(zoneFilePath, mapPath);
	Q_strcat(zoneFilePath, pMapName, MAX_PATH);
	Q_strncat(zoneFilePath, zoneFileEnding, MAX_PATH);

	int counter = 0;

	KeyValues* zoneKV = new KeyValues(pMapName);
	if (zoneKV->LoadFromFile(filesystem, zoneFilePath, "GAME"))
	{
		// Count map zones in file
		CMapzoneData::zoneCount = 0;
		for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
		{
			CMapzoneData::zoneCount++;
		}
		CMapzoneData::zones = new CMapzone[CMapzoneData::zoneCount];

		// Go through checkpoints
		for (KeyValues *cp = zoneKV->GetFirstSubKey(); cp; cp = cp->GetNextKey())
		{
			// Load position information (will default to 0 if the keys don't exist)
			int xPos = cp->GetInt("xPos");
			int yPos = cp->GetInt("yPos");
			int zPos = cp->GetInt("zPos");

			int xRot = cp->GetInt("xRot");
			int yRot = cp->GetInt("yRot");
			int zRot = cp->GetInt("zRot");

			int xScale = cp->GetInt("xScale");
			int yScale = cp->GetInt("yScale");
			int zScale = cp->GetInt("zScale");

			// Do specific things for different types of checkpoints
			if (Q_strcmp(cp->GetName(), "start") == 0)
			{
				CMapzoneData::zones[counter] = CMapzone(0, Vector(xPos, yPos, zPos), Vector(xRot, yRot, zRot), Vector(xScale, yScale, zScale));
			}
			else if (Q_strcmp(cp->GetName(), "checkpoint") == 0)
			{
				CMapzoneData::zones[counter] = CMapzone(1, Vector(xPos, yPos, zPos), Vector(xRot, yRot, zRot), Vector(xScale, yScale, zScale), cp->GetInt("index"));
			}
			else if (Q_strcmp(cp->GetName(), "end") == 0)
			{
				CMapzoneData::zones[counter] = CMapzone(2, Vector(xPos, yPos, zPos), Vector(xRot, yRot, zRot), Vector(xScale, yScale, zScale));
			}
			else
			{
				Log("Error while reading zone file! Unknown checkpoint type.");
			}

			counter++;
		}
	}
	zoneKV->deleteThis();
}

CMapzoneData::~CMapzoneData()
{
	RemoveMapZones();
}

void CMapzoneData::SpawnMapZones()
{
	for (int i = 0; i < CMapzoneData::zoneCount; i++)
	{
		zones[i].SpawnZone();
	}
}

void CMapzoneData::RemoveMapZones()
{
	for (int i = 0; i < CMapzoneData::zoneCount; i++)
	{
		zones[i].RemoveZone();
	}
}