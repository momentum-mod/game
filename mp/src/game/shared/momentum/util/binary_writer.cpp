#include "cbase.h"
#include "binary_writer.h"

CBinaryWriter::CBinaryWriter(FileHandle_t file, bool close /* = false */) :
    m_pFile(file),
    m_bShouldClose(close),
    m_bShouldFlipEndianness(false)
{
}

CBinaryWriter::~CBinaryWriter()
{
    if (m_bShouldClose)
        filesystem->Close(m_pFile);
}

void CBinaryWriter::WriteBool(bool data)
{
    WriteUInt8(data ? 1 : 0);
}

void CBinaryWriter::WriteInt8(int8 data)
{
    WriteData(&data, 1);
}

void CBinaryWriter::WriteUInt8(uint8 data)
{
    WriteData(&data, 1);
}

void CBinaryWriter::WriteInt16(int16 data)
{
    uint16 value = FixEndianness16(*(uint16*) &data);
    WriteData(&value, 2);
}

void CBinaryWriter::WriteUInt16(uint16 data)
{
    uint16 value = FixEndianness16(data);
    WriteData(&value, 2);
}

void CBinaryWriter::WriteInt32(int32 data)
{
    uint32 value = FixEndianness32(*(uint32*) &data);
    WriteData(&value, 4);
}

void CBinaryWriter::WriteUInt32(uint32 data)
{
    uint32 value = FixEndianness32(data);
    WriteData(&value, 4);
}

void CBinaryWriter::WriteInt64(int64 data)
{
    uint64 value = FixEndianness64(*(uint64*) &data);
    WriteData(&value, 8);
}

void CBinaryWriter::WriteUInt64(uint64 data)
{
    uint64 value = FixEndianness64(data);
    WriteData(&value, 8);
}

void CBinaryWriter::WriteFloat(float data)
{
    uint32 value = FixEndianness32(*(uint32*) &data);
    WriteData(&value, 4);
}

void CBinaryWriter::WriteDouble(double data)
{
    uint64 value = FixEndianness64(*(uint64*) &data);
    WriteData(&value, 8);
}

void CBinaryWriter::WriteString(const char* data)
{
    uint16 length = Q_strlen(data);
    WriteUInt16(length);
    WriteData(data, length);
}

void CBinaryWriter::WriteData(const void* data, int length)
{
    filesystem->Write(data, length, m_pFile);
}

uint16 CBinaryWriter::FixEndianness16(uint16 data)
{
    if (!m_bShouldFlipEndianness)
        return data;

    return ((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);
}

uint32 CBinaryWriter::FixEndianness32(uint32 data)
{
    if (!m_bShouldFlipEndianness)
        return data;

    return ((data >> 24) & 0x000000FF) | ((data >> 8) & 0x0000FF00) |
        ((data << 8) & 0x00FF0000) | ((data << 24) & 0xFF000000);
}

uint64 CBinaryWriter::FixEndianness64(uint64 data)
{
    if (!m_bShouldFlipEndianness)
        return data;

    return ((data >> 56) & 0x00000000000000FF) | ((data >> 40) & 0x000000000000FF00) |
        ((data >> 24) & 0x0000000000FF0000) | ((data >> 8) & 0x00000000FF000000) | 
        ((data << 8) & 0x000000FF00000000) | ((data << 24) & 0x0000FF0000000000) | 
        ((data << 40) & 0x00FF000000000000) | ((data << 56) & 0xFF00000000000000);
}

