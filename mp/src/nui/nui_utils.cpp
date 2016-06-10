#include "nui_utils.h"

#ifdef _WIN32
#include "winlite.h"

size_t CMomNUIUtils::GetSizeOfCode(void* handle)
{
    HMODULE hModule = (HMODULE)handle;

    if (!hModule)
        return NULL;

    PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER(hModule);

    if (!pDosHeader)
        return NULL;

    PIMAGE_NT_HEADERS pNTHeader = PIMAGE_NT_HEADERS((uint64) hModule + pDosHeader->e_lfanew);

    if (!pNTHeader)
        return NULL;

    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNTHeader->OptionalHeader;

    if (!pOptionalHeader)
        return NULL;

    return pOptionalHeader->SizeOfCode;
}

size_t CMomNUIUtils::OffsetToCode(void* handle)
{
    HMODULE hModule = (HMODULE)handle;

    if (!hModule)
        return NULL;

    PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER(hModule);

    if (!pDosHeader)
        return NULL;

    PIMAGE_NT_HEADERS pNTHeader = PIMAGE_NT_HEADERS(hModule + pDosHeader->e_lfanew);

    if (!pNTHeader)
        return NULL;

    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNTHeader->OptionalHeader;

    if (!pOptionalHeader)
        return NULL;

    return pOptionalHeader->BaseOfCode;
}

void* CMomNUIUtils::SearchPattern(uint8* baseAddress, size_t scanSize, uint8* pattern, size_t patternSize, uint8 p_Wildcard /* = 0xDD */)
{
    size_t badCharSkip[UCHAR_MAX + 1];
    uint8* scanEnd = baseAddress + scanSize - patternSize;

    size_t index = 0;
    size_t last = patternSize - 1;
    for (index = last; index > 0 && pattern[index] != p_Wildcard; --index);

    size_t diff = patternSize - index;

    for (index = 0; index <= UCHAR_MAX; ++index)
        badCharSkip[index] = diff;

    for (index = 0; index < last; ++index)
        badCharSkip[pattern[index]] = last - index;

    for (; baseAddress < scanEnd; baseAddress += badCharSkip[baseAddress[last]])
    {
        for (index = last; index > 0; --index)
            if (pattern[index] != p_Wildcard && baseAddress[index] != pattern[index])
                goto skip;

        return baseAddress;
    skip:;
    }

    return nullptr;
}
#endif