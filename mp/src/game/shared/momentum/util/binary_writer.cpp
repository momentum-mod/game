#include "cbase.h"
#include "binary_writer.h"

BinaryWriter::BinaryWriter(FileHandle_t file, bool close /* = false */) :
	m_File(file),
	m_ShouldClose(close),
	m_ShouldFlipEndianness(false)
{
}

BinaryWriter::~BinaryWriter()
{
	if (m_ShouldClose)
		filesystem->Close(m_File);
}

void BinaryWriter::WriteInt8(int8 data)
{
	WriteData(&data, 1);
}

void BinaryWriter::WriteUInt8(uint8 data)
{
	WriteData(&data, 1);
}

void BinaryWriter::WriteInt16(int16 data)
{
	uint16 value = FixEndianness16(static_cast<uint16>(data));
	WriteData(&value, 2);
}

void BinaryWriter::WriteUInt16(uint16 data)
{
	uint16 value = FixEndianness16(data);
	WriteData(&value, 2);
}

void BinaryWriter::WriteInt32(int32 data)
{
	uint32 value = FixEndianness32(static_cast<uint32>(data));
	WriteData(&value, 4);
}

void BinaryWriter::WriteUInt32(uint32 data)
{
	uint32 value = FixEndianness32(data);
	WriteData(&value, 4);
}

void BinaryWriter::WriteInt64(int64 data)
{
	uint64 value = FixEndianness64(static_cast<uint64>(data));
	WriteData(&value, 8);
}

void BinaryWriter::WriteUInt64(uint64 data)
{
	uint64 value = FixEndianness64(data);
	WriteData(&value, 8);
}

void BinaryWriter::WriteFloat(float data)
{
	uint32 value = FixEndianness32(static_cast<uint32>(data));
	WriteData(&value, 4);
}

void BinaryWriter::WriteDouble(double data)
{
	uint64 value = FixEndianness64(static_cast<uint64>(data));
	WriteData(&value, 8);
}

void BinaryWriter::WriteString(const char* data)
{
	uint32 length = Q_strlen(data);
	WriteUInt32(length);
	WriteData(data, length);
}

void BinaryWriter::WriteData(const void* data, int length)
{
	filesystem->Write(data, length, m_File);
}

uint16 BinaryWriter::FixEndianness16(uint16 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);
}

uint32 BinaryWriter::FixEndianness32(uint32 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 24) & 0x000000FF) | ((data >> 8) & 0x0000FF00) |
		((data << 8) & 0x00FF0000) | ((data << 24) & 0xFF000000);
}

uint64 BinaryWriter::FixEndianness64(uint64 data)
{
	if (!m_ShouldFlipEndianness)
		return data;

	return ((data >> 56) & 0x00000000000000FF) | ((data >> 40) & 0x000000000000FF00) |
		((data >> 24) & 0x0000000000FF0000) | ((data >> 8) & 0x00000000FF000000) | 
		((data << 8) & 0x000000FF00000000) | ((data << 24) & 0x0000FF0000000000) | 
		((data << 40) & 0x00FF000000000000) | ((data << 56) & 0xFF00000000000000);
}

