#pragma once

#include "cbase.h"
#include "filesystem.h"

class BinaryWriter
{
public:
	BinaryWriter(FileHandle_t file, bool close = false);
	~BinaryWriter();

public:
	inline void ShouldFlipEndianness(bool flip) { m_ShouldFlipEndianness = flip; }
	inline void Seek(int position, FileSystemSeek_t seek) { filesystem->Seek(m_File, position, seek); }

public:
	void WriteBool(bool data);
	void WriteInt8(int8 data);
	void WriteUInt8(uint8 data);
	void WriteInt16(int16 data);
	void WriteUInt16(uint16 data);
	void WriteInt32(int32 data);
	void WriteUInt32(uint32 data);
	void WriteInt64(int64 data);
	void WriteUInt64(uint64 data);
	void WriteFloat(float data);
	void WriteDouble(double data);
	void WriteString(const char* data);
	void WriteData(const void* data, int length);

private:
	uint16 FixEndianness16(uint16 data);
	uint32 FixEndianness32(uint32 data);
	uint64 FixEndianness64(uint64 data);

private:
	FileHandle_t m_File;
	bool m_ShouldClose;
	bool m_ShouldFlipEndianness;
};