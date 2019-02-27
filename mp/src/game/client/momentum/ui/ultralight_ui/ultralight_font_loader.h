#pragma once

#include <Ultralight/platform/FontLoader.h>

class UltralightFontLoader : public ultralight::FontLoader
{
    virtual ultralight::String16 fallback_font() const;

    virtual ultralight::Ref<ultralight::Buffer> Load(const ultralight::String16 &family, int weight, bool italic,
                                                     float size);
};