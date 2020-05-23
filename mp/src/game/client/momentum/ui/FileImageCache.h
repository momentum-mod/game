#pragma once

#include "utlbuffer.h"
#include "utldict.h"

struct ImageCacheEntry
{
    CUtlBuffer m_bufOriginalImage;
};

class CFileImageCache
{
public:
    CFileImageCache() {}

    void AddImageToCache(const char *pPath, const CUtlBuffer &pBuf)
    {
        const auto foundIndex = m_dictImages.Find(pPath);
        if (m_dictImages.IsValidIndex(foundIndex))
            return;

        const auto pEntry = new ImageCacheEntry;
        pEntry->m_bufOriginalImage.CopyBuffer(pBuf);

        const auto index = m_vecImages.AddToTail(pEntry);
        m_dictImages.Insert(pPath, index);
    }

    void RemoveImageFromCache(const char *pImagePath)
    {
        const auto index = m_dictImages.Find(pImagePath);
        if (!m_dictImages.IsValidIndex(index))
            return;

        m_vecImages.PurgeAndDeleteElement(m_dictImages[index]);
        m_dictImages.RemoveAt(index);
    }

    ImageCacheEntry *FindImageByPath(const char *pPath)
    {
        const auto index = m_dictImages.Find(pPath);
        if (m_dictImages.IsValidIndex(index))
        {
            return m_vecImages[m_dictImages[index]];
        }

        return nullptr;
    }

    CUtlVector<ImageCacheEntry *> m_vecImages;
    CUtlDict<int, uint16> m_dictImages;
};

extern CFileImageCache *g_pFileImageCache;