#include "pch_mapselection.h"
#include "util/jsontokv.h"
#include "view_scene.h"

#include "vgui_bitmapimage.h"

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
                    Q_strcpy(m.m_szThumbnailUrl, pRun->GetString("thumbnail"));
                    map.m_mMap = m;
                    KeyValues *kv = new KeyValues("map");
                    if (Q_strlen(m.m_szThumbnailUrl) > 3)
                    {
                        CImageDownloader imageDownloader;
                        map.m_iMapImageIndex = imageDownloader.Process(m.m_iMapId, m.m_szMapName, m.m_szThumbnailUrl, m_pMapList);
                    }
                    kv->SetString(KEYNAME_MAP_NAME, m.m_szMapName);
                    kv->SetString(KEYNAME_MAP_LAYOUT, m.m_bHasStages ? "STAGED" : "LINEAR");
                    kv->SetInt(KEYNAME_MAP_ZONE_COUNT, m.m_iZoneCount);
                    kv->SetInt(KEYNAME_MAP_DIFFICULTY, m.m_iDifficulty);
                    kv->SetString(KEYNAME_MAP_BEST_TIME, m.m_szBestTime);
                    kv->SetInt(KEYNAME_MAP_IMAGE, map.m_iMapImageIndex);
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

int CImageDownloader::Process(const int iMapId, const char* szMapName, const char* szUrl, CMapListPanel* pTargetPanel)
{
    if (!pTargetPanel || !pTargetPanel->GetImageList())
    {
        return 0;
    }
    Q_strcpy(m_szImageUrl, szUrl);
    Q_strcpy(m_szMapName, szMapName);
    
    m_pImageList = pTargetPanel->GetImageList();
    if (!g_pMomentumUtil->MapThumbnailExists(szMapName))
    {
        // fetch the image
        m_iTargetIndex = m_pImageList->AddImage(nullptr);
        g_pMomentumUtil->CreateAndSendHTTPReq(szUrl, &cbDownloadCallback, &CImageDownloader::Callback, this);
    }
    else
    {
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
            if (iterImage == newImage)
            {
                // Image found, return its index (We save it on the target index too just in case we need it later)
                m_iTargetIndex = i;
                return m_iTargetIndex;
            }
        }
        // We're here because the image was not found, lets add it.
        // We first need to get the vtf into something that the map lists understands
        
        m_iTargetIndex = m_pImageList->AddImage(newImage);
    }
    return m_iTargetIndex;
}

void CImageDownloader::Callback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (bIOFailure || !pCallback || !pCallback->m_bRequestSuccessful || pCallback->m_eStatusCode != k_EHTTPStatusCode200OK)
        return;

    FileHandle_t file;
    char szPreviewPathSmall[MAX_PATH];
    char szVTFPreviewPath[MAX_PATH];
    char szVMTPreviewPath[MAX_PATH];
    Q_snprintf(szPreviewPathSmall, MAX_PATH, "maps/%s", m_szMapName);
        
    Q_snprintf(szVTFPreviewPath, MAX_PATH, "materials/vgui/%s.vtf", szPreviewPathSmall);
    Q_snprintf(szVMTPreviewPath, MAX_PATH, "materials/vgui/%s.vmt", szPreviewPathSmall);
    
    file = filesystem->Open(szVTFPreviewPath, "w+b", "MOD");
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("CImageDownloader::Callback: 0 body size!\n");
        return;
    }
    DevLog("Size of body: %u\n", size);
    uint8* pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    // write the file
    filesystem->Write(pData, size, file);
    // save the file
    
    filesystem->Close(file);
    DevLog("Successfully written file to %s\n", szVTFPreviewPath);

    // Create the VMT file for the texture
    KeyValues *kvFile = new KeyValues("UnlitGeneric");
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
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}
