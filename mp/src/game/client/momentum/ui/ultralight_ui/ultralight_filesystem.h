#ifndef ULTRALIGHT_FILESYSTEM_H
#define ULTRALIGHT_FILESYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include <Ultralight/Ultralight.h>
#include "platform.h"

// This only implements some of the functions in the interface, most of them aren't used by ultralight anyway.
class SourceFileSystem : public ultralight::FileSystem
{
  public:
    // Check if file path exists, return true if exists.
    virtual bool FileExists(const ultralight::String16 &path) OVERRIDE;

    // Delete file, return true on success.
    virtual bool DeleteFile_(const ultralight::String16 &path) OVERRIDE;

    // Delete empty directory, return true on success.
    virtual bool DeleteEmptyDirectory(const ultralight::String16 &path) OVERRIDE;

    // Move file, return true on success.
    virtual bool MoveFile_(const ultralight::String16 &old_path, const ultralight::String16 &new_path) OVERRIDE;

    // Get file size, store result in 'result'. Return true on success.
    virtual bool GetFileSize(const ultralight::String16 &path, int64_t &result) OVERRIDE;

    // Get file size of previously opened file, store result in 'result'. Return true on success.
    virtual bool GetFileSize(ultralight::FileHandle handle, int64_t &result) OVERRIDE;

    // Get file mime type (eg "text/html"), store result in 'result'. Return true on success.
    virtual bool GetFileMimeType(const ultralight::String16 &path, ultralight::String16 &result) OVERRIDE;

    // Get file last modification time, store result in 'result'. Return true on success.
    virtual bool GetFileModificationTime(const ultralight::String16 &path, time_t &result) OVERRIDE;

    // Get file creation time, store result in 'result'. Return true on success.
    virtual bool GetFileCreationTime(const ultralight::String16 &path, time_t &result) OVERRIDE;

    // Get path type (file or directory).
    virtual ultralight::MetadataType GetMetadataType(const ultralight::String16 &path) OVERRIDE;

    // Concatenate path with another path component. Return concatenated result.
    virtual ultralight::String16 GetPathByAppendingComponent(const ultralight::String16 &path,
                                                             const ultralight::String16 &component) OVERRIDE;

    // Create directory, return true on success.
    virtual bool CreateDirectory_(const ultralight::String16 &path) OVERRIDE;

    // Get home directory path.
    virtual ultralight::String16 GetHomeDirectory() OVERRIDE;

    // Get filename component from path.
    virtual ultralight::String16 GetFilenameFromPath(const ultralight::String16 &path) OVERRIDE;

    // Get directory name from path.
    virtual ultralight::String16 GetDirectoryNameFromPath(const ultralight::String16 &path) OVERRIDE;

    // Get volume from path and store free space in 'result'. Return true on success.
    virtual bool GetVolumeFreeSpace(const ultralight::String16 &path, uint64_t &result) OVERRIDE;

    // Get volume from path and return its unique volume id.
    virtual int32_t GetVolumeId(const ultralight::String16 &path) OVERRIDE;

    // Get file listing for directory path with optional filter, return vector of file paths.
    virtual ultralight::Ref<ultralight::String16Vector> ListDirectory(const ultralight::String16 &path,
                                                                      const ultralight::String16 &filter) OVERRIDE;

    // Open a temporary file with suggested prefix, store handle in 'handle'. Return path of temporary file.
    virtual ultralight::String16 OpenTemporaryFile(const ultralight::String16 &prefix,
                                                   ultralight::FileHandle &handle) OVERRIDE;

    // Open file path for reading or writing. Return file handle on success, or invalidultralight::FileHandle on failure.
    virtual ultralight::FileHandle OpenFile(const ultralight::String16 &path, bool open_for_writing) OVERRIDE;

    // Close previously-opened file.
    virtual void CloseFile(ultralight::FileHandle &handle) OVERRIDE;

    // Seek currently-opened file, with offset relative to certain origin. Return new file offset.
    virtual int64_t SeekFile(ultralight::FileHandle handle, int64_t offset, ultralight::FileSeekOrigin origin) OVERRIDE;

    // Truncate currently-opened file with offset, return true on success.
    virtual bool TruncateFile(ultralight::FileHandle handle, int64_t offset) OVERRIDE;

    // Write to currently-opened file, return number of bytes written or -1 on failure.
    virtual int64_t WriteToFile(ultralight::FileHandle handle, const char *data, int64_t length) OVERRIDE;

    // Read from currently-opened file, return number of bytes read or -1 on failure.
    virtual int64_t ReadFromFile(ultralight::FileHandle handle, char *data, int64_t length) OVERRIDE;

    // Copy file from source to destination, return true on success.
    virtual bool CopyFile_(const ultralight::String16 &source_path,
                           const ultralight::String16 &destination_path) OVERRIDE;
};

#endif