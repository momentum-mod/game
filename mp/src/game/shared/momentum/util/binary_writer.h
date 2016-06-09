#pragma once

#include "cbase.h"
#include "filesystem.h"

class CBinaryWriter
{
public:
    CBinaryWriter(FileHandle_t file, bool close = false);
    ~CBinaryWriter();

public:
    inline void ShouldFlipEndianness(bool flip) { m_bShouldFlipEndianness = flip; }
    inline void Seek(int position, FileSystemSeek_t seek) { filesystem->Seek(m_pFile, position, seek); }

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
    FileHandle_t m_pFile;
    bool m_bShouldClose;
    bool m_bShouldFlipEndianness;
};