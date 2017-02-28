#include "pch_mapselection.h"
#include "util/jsontokv.h"
#include "view_scene.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//			NOTE:	m_Servers can not use more than 96 sockets, else it will
//					cause internet explorer to Stop working under win98 SE!
//-----------------------------------------------------------------------------
COnlineMaps::COnlineMaps(vgui::Panel *parent, const char *panelName) : CBaseMapsPage(parent, panelName)
{
    m_bRequireUpdate = true;
    m_bOfflineMode = !IsSteamGameServerBrowsingEnabled();
    m_iCurrentPage = 0;
    LoadFilterSettings();
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COnlineMaps::~COnlineMaps()
{
}


void COnlineMaps::MapsQueryCallback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (bIOFailure)
    {
        RefreshComplete(eNoServerReturn);
        return;
    }

    if (pCallback->m_eStatusCode != k_EHTTPStatusCode200OK)
    {
        RefreshComplete(eServerBadReturn);
        return;
    }

    EMapQueryOutputs returnCode = eSuccess;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        RefreshComplete(eNoServerReturn);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);
            KeyValues *pRuns = pResponse->FindKey("maps");

            if (pRuns && !pRuns->IsEmpty())
            {
                FOR_EACH_SUBKEY(pRuns, pRun)
                {
                    mapdisplay_t map;
                    mapstruct_t m;
                    Q_strcpy(m.m_szMapName, pRun->GetString("map_name"));
                    m.m_bHasStages = !pRun->GetBool("linear");
                    m.m_iZoneCount = pRun->GetInt("zones");
                    m.m_iDifficulty = pRun->GetInt("difficulty");
                    m.m_iGameMode = pRun->GetInt("gamemode");
                    m.m_iMapId = pRun->GetInt("id");
                    Q_strcpy(m.m_szBestTime, pRun->GetString("zones"));
                    Q_strcpy(m.m_szMapUrl, pRun->GetString("file_path"));
                    Q_strcpy(m.m_szZoneUrl, pRun->GetString("zone_file"));
                    Q_strcpy(m.m_szThumbnailUrl, pRun->GetString("thumbnail"));
                    map.m_mMap = m;
                    KeyValues *kv = new KeyValues("map");
                    if (Q_strlen(m.m_szThumbnailUrl) > 3)
                    {
                        // We ran into a good issue here because of the life time of imageDownloader. Thanks @Gocnak for noticing this after almost an hour

                        // We fix it by creating a pointer, and it gets deleted when we finish the callback. We're too good
                        CImageDownloader *imageDownloader = new CImageDownloader();
                        if (imageDownloader->Process(m.m_szMapName, m.m_szThumbnailUrl, m_pMapList, map.m_iMapImageIndex))
                            delete imageDownloader;
                    }
                    kv->SetString(KEYNAME_MAP_NAME, m.m_szMapName);
                    kv->SetString(KEYNAME_MAP_LAYOUT, m.m_bHasStages ? "STAGED" : "LINEAR");
                    kv->SetInt(KEYNAME_MAP_ZONE_COUNT, m.m_iZoneCount);
                    kv->SetInt(KEYNAME_MAP_DIFFICULTY, m.m_iDifficulty);
                    kv->SetString(KEYNAME_MAP_BEST_TIME, m.m_szBestTime);
                    kv->SetInt(KEYNAME_MAP_IMAGE, map.m_iMapImageIndex);
                    kv->SetString(KEYNAME_MAP_PATH, m.m_szMapUrl);
                    kv->SetString(KEYNAME_MAP_ZONE_PATH, m.m_szZoneUrl);
                    map.m_iListID = m_pMapList->AddItem(kv, 0, false, false);
                    m_vecMaps.AddToTail(map); 
                }
            }
            else
            {
                returnCode = eNoMapsReturned;
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
        returnCode = eServerBadReturn;
    }

    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    RefreshComplete(returnCode);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COnlineMaps::PerformLayout()
{
    if (!m_bOfflineMode && m_bRequireUpdate && MapSelectorDialog().IsVisible())
    {
        PostMessage(this, new KeyValues("GetNewServerList"), 0.1f);
        m_bRequireUpdate = false;
    }

    if (m_bOfflineMode)
    {
        m_pMapList->SetEmptyListText("#ServerBrowser_OfflineMode");
        m_pStartMap->SetEnabled(false);
        m_pQueryMaps->SetEnabled(false);
        m_pQueryMapsQuick->SetEnabled(false);
        m_pFilter->SetEnabled(false);
    }

    BaseClass::PerformLayout();
    //m_pLocationFilter->SetEnabled(true);
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh if needed
//-----------------------------------------------------------------------------
void COnlineMaps::OnPageShow()
{
    GetNewMapList();
}

void COnlineMaps::OnMapStart()
{
    if (!m_pMapList->GetSelectedItemsCount()) return;
    CMapDownloader* mapDownloader = new CMapDownloader;
    KeyValues *kv = m_pMapList->GetItem(m_pMapList->GetSelectedItem(0));
    if (mapDownloader->Process(kv->GetString(KEYNAME_MAP_NAME), kv->GetString(KEYNAME_MAP_PATH, nullptr), kv->GetString(KEYNAME_MAP_ZONE_PATH, nullptr), this))
        delete mapDownloader;
}


//-----------------------------------------------------------------------------
// Purpose: Called every frame, maintains sockets and runs refreshes
//-----------------------------------------------------------------------------
void COnlineMaps::OnTick()
{
    BaseClass::OnTick();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COnlineMaps::GetNewMapList()
{
    UpdateStatus();

    m_bRequireUpdate = false;

    m_pMapList->DeleteAllItems();
    m_vecMaps.RemoveAll();
    m_iCurrentPage = 0;
    StartRefresh();
}

void COnlineMaps::StartSelectedMap()
{
    BaseClass::OnMapStart();
}

void COnlineMaps::RefreshComplete(EMapQueryOutputs eResponse)
{
    SetRefreshing(false);
    UpdateFilterSettings();
    switch (eResponse)
    {
        case eNoMapsReturned: 
            m_pMapList->SetEmptyListText("#ServerBrowser_NoInternetGames"); 
            break;
        case eNoServerReturn: 
        case eServerBadReturn:
            m_pMapList->SetEmptyListText("#ServerBrowser_MasterServerNotResponsive");
            break;
        case eSuccess: 
        default:
            break;
    }

    UpdateStatus();
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool COnlineMaps::SupportsItem(IMapList::InterfaceItem_e item)
{
    switch (item)
    {
        case FILTERS:
        case GETNEWLIST:
            return true;

        default:
            return false;
    }
}


void COnlineMaps::StartRefresh()
{
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        SetRefreshing(true);
        ClearMapList();
        char szUrl[BUFSIZ];
        Q_snprintf(szUrl, BUFSIZ, "%s/getmaps/2/0/0", MOM_APIDOMAIN);
        //KeyValues *kvFilters = GetFilters();
        g_pMomentumUtil->CreateAndSendHTTPReq(szUrl, &cbMapsQuery, &COnlineMaps::MapsQueryCallback, this, k_EHTTPMethodGET);
        //kvFilters->deleteThis();
    }
}


//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void COnlineMaps::OnOpenContextMenu(int itemID)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pMapList);
    menu->ShowMenu(this, true, true);
}

bool CImageDownloader::Process(const char* szMapName, const char* szUrl, CMapListPanel* pTargetPanel, int &index)
{
    index = 0;
    if (!pTargetPanel || !pTargetPanel->GetImageList())
        return true;

    Q_strcpy(m_szImageUrl, szUrl);
    Q_strcpy(m_szMapName, szMapName);
    
    m_pImageList = pTargetPanel->GetImageList();
    if (!g_pMomentumUtil->MapThumbnailExists(szMapName))
    {
        // fetch the image
        m_iTargetIndex = m_pImageList->AddImage(nullptr);
        index = m_iTargetIndex;
        // If the request fails, then delete me, otherwise wait for the callback to.
        return !g_pMomentumUtil->CreateAndSendHTTPReq(szUrl, &cbDownloadCallback, &CImageDownloader::Callback, this);
    }

    // If it exists, lets search it on the panel list, in case it's already there
    char szPath[MAX_PATH];
    Q_snprintf(szPath, MAX_PATH, "maps/%s", szMapName);
    IImage *newImage = scheme()->GetImage(szPath, false);

    const int itemCount = m_pImageList->GetImageCount();
    for (int i = 0; i < itemCount; ++i)
    {
        IImage *iterImage = m_pImageList->GetImage(i);
        if (!iterImage) continue;
        // This can't use map id, use image comparising
        if (iterImage->GetID() == newImage->GetID())
        {
            // Image found, return its index (We save it on the target index too just in case we need it later)
            index = i;
            return true; // Delete me, we already exist
        }
    }

    // We're here because the image was not found, lets add it.
    // We first need to get the vtf into something that the map lists understands      
    index = m_pImageList->AddImage(newImage);
    return true; // Delete me, we already exist
}

void CImageDownloader::Callback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (!bIOFailure && pCallback && pCallback->m_bRequestSuccessful && pCallback->m_eStatusCode == k_EHTTPStatusCode200OK)
    {
        char szPreviewPathSmall[MAX_PATH];
        char szVTFPreviewPath[MAX_PATH];
        char szVMTPreviewPath[MAX_PATH];
        Q_snprintf(szPreviewPathSmall, MAX_PATH, "maps/%s", m_szMapName);

        Q_snprintf(szVTFPreviewPath, MAX_PATH, "materials/vgui/%s.vtf", szPreviewPathSmall);
        Q_snprintf(szVMTPreviewPath, MAX_PATH, "materials/vgui/%s.vmt", szPreviewPathSmall);
        FileHandle_t file = filesystem->Open(szVTFPreviewPath, "w+b", "MOD");
        uint32 size;
        steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
        if (size > 0)
        {
            DevLog("Size of body: %u\n", size);
            uint8* pData = new uint8[size];
            steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
            // write the file
            filesystem->Write(pData, size, file);
            // save the file

            filesystem->Close(file);
            DevLog("Successfully written file to %s\n", szVTFPreviewPath);

            // Create the VMT file for the texture
            KeyValues* kvFile = new KeyValues("UnlitGeneric");
            char szTexturePath[MAX_PATH];
            Q_snprintf(szTexturePath, MAX_PATH, "vgui/%s", szPreviewPathSmall);
            kvFile->SetString("$basetexture", szTexturePath);
            kvFile->SetInt("$translucent", 1);
            kvFile->SetInt("$ignorez", 1);
            if (kvFile->SaveToFile(filesystem, szVMTPreviewPath, "MOD"))
            {
                m_pImageList->SetImageAtIndex(m_iTargetIndex, scheme()->GetImage(szPreviewPathSmall, false));
                DevLog("Saved VMT to %s\n", szVMTPreviewPath);
            }

            kvFile->deleteThis();

            // Free resources
            if (pData)
            {
                delete[] pData;
            }
            pData = nullptr;
        }
    }

    if (pCallback)
        steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    delete this;
}

bool CMapDownloader::Process(const char* szMapName, const char* szMapUrl, const char* szZoneUrl, COnlineMaps *pTarget)
{
    if (!pTarget || szMapName == nullptr)
        return true;
    m_pMapTab = pTarget;
    Q_strcpy(m_szMapName, szMapName);
    
    if (szMapUrl == nullptr || szZoneUrl == nullptr)
    {
        // MOM_TODO: Requery web for this data
        return true; // Change this once we requery needed data
    }
    if (!g_pMomentumUtil->MapExists(szMapName))
    {
        // We have to delet this here only if both Reqs failed:
        bool mapReq = !g_pMomentumUtil->CreateAndSendHTTPReq(szMapUrl, &cbMapDownloadCallback, &CMapDownloader::MapCallback, this);
        bool zonReq = !g_pMomentumUtil->CreateAndSendHTTPReq(szZoneUrl, &cbZoneDownloadCallback, &CMapDownloader::ZoneCallback, this);
        return mapReq && zonReq;
    }
    return true;
}

void CMapDownloader::ZoneCallback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (!bIOFailure && pCallback && pCallback->m_bRequestSuccessful && pCallback->m_eStatusCode == k_EHTTPStatusCode200OK)
    {
        char szMapPath[MAX_PATH];
        Q_snprintf(szMapPath, MAX_PATH, "maps/%s.zon", m_szMapName);
        FileHandle_t file = filesystem->Open(szMapPath, "w+b", "MOD");
        uint32 size;
        steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
        if (size > 0)
        {
            DevLog("Size of body: %u\n", size);
            uint8* pData = new uint8[size];
            steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
            // write the file
            filesystem->Write(pData, size, file);
            // save the file
            filesystem->Close(file);
            DevLog("Successfully written file to %s\n", szMapPath);
        }
    }
    if (pCallback)
        steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    bool bOtherActive = cbMapDownloadCallback.IsActive();
    DevLog("%s thinks that other is %s\n", __FUNCTION__, bOtherActive ? "active" : "closed");
    if (!bOtherActive)
    {
        m_pMapTab->StartSelectedMap();
        delete this;
    }
}

void CMapDownloader::MapCallback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (!bIOFailure && pCallback && pCallback->m_bRequestSuccessful && pCallback->m_eStatusCode == k_EHTTPStatusCode200OK)
    {
        char szMapPath[MAX_PATH];
        Q_snprintf(szMapPath, MAX_PATH, "maps/%s.bsp", m_szMapName);
        FileHandle_t file = filesystem->Open(szMapPath, "w+b", "MOD");
        uint32 size;
        steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
        if (size > 0)
        {
            DevLog("Size of body: %u\n", size);
            uint8* pData = new uint8[size];
            steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
            // write the file
            filesystem->Write(pData, size, file);
            // save the file
            filesystem->Close(file);
            DevLog("Successfully written file to %s\n", szMapPath);
        }
    }
    if (pCallback)
        steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    bool bOtherActive = cbZoneDownloadCallback.IsActive();
    DevLog("%s thinks that other is %s\n", __FUNCTION__, bOtherActive ? "active" : "closed");
    if (!bOtherActive)
    {
        m_pMapTab->StartSelectedMap();
        delete this;
    }
}
