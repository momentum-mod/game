#pragma once

#include "BaseMapsPage.h"

//-----------------------------------------------------------------------------
// Purpose: Local maps list
//-----------------------------------------------------------------------------
class CLocalMaps : public CBaseMapsPage
{
    DECLARE_CLASS_SIMPLE(CLocalMaps, CBaseMapsPage);

public: 

    CLocalMaps(vgui::Panel *parent);
    ~CLocalMaps();

    // property page handlers
    virtual void OnPageShow() OVERRIDE;

    // IGameList handlers
    // returns true if the game list supports the specified ui elements
    virtual bool SupportsItem(InterfaceItem_e item) OVERRIDE;

    //Filters based on the filter data
    void StartRefresh() OVERRIDE;
    void GetNewMapList() OVERRIDE;//called upon loading
    void AddNewMapToVector(const char* mapname);

    virtual void OnMapStart() OVERRIDE { BaseClass::OnMapStart(); }

    // Tell the game list what to put in there when there are no games found.
    virtual void SetEmptyListText();

    /*void GetWorkshopItems();
    void OnWorkshopDownloadComplete(DownloadItemResult_t *pCallback, bool bIOFailure);
    CCallResult<CLocalMaps, DownloadItemResult_t> m_DownloadCompleteCallback;

    void AddWorkshopItemToLocalMaps(PublishedFileId_t id);*/
private:
    // context menu message handlers
    MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

    // true if we're broadcasting for servers
    bool m_bLoadedMaps;

};