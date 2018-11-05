#include "cbase.h"
#include "server_events.h"
#include "mom_shareddefs.h"
#include "tickset.h"
#include "mapzones.h"
#include "mom_timer.h"

#include "tier0/memdbgon.h"
#include "util/mom_util.h"
#include "fmtstr.h"

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
                                                       m_hAuthTicket(k_HAuthTicketInvalid),
                                                       m_iAuthActualSize(0)
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

void CMOMServerEvents::OnAuthHTTP(HTTPRequestCompleted_t* pParam, bool bIOFailure)
{
    Msg("HTTP callback recv\n");
    if (bIOFailure)
        Warning("IO failure!\n");
    Msg("Status: %i\n", pParam->m_eStatusCode);
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
    if (m_bufAuthBuffer)
        delete[] m_bufAuthBuffer;

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