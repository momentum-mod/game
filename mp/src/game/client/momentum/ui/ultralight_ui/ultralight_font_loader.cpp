#include "cbase.h"
#include "ultralight_font_loader.h"

#include <Ultralight/Ultralight.h>
#include <filesystem.h>

ultralight::String16 UltralightFontLoader::fallback_font() const { return "BigNoodleTitling"; }

ultralight::Ref<ultralight::Buffer> UltralightFontLoader::Load(const ultralight::String16 &family, int weight,
                                                               bool italic, float size)
{
    char filename[MAX_PATH];
    Q_snprintf(filename, sizeof(filename), "resource/font/%s", ultralight::String(family).utf8().data());

    bool bold = (weight >= 700);
    char stylizedFilename[MAX_PATH];

    if (bold && italic)
    {
        Q_snprintf(stylizedFilename, sizeof(stylizedFilename), "%s-BoldItalic.ttf", filename);
    }
    else if (bold)
    {
        Q_snprintf(stylizedFilename, sizeof(stylizedFilename), "%s-Bold.ttf", filename);
    }
    else if (italic)
    {
        Q_snprintf(stylizedFilename, sizeof(stylizedFilename), "%s-Italic.ttf", filename);
    }
    else
    {
        Q_snprintf(stylizedFilename, sizeof(stylizedFilename), "%s-Regular.ttf", filename);
    }

    CUtlBuffer buffer;
    if (!g_pFullFileSystem->ReadFile(stylizedFilename, "MOD", buffer))
    {
        // Fall back to non-stylized name and try that
        Q_snprintf(stylizedFilename, sizeof(stylizedFilename), "%s.ttf", filename);
        if (!g_pFullFileSystem->ReadFile(stylizedFilename, "MOD", buffer))
        {
            // Seems like the font just doesn't exist, fall back to Roboto
            return ultralight::DefaultFontLoader()->Load("Roboto", weight, italic, size);
        }
    }

    return ultralight::Buffer::Create(buffer.Base(), buffer.Size());
}
