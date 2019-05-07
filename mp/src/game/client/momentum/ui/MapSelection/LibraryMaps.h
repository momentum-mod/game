#pragma once

#include "BaseMapsPage.h"

//-----------------------------------------------------------------------------
// Purpose: Local maps list
//-----------------------------------------------------------------------------
class CLibraryMaps : public CBaseMapsPage
{
    DECLARE_CLASS_SIMPLE(CLibraryMaps, CBaseMapsPage);

public: 

    CLibraryMaps(Panel *parent);
    ~CLibraryMaps();

    MapListType_e GetMapListType() OVERRIDE { return MAP_LIST_LIBRARY; }

    //Filters based on the filter data
    void OnGetNewMapList() OVERRIDE;

    // Empty (for now?)
    void SetListCellColors(MapData* pData, KeyValues* pKvInto) OVERRIDE {}

    void OnMapCacheUpdated(KeyValues *pKv) OVERRIDE;
    void OnMapListDataUpdate(int id) OVERRIDE;

    void OnTabSelected() OVERRIDE;

    /*void GetWorkshopItems();
    void OnWorkshopDownloadComplete(DownloadItemResult_t *pCallback, bool bIOFailure);
    CCallResult<CLocalMaps, DownloadItemResult_t> m_DownloadCompleteCallback;

    void AddWorkshopItemToLocalMaps(PublishedFileId_t id);*/
private:
    ConVarRef m_cvarAutoDownload, m_cvarDeleteQueue;
    bool m_bLoadedMaps;

};