#pragma once

#include "platform.h"
#include <RmlUi/Core/FileInterface.h>

class RmlFileInterface : public Rml::Core::FileInterface
{
  public:
    // Opens a file.
    virtual Rml::Core::FileHandle Open(const Rml::Core::String &path) OVERRIDE;

    // Closes a previously opened file.
    virtual void Close(Rml::Core::FileHandle file) OVERRIDE;

    // Reads data from a previously opened file.
    virtual size_t Read(void *buffer, size_t size, Rml::Core::FileHandle file) OVERRIDE;

    // Seeks to a point in a previously opened file.
    virtual bool Seek(Rml::Core::FileHandle file, long offset, int origin) OVERRIDE;

    // Returns the current position of the file pointer.
    virtual size_t Tell(Rml::Core::FileHandle file) OVERRIDE;
};