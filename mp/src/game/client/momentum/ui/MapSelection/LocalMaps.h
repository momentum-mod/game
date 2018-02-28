#ifndef LOCALMAPS_H
#define LOCALMAPS_H
#ifdef _WIN32
#pragma once
#endif

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
    virtual void OnPageShow() override;

    // IGameList handlers
    // returns true if the game list supports the specified ui elements
    virtual bool SupportsItem(InterfaceItem_e item) override;

    // Control which button are visible.
    void ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter);

    //Filters based on the filter data
    void StartRefresh() override;
    void GetNewMapList() override;//called upon loading
    void AddNewMapToVector(const char* mapname);

    virtual void OnMapStart() override { BaseClass::OnMapStart(); }

    // Tell the game list what to put in there when there are no games found.
    virtual void SetEmptyListText();

    void GetWorkshopItems();
    void OnWorkshopDownloadComplete(DownloadItemResult_t *pCallback, bool bIOFailure);
    CCallResult<CLocalMaps, DownloadItemResult_t> m_DownloadCompleteCallback;

    void AddWorkshopItemToLocalMaps(PublishedFileId_t id);
private:
    // context menu message handlers
    MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

    // true if we're broadcasting for servers
    bool m_bLoadedMaps;

    //Fills a mapstruct with data read from local files
    static void FillMapstruct(mapstruct_t *);
};

#endif // LOCALMAPS_H