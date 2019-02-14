#ifndef FILEWATCHDOG_H
#define FILEWATCHDOG_H
#ifdef _WIN32
#pragma once
#endif

#include <cstdint>
#include <platform.h>
#include <utldelegate.h>
#include <igamesystem.h>

#define INVALID_WATCH_HANDLE ((uint32_t)-1)

typedef CUtlDelegate<void(const char *filename)> FileChangedCallback_t;
typedef uint32_t FileWatchHandle_t;

class FileWatchdog : CAutoGameSystem
{
  public:
    virtual bool Init();
    virtual void Shutdown();

    FileWatchHandle_t AddFileChangeListener(const char *filename, FileChangedCallback_t callback);
    bool RemoveFileChangeListener(FileWatchHandle_t handle);
};

extern FileWatchdog *g_pFileWatchdog;
#endif
