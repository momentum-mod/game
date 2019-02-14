#include "FileWatchdog.h"

#include <filesystem.h>
#include <utlmap.h>
#include <utlvector.h>
#include <winlite.h>
#undef GetCurrentDirectory

#include <threadtools.h>

static FileWatchdog g_FileWatchdog;
FileWatchdog *g_pFileWatchdog = &g_FileWatchdog;

struct FileWatchCallbacks
{
	FileWatchCallbacks() {}
    FileWatchCallbacks(const FileWatchCallbacks &src)
    {
        Q_strcpy(filename, src.filename);
        listeners.CopyArray(src.listeners.Base(), src.listeners.Size());
        last_check_time = src.last_check_time;
    }

    struct FileWatchListener
    {
        FileWatchHandle_t listen_handle;
        FileChangedCallback_t callback;
    };
    char filename[MAX_PATH];
    CUtlVector<FileWatchListener> listeners;
    long last_check_time;
};

static uint32_t listener_count;
static CUtlVector<FileWatchCallbacks> watched_files;
static HANDLE filechange_handle;

static CThreadMutex filewatch_lock;

class WatcherThread : public CThread
{
  public:
    virtual int Run() OVERRIDE
    {
        while (true)
        {
            filewatch_lock.Lock();
            if (filechange_handle == INVALID_HANDLE_VALUE)
            {
                filewatch_lock.Unlock();
                break;
            }
            filewatch_lock.Unlock();

            unsigned long status = VCRHook_WaitForSingleObject(filechange_handle, INFINITE);

            switch (status)
            {
            case WAIT_OBJECT_0:
            {
                CAutoLock lock(filewatch_lock);
                for (int i = 0; i < watched_files.Size(); i++)
                {
                    FileWatchCallbacks &fwc = watched_files[i];

                    // file was written to since we last checked
                    const long last_write_time = g_pFullFileSystem->GetFileTime(fwc.filename);
                    if (last_write_time > fwc.last_check_time)
                    {
                        for (int i = 0; i < fwc.listeners.Size(); i++)
                        {
                            fwc.listeners[i].callback(fwc.filename);
                        }

                        VCRHook_Time(&fwc.last_check_time);

                        break;
                    }
                }

                if (filechange_handle == INVALID_HANDLE_VALUE)
                {
                    return 0;
                }

                BOOL res = FindNextChangeNotification(filechange_handle);
                AssertMsg(res, "Failed to re-watch directory");

                break;
            }
            case WAIT_TIMEOUT:
            {
                Warning("File watchdog timed out");

                break;
            }
            default:
            {
                AssertMsg(false, "Unhandled wait status (%s)", status);

                break;
            }
            }

            Yield();
        }
        return 0;
    }
};

static WatcherThread watcher_thread;

bool FileWatchdog::Init()
{
    char currdir[MAX_PATH];
    g_pFullFileSystem->RelativePathToFullPath(".", "MOD", currdir, sizeof(currdir));

    filechange_handle = FindFirstChangeNotification(currdir, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);

    if (filechange_handle == INVALID_HANDLE_VALUE)
    {
        AssertMsg(false, "Failed to create change handle");
        return false;
    }

    watcher_thread.Start();

    return true;
}

void FileWatchdog::Shutdown()
{
    filewatch_lock.Lock();
    FindCloseChangeNotification(filechange_handle);
    filechange_handle = INVALID_HANDLE_VALUE;
    filewatch_lock.Unlock();

    watcher_thread.Join();
}

FileWatchHandle_t FileWatchdog::AddFileChangeListener(const char *filename, FileChangedCallback_t callback)
{
    if (!g_pFullFileSystem->FileExists(filename))
    {
        AssertMsg(false, "File '%s' does not exist.", filename);
        return INVALID_WATCH_HANDLE;
    }

    CAutoLock lock(filewatch_lock);

    int existing = -1;
    for (int i = 0; i < watched_files.Size(); i++)
    {
        const FileWatchCallbacks &fwc = watched_files[i];
        if (Q_strcmp(fwc.filename, filename) == 0)
        {
            existing = i;
            break;
        }
    }

    FileWatchCallbacks::FileWatchListener listener;
    listener.listen_handle = listener_count++;
    listener.callback = callback;

    // new file to watch, add it
    if (existing == -1)
    {
        FileWatchCallbacks fwc;
        Q_strncpy(fwc.filename, filename, sizeof(fwc.filename));
        VCRHook_Time(&fwc.last_check_time);
        fwc.listeners.AddToTail(listener);
        watched_files.AddToTail(fwc);
    }
    // file is already being watched, just add the callback
    else
    {
        watched_files[existing].listeners.AddToTail(listener);
    }

    return listener.listen_handle;
}

bool FileWatchdog::RemoveFileChangeListener(FileWatchHandle_t handle)
{
    CAutoLock lock(filewatch_lock);

    if (handle == INVALID_WATCH_HANDLE)
    {
        return false;
    }

    for (int i = 0; i < watched_files.Size(); i++)
    {
        CUtlVector<FileWatchCallbacks::FileWatchListener> &listeners = watched_files[i].listeners;
        for (int j = 0; j < listeners.Size(); j++)
        {
            if (listeners[j].listen_handle == handle)
            {
                listeners.FastRemove(j);
                return true;
            }
        }
    }

    return false;
}
