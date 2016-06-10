#pragma once

#include "nui_predef.h"

class CMomNUIUtils
{
#ifdef _WIN32
public:
    static size_t OffsetToCode(void* handle);
    static size_t GetSizeOfCode(void* handle);
    static void* SearchPattern(uint8* p_BaseAddress, size_t p_ScanSize, uint8* p_Pattern, size_t p_PatternSize, uint8 p_Wildcard = 0xDD);
#endif
};