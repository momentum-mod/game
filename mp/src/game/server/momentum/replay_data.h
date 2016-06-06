#pragma once

#include "cbase.h"
#include "util/serialization.h"
#include "util/run_stats.h"

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52

// A single frame of the replay.
class ReplayFrame : 
	public ISerializable
{
public:
	ReplayFrame() :
		m_qEyeAngles(0, 0, 0),
		m_vPlayerOrigin(0, 0, 0),
		m_nPlayerButtons(0)
	{
	}

	ReplayFrame(BinaryReader* reader)
	{
		m_qEyeAngles.x = reader->ReadFloat();
		m_qEyeAngles.y = reader->ReadFloat();
		m_qEyeAngles.z = reader->ReadFloat();

		m_vPlayerOrigin.x = reader->ReadFloat();
		m_vPlayerOrigin.y = reader->ReadFloat();
		m_vPlayerOrigin.z = reader->ReadFloat();

		m_nPlayerButtons = reader->ReadInt32();
	}

	ReplayFrame(const QAngle& eye, const Vector& origin, int buttons) :
		m_qEyeAngles(eye),
		m_vPlayerOrigin(origin),
		m_nPlayerButtons(buttons)
	{
	}

public:
	virtual void Serialize(BinaryWriter* writer) override
	{
		writer->WriteFloat(m_qEyeAngles.x);
		writer->WriteFloat(m_qEyeAngles.y);
		writer->WriteFloat(m_qEyeAngles.z);

		writer->WriteFloat(m_vPlayerOrigin.x);
		writer->WriteFloat(m_vPlayerOrigin.y);
		writer->WriteFloat(m_vPlayerOrigin.z);

		writer->WriteInt32(m_nPlayerButtons);
	}

public:
	inline QAngle EyeAngles() const { return m_qEyeAngles; }
	inline Vector PlayerOrigin() const { return m_vPlayerOrigin; }
	inline int PlayerButtons() const { return m_nPlayerButtons; }

private:
	QAngle m_qEyeAngles;
	Vector m_vPlayerOrigin;
	int m_nPlayerButtons;
};

class ReplayHeader :
	public ISerializable
{
public:
	ReplayHeader()
	{
	}
	
	ReplayHeader(BinaryReader* reader)
	{
		m_Version = reader->ReadUInt8();
		reader->ReadString(m_MapName, sizeof(m_MapName) - 1);
		reader->ReadString(m_PlayerName, sizeof(m_PlayerName) - 1);
		m_SteamID = reader->ReadUInt64();
		m_TickInterval = reader->ReadFloat();
		m_RunTime = reader->ReadFloat();
		m_RunFlags = reader->ReadInt32();
	}

public:
	virtual void Serialize(BinaryWriter* writer) override
	{
		writer->WriteUInt8(m_Version);
		writer->WriteString(m_MapName);
		writer->WriteString(m_PlayerName);
		writer->WriteUInt64(m_SteamID);
		writer->WriteFloat(m_TickInterval);
		writer->WriteFloat(m_RunTime);
		writer->WriteInt32(m_RunFlags);
	}

public:
	uint8 m_Version; // The version of the replay.
	char m_MapName[256]; // The map the run was done in.
	char m_PlayerName[256]; // The name of the player that did this run.
	uint64 m_SteamID; // The steamID of the player that did this run.
	float m_TickInterval; // The tickrate of the run.
	float m_RunTime; // The total runtime of the run in seconds.
	int m_RunFlags; // The flags the player ran with.
};