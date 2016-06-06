#pragma once

#include "cbase.h"
#include "filesystem.h"

class BinaryReader
{
public:
	BinaryReader(FileHandle_t file, bool close = false);
	~BinaryReader();

public:
	inline void ShouldFlipEndianness(bool flip) { m_ShouldFlipEndianness = flip; }
	inline void Seek(int position, FileSystemSeek_t seek) { filesystem->Seek(m_File, position, seek); }

public:
	bool ReadBool();
	int8 ReadInt8();
	uint8 ReadUInt8();
	int16 ReadInt16();
	uint16 ReadUInt16();
	int32 ReadInt32();
	uint32 ReadUInt32();
	int64 ReadInt64();
	uint64 ReadUInt64();
	float ReadFloat();
	double ReadDouble();
	uint16 ReadString(char* data);
	void ReadData(void* data, int length);

private:
	uint16 FixEndianness16(uint16 data);
	uint32 FixEndianness32(uint32 data);
	uint64 FixEndianness64(uint64 data);

private:
	FileHandle_t m_File;
	bool m_ShouldClose;
	bool m_ShouldFlipEndianness;
};