#ifndef INTERNETGAMES_H
#define INTERNETGAMES_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Internet games list
//-----------------------------------------------------------------------------
class COnlineMaps : public CBaseMapsPage
{

    DECLARE_CLASS_SIMPLE(COnlineMaps, CBaseMapsPage);

public:
    COnlineMaps(vgui::Panel *parent, const char *panelName = "OnlineMaps");
    ~COnlineMaps();

    // property page handlers
    virtual void OnPageShow();

    // returns true if the game list supports the specified ui elements
    virtual bool SupportsItem(IMapList::InterfaceItem_e item);

    // Starts the map query
    void StartRefresh() OVERRIDE;

    // gets a new server list
    MESSAGE_FUNC(GetNewMapList, "GetNewMapList");


    enum EMapQueryOutputs
    {
        eNoMapsReturned,
        eNoServerReturn,
        eServerBadReturn,
        eSuccess
    };

    void RefreshComplete(EMapQueryOutputs response);

    //virtual void LoadFilterSettings() {};//MOM_TODO: make this filter online maps (by name/gametype/difficulty?)

    CCallResult<COnlineMaps, HTTPRequestCompleted_t> cbMapsQuery;
    void MapsQueryCallback(HTTPRequestCompleted_t *, bool);

protected:
    // vgui overrides
    virtual void PerformLayout();
    virtual void OnTick();
private:

    // Called once per frame to check re-send request to master server
    //void CheckRetryRequest(ESteamServerType serverType);
    // opens context menu (user right clicked on a server)
    MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

    bool m_bRequireUpdate;	// checks whether we need an update upon opening

    bool m_bOfflineMode;

    int m_iCurrentPage;
};

class CImageDownloader
{
    DECLARE_CLASS_NOBASE(CImageDownloader)

public:
    CImageDownloader()
    {
        m_iTargetIndex = -1;
        m_pImageList = nullptr;
    }

    int Process(const int iMapId, const char *szMapName, const char *szUrl, CMapListPanel* pTargetPanel);
private:

    CCallResult<CImageDownloader, HTTPRequestCompleted_t> cbDownloadCallback;
    void Callback(HTTPRequestCompleted_t* pCallback, bool bIOFailure);

    char m_szMapName[MAX_PATH];
    char m_szImageUrl[MAX_PATH];
    int m_iTargetIndex;
    ImageList *m_pImageList;
};

#endif // INTERNETGAMES_H