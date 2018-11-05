#include "cbase.h"
#include "server_events.h"
#include "mom_shareddefs.h"
#include "tickset.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "util/mom_util.h"
#include "fmtstr.h"
#include "gason.h"
#include "util/jsontokv.h"

#include "tier0/memdbgon.h"

//This is only called when "map ____" is called, if the user uses changelevel then...
// \/(o_o)\/
void Momentum::GameInit()
{
    ConVarRef gm("mom_gamemode");
    ConVarRef map("host_map");
    const char *pMapName = map.GetString();
    // This will only happen if the user didn't use the map selector to start a map
    ConVarRef("sv_contact").SetValue("http://momentum-mod.org/contact");
    //set gamemode depending on map name
    //MOM_TODO: This needs to read map entity/momfile data and set accordingly

    if (!Q_strnicmp(pMapName, "surf_", strlen("surf_")))
    {
        DevLog("Setting game mode to surf (GM# %d)\n", MOMGM_SURF);
        gm.SetValue(MOMGM_SURF);
    }
    else if (!Q_strnicmp(pMapName, "bhop_", strlen("bhop_")))
    {
        DevLog("Setting game mode to bhop (GM# %d)\n", MOMGM_BHOP);
        gm.SetValue(MOMGM_BHOP);
    }
    else if (!Q_strnicmp(pMapName, "kz_", strlen("kz_")))
    {
        DevLog("Setting game mode to scroll (GM# %d)\n", MOMGM_SCROLL);
        gm.SetValue(MOMGM_SCROLL);
    }
    else if (!Q_strcmp(pMapName, "background") || !Q_strcmp(pMapName, "credits"))
    {
        gm.SetValue(MOMGM_ALLOWED);
    }
    else
    {
        DevLog("Setting game mode to unknown\n");
        gm.SetValue(MOMGM_UNKNOWN);
    }
}

CMOMServerEvents::CMOMServerEvents(const char* pName): CAutoGameSystemPerFrame(pName), zones(nullptr),
                                                       m_hAuthTicket(k_HAuthTicketInvalid), m_bufAuthBuffer(nullptr),
                                                       m_iAuthActualSize(0), m_pAPIKey(nullptr)
{
}

void CMOMServerEvents::PostInit()
{
    TickSet::TickInit();
    MountAdditionalContent();

    DoAuth();

    // MOM_TODO: connect to site
    /*if (SteamAPI_IsSteamRunning())
    {

    }*/
}

void CMOMServerEvents::Shutdown()
{

    if (m_hAuthTicket != k_HAuthTicketInvalid)
    {
        SteamUser()->CancelAuthTicket(m_hAuthTicket);
        delete[] m_bufAuthBuffer;
    }
}

void CMOMServerEvents::LevelInitPreEntity()
{
    const char *pMapName = gpGlobals->mapname.ToCStr();
    // (Re-)Load zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }
    zones = new CMapzoneData(pMapName);
    zones->SpawnMapZones();
}


void CMOMServerEvents::LevelInitPostEntity()
{
    //disable point_servercommand
    ConVarRef servercommand("sv_allow_point_servercommand");
    servercommand.SetValue(0);
}

void CMOMServerEvents::LevelShutdownPreEntity()
{
    // Unload zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }
}

void CMOMServerEvents::LevelShutdownPostEntity()
{
    ConVarRef fullbright("mat_fullbright");
    // Shut off fullbright if the map enabled it
    if (fullbright.IsValid() && fullbright.GetBool())
        fullbright.SetValue(0);
}
void CMOMServerEvents::FrameUpdatePreEntityThink()
{
    if (!g_pMomentumTimer->GotCaughtCheating())
    {
        ConVarRef cheatsRef("sv_cheats");
        if (cheatsRef.GetBool())
        {
            g_pMomentumTimer->SetCheating(true);
            g_pMomentumTimer->Stop(false);
        }
    }
}

void CMOMServerEvents::OnAuthHTTP(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    DevWarning(2, "%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
    {
        Warning("%s - bIOFailure is true!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
    {
        Warning("%s - k_EHTTPStatusCode404NotFound !\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode4xxUnknown)
    {
        Warning("%s - No friends found on this map. You must be a teapot!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode500InternalServerError)
    {
        Warning("%s - INTERNAL SERVER ERROR!\n", __FUNCTION__);
        return;
    }

    uint32 size;
    SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

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
            CKeyValuesDumpContextAsDevMsg dump;
            pResponse->Dump(&dump);
            KeyValues::AutoDelete ad(pResponse);

            uint32 tokenLength = pResponse->GetInt("length");

            if (tokenLength)
            {
                m_pAPIKey = new char[tokenLength + 1];
                Q_strncpy(m_pAPIKey, pResponse->GetString("token"), tokenLength + 1);
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }

    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void CMOMServerEvents::OnGameOverlay(GameOverlayActivated_t* pParam)
{
    engine->ServerCommand("unpause\n");
}

void CMOMServerEvents::OnAuthTicket(GetAuthSessionTicketResponse_t* pParam)
{
    Msg("Ticket callback!\n");
    if (pParam->m_eResult == k_EResultOK)
    {
        Msg("Ticket okay!\n");
        if (pParam->m_hAuthTicket == m_hAuthTicket)
        {
            Msg("It's our ticket, we should send it to the site now!\n");
            // Send it to the site

            bool bSuccess = false;
            if (SteamHTTP())
            {
                HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, "http://localhost:3002/api/auth/steam/user");

                uint64 id = SteamUser()->GetSteamID().ConvertToUint64();
                CFmtStr idStr("%llu", id);
                Msg("Sending ID %s\n", idStr.Get());
                SteamHTTP()->SetHTTPRequestHeaderValue(handle, "id", idStr.Get());

                if (SteamHTTP()->SetHTTPRequestRawPostBody(handle, "application/octet-stream", m_bufAuthBuffer, m_iAuthActualSize))
                    Msg("Body Set!\n");

                SteamAPICall_t apiHandle;

                if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
                {
                    m_cAuthCallresult.Set(apiHandle, this, &CMOMServerEvents::OnAuthHTTP);
                    bSuccess = true;
                }
                else
                {
                    Warning("Failed to send HTTP Request!\n");
                    SteamHTTP()->ReleaseHTTPRequest(handle); // GC
                }
            }
            else
            {
                Warning("Steampicontext failure.\nCould not find Steam Api Context active");
            }

            if (bSuccess)
                Msg("Sent out the request!\n");
        }
    }
}

void CMOMServerEvents::DoAuth()
{
    Msg("Getting the ticket...\n");
    if (m_pAPIKey)
        delete[] m_pAPIKey;
    m_pAPIKey = nullptr;
    if (m_bufAuthBuffer)
        delete[] m_bufAuthBuffer;
    m_bufAuthBuffer = nullptr;

    m_bufAuthBuffer = new byte[1024];
    m_hAuthTicket = SteamUser()->GetAuthSessionTicket(m_bufAuthBuffer, 1024, &m_iAuthActualSize);
    if (m_hAuthTicket == k_HAuthTicketInvalid)
        Warning("Initial call failed!\n");
}


void CMOMServerEvents::MountAdditionalContent()
{
    // From the Valve SDK wiki
    KeyValues *pMainFile = new KeyValues("gameinfo.txt");
    bool bLoad = false;
#ifndef _WINDOWS
    // case sensitivity
    bLoad = pMainFile->LoadFromFile(filesystem, "GameInfo.txt", "MOD");
#endif
    if (!bLoad)
        bLoad = pMainFile->LoadFromFile(filesystem, "gameinfo.txt", "MOD");

    if (pMainFile && bLoad)
    {
        KeyValues *pFileSystemInfo = pMainFile->FindKey("FileSystem");
        if (pFileSystemInfo)
        {
            for (KeyValues *pKey = pFileSystemInfo->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
            {
                if (Q_strcmp(pKey->GetName(), "AdditionalContentId") == 0)
                {
                    int appid = abs(pKey->GetInt());
                    if (appid)
                        if (filesystem->MountSteamContent(-appid) != FILESYSTEM_MOUNT_OK)
                            Warning("Unable to mount extra content with appId: %i\n", appid);
                }
            }
        }
    }
    pMainFile->deleteThis();
}

CMOMServerEvents g_MOMServerEvents("CMOMServerEvents");

CON_COMMAND(auth_test, "Auth test")
{
    g_MOMServerEvents.DoAuth();
}