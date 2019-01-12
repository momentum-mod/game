#ifndef ULTRALIGHT_FONTLOADER_WIN_H
#define ULTRALIGHT_FONTLOADER_WIN_H
#ifdef _WIN32
#pragma once
#endif

#include <Ultralight/platform/FontLoader.h>
#include "utlmap.h"
#include "platform.h"

/**
 * FontLoader implementation for Windows.
 */
class UltralightFontLoaderWin : public ultralight::FontLoader
{
public:
  UltralightFontLoaderWin() {}
  virtual ultralight::String16 fallback_font() const OVERRIDE;
  virtual ultralight::Ref<ultralight::Buffer> Load(const ultralight::String16 &family, int weight, bool italic,
                                                   float size) OVERRIDE;

protected:
  CUtlMap<uint64_t, ultralight::RefPtr<ultralight::Buffer>> m_mapFonts;
};

#endif