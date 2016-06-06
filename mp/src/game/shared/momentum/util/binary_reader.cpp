#include "cbase.h"
#include "binary_reader.h"

BinaryReader::BinaryReader(FileHandle_t file, bool close /* = false */) :
	m_File(file),
	m_ShouldClose(close),
	m_ShouldFlipEndianness(false)
{
}

BinaryReader::~BinaryReader()
{
	if (m_ShouldClose)
		filesystem->Close(m_File);
}

bool BinaryReader::ReadBool()
{
	return ReadUInt8() != 0;
}

int8 BinaryReader::ReadInt8()
{
	uint8 data[1];
	ReadData(data, 1);

	return *(int8*) data;
}

uint8 BinaryReader::ReadUInt8()
{
	uint8 data[1];
	ReadData(data, 1);

	return *(uint8*) data;
}

int16 BinaryReader::ReadInt16()
{
	uint8 data[2];
	ReadData(data, 2);

	return static_cast<int16>(FixEndianness16(*(uint16*) data));
}

uint16 BinaryReader::ReadUInt16()
{
	uint8 data[2];
	ReadData(data, 2);

	return FixEndianness16(*(uint16*) data);
}

int32 BinaryReader::ReadInt32()
{
	uint8 data[4];
	ReadData(data, 4);

	return static_cast<int32>(FixEndianness32(*(uint32*) data));
}

uint32 BinaryReader::ReadUInt32()
{
	uint8 data[4];
	ReadData(data, 4);

	return FixEndianness32(*(uint32*) data);
}

int64 BinaryReader::ReadInt64()
{
	uint8 data[8];
	ReadData(data, 8);

	return static_cast<int64>(FixEndianness64(*(uint64*) data));
}

uint64 BinaryReader::ReadUInt64()
{
	uint8 data[8];
	ReadData(data, 8);

	return FixEndianness64(*(uint64*) data);
}

float BinaryReader::ReadFloat()
{
	uint8 data[4];
	ReadData(data, 4);

	uint32 value = FixEndianness32(*(uint32*) data);
	return *(float*) &value;
}

double BinaryReader::ReadDouble()
{
	uint8 data[8];
	ReadData(data, 8);

	uint64 value = FixEndianness64(*(uint64*) data);
	return *(double*) &value;
}

uint16 BinaryReader::ReadString(char* data)
{
	uint16 length = ReadUInt16();
	ReadData(data, length);
	data[length] = '\0';
	return length;
}

void BinaryReader::ReadData(void* data, int length)
{
	filesystem->Read(data, length, m_File);
}

uint16 BinaryReader::FixEndianness16(uint16 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);
}

uint32 BinaryReader::FixEndianness32(uint32 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 24) & 0x000000FF) | ((data >> 8) & 0x0000FF00) |
		((data << 8) & 0x00FF0000) | ((data << 24) & 0xFF000000);
}

uint64 BinaryReader::FixEndianness64(uint64 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 56) & 0x00000000000000FF) | ((data >> 40) & 0x000000000000FF00) |
		((data >> 24) & 0x0000000000FF0000) | ((data >> 8) & 0x00000000FF000000) | 
		((data << 8) & 0x000000FF00000000) | ((data << 24) & 0x0000FF0000000000) | 
		((data << 40) & 0x00FF000000000000) | ((data << 56) & 0xFF00000000000000);
}

