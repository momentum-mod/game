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

    // property page handlers
    virtual void OnPageShow() OVERRIDE;

    MapListType_e GetMapListType() OVERRIDE { return MAP_LIST_LIBRARY; }

    //Filters based on the filter data
    void GetNewMapList() OVERRIDE;//called upon loading

    // Tell the game list what to put in there when there are no games found.
    virtual void SetEmptyListText();

    // Empty (for now?)
    void SetListCellColors(MapData* pData, KeyValues* pKvInto) OVERRIDE {}

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    /*void GetWorkshopItems();
    void OnWorkshopDownloadComplete(DownloadItemResult_t *pCallback, bool bIOFailure);
    CCallResult<CLocalMaps, DownloadItemResult_t> m_DownloadCompleteCallback;

    void AddWorkshopItemToLocalMaps(PublishedFileId_t id);*/
private:

    // true if we're broadcasting for servers
    bool m_bLoadedMaps;

};