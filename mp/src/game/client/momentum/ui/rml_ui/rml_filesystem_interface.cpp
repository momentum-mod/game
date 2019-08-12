#include "cbase.h"
#include "rml_filesystem_interface.h"

#include "filesystem.h"

Rml::Core::FileHandle RmlFileInterface::Open(const Rml::Core::String& path)
{
    const char *filename = path.CString();
    return (Rml::Core::FileHandle)g_pFullFileSystem->Open(filename, "rb");
}

void RmlFileInterface::Close(Rml::Core::FileHandle file)
{
    g_pFullFileSystem->Close((FileHandle_t)file);
}

size_t RmlFileInterface::Read(void *buffer, size_t size, Rml::Core::FileHandle file)
{
    return (size_t)g_pFullFileSystem->Read(buffer, (int)size, (FileHandle_t)file);
}

bool RmlFileInterface::Seek(Rml::Core::FileHandle file, long offset, int origin)
{
    g_pFullFileSystem->Seek((FileHandle_t)file, offset, (FileSystemSeek_t)origin);
    return true;
}

size_t RmlFileInterface::Tell(Rml::Core::FileHandle file)
{
    return (size_t)g_pFullFileSystem->Tell((FileHandle_t)file);
}