#include "cbase.h"
#include "ultralight_font_loader.h"

#include <Ultralight/Ultralight.h>
#include <filesystem.h>

ultralight::String16 UltralightFontLoader::fallback_font() const { return "BigNoodleTitling"; }

ultralight::Ref<ultralight::Buffer> UltralightFontLoader::Load(const ultralight::String16 &family, int weight,
                                                               bool italic, float size)
{
    char filename[MAX_PATH];
    Q_snprintf(filename, sizeof(filename), "resource/font/%s.ttf", ultralight::String(family).utf8().data());


    CUtlBuffer buffer;
    if (!g_pFullFileSystem->ReadFile(filename, "MOD", buffer))
    {
        return ultralight::DefaultFontLoader()->Load("Roboto", weight, italic, size);
    }

    return ultralight::Buffer::Create(buffer.Base(), buffer.Size());
}
