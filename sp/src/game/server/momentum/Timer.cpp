#include "cbase.h"
#include "Timer.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

void CTimer::Start(int start)
{
    m_iStartTick = start;
    SetRunning(true);
    DispatchStateMessage();
}

void CTimer::PostTime()
{
    if (steamapicontext->SteamHTTP() && steamapicontext->SteamUser() && !m_bWereCheatsActivated)
    {

        //Get required info 
        //MOM_TODO include the extra security measures for beta+
        uint64 steamID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
        const char* map = gpGlobals->mapname.ToCStr();
        int ticks = gpGlobals->tickcount - m_iStartTick;

        TickSet::Tickrate tickRate = TickSet::GetCurrentTickrate();

        //Build URL
        char webURL[512];
        Q_snprintf(webURL, 512, "http://momentum-mod.org/postscore/%llu/%s/%i/%s", steamID, map,
            ticks, tickRate.sType);

        DevLog("Ticks sent to server: %i\n", ticks);

        //Build request
        mom_UTIL.PostTime(webURL);
    }
    else
    {
        Warning("Failed to post scores online: Cannot access STEAM HTTP or Steam User!\n");
    }
}

////MOM_TODO: REMOVEME
//CON_COMMAND(mom_test_hash, "Tests SHA1 Hashing\n")
//{
//    char pathToZone[MAX_PATH];
//    char mapName[MAX_PATH];
//    V_ComposeFileName("maps", gpGlobals->mapname.ToCStr(), mapName, MAX_PATH);
//    Q_strncat(mapName, ".zon", MAX_PATH);
//    filesystem->RelativePathToFullPath(mapName, "MOD", pathToZone, MAX_PATH);
//    Log("File path is: %s\n", pathToZone);
//
//    CSHA1 sha1;
//    sha1.HashFile(pathToZone);
//    sha1.Final();
//    unsigned char hash[20];
//    sha1.GetHash(hash);
//    Log("The hash for %s is: ", mapName);
//    for (int i = 0; i < 20; i++)
//    {
//        Log("%02x", hash[i]);
//    }
//    Log("\n");
//}
//Called upon map load, loads any and all times stored in the <mapname>.tim file
void CTimer::LoadLocalTimes(const char *szMapname)
{
    char timesFilePath[MAX_PATH];
    Q_strcpy(timesFilePath, c_mapDir);
    Q_strcat(timesFilePath, szMapname, MAX_PATH);
    Q_strncat(timesFilePath, c_timesExt, MAX_PATH);
    KeyValues *timesKV = new KeyValues(szMapname);
    if (timesKV->LoadFromFile(filesystem, timesFilePath, "MOD"))
    {
        for (KeyValues *kv = timesKV->GetFirstSubKey(); kv; kv = kv->GetNextKey())
        {
            Time t;
            t.ticks = Q_atoi(kv->GetName());
            t.tickrate = kv->GetFloat("rate");
            t.date = (time_t) kv->GetInt("date");
            localTimes.AddToTail(t);
        }
    }
    else
    {
        DevLog("Failed to load local times; no local file was able to be loaded!\n");
    }
    timesKV->deleteThis();
}

//Called every time a new time is achieved
void CTimer::SaveTime()
{
    const char *szMapName = gpGlobals->mapname.ToCStr();
    KeyValues *timesKV = new KeyValues(szMapName);
    int count = localTimes.Count();

    for (int i = 0; i < count; i++)
    {
        Time t = localTimes[i];
        char timeName[512];
        Q_snprintf(timeName, 512, "%i", t.ticks);
        KeyValues *pSubkey = new KeyValues(timeName);
        pSubkey->SetFloat("rate", t.tickrate);
        pSubkey->SetInt("date", t.date);
        timesKV->AddSubKey(pSubkey);
    }

    char file[MAX_PATH];
    Q_strcpy(file, c_mapDir);
    Q_strcat(file, szMapName, MAX_PATH);
    Q_strncat(file, c_timesExt, MAX_PATH);

    if (timesKV->SaveToFile(filesystem, file, "MOD", true))
    {
        Log("Successfully saved new time!\n");
        IGameEvent *savedEvent = gameeventmanager->CreateEvent("runtime_saved");
        if (savedEvent)
            gameeventmanager->FireEvent(savedEvent);
    }

    timesKV->deleteThis();
}

void CTimer::Stop(bool endTrigger)
{
    if (endTrigger)
    {
        // Post time to leaderboards if they're online
        // and if cheats haven't been turned on this session
        if (SteamAPI_IsSteamRunning() && !m_bWereCheatsActivated)
            PostTime();

        //Save times locally too, regardless of SteamAPI condition
        Time t;
        t.ticks = gpGlobals->tickcount - m_iStartTick;
        t.tickrate = gpGlobals->interval_per_tick;
        time(&t.date);
        localTimes.AddToTail(t);

        SaveTime();
    }
    SetRunning(false);
    DispatchStateMessage();
}

void CTimer::OnMapEnd(const char *pMapName)
{
    SetCurrentCheckpointTrigger(NULL);
    SetStartTrigger(NULL);
    SetCurrentStage(NULL);
    RemoveAllCheckpoints();
    localTimes.Purge();
    //MOM_TODO: onlineTimes.RemoveAll();
}

void CTimer::OnMapStart(const char *pMapName)
{
    RequestStageCount();
    //DispatchMapStartMessage();
    LoadLocalTimes(pMapName);
    //MOM_TODO: g_Timer.LoadOnlineTimes();
}

void CTimer::RequestStageCount()
{
    CTriggerStage *stage = (CTriggerStage *) gEntList.FindEntityByClassname(NULL, "trigger_momentum_timer_stage");
    int iCount = 1;//CTriggerStart counts as one
    while (stage)
    {
        iCount++;
        stage = (CTriggerStage *) gEntList.FindEntityByClassname(stage, "trigger_momentum_timer_stage");
    }
    m_iStageCount = iCount;
}

void CTimer::DispatchResetMessage()
{
    CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
    user.MakeReliable();
    UserMessageBegin(user, "Timer_Reset");
    MessageEnd();
}

void CTimer::DispatchStageMessage()
{
    CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
    if (cPlayer && GetCurrentStage())
    {
        CSingleUserRecipientFilter user(cPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "Timer_Stage");
		WRITE_LONG(GetCurrentStage()->GetStageNumber());
        MessageEnd();
    }
}

void CTimer::DispatchStateMessage()
{
    CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
    if (cPlayer)
    {
        CSingleUserRecipientFilter user(cPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "Timer_State");
        WRITE_BOOL(m_bIsRunning);
        WRITE_LONG(m_iStartTick);
        MessageEnd();
    }
}

void CTimer::DispatchCheckpointMessage()
{
    CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
    if (cPlayer)
    {
        CSingleUserRecipientFilter user(cPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "Timer_Checkpoint");
        WRITE_BOOL(m_bUsingCPMenu);
        WRITE_LONG(m_iCurrentStepCP + 1);
        WRITE_LONG(checkpoints.Count());
        MessageEnd();
    }
}

void CTimer::DispatchStageCountMessage()
{
    CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
    if (cPlayer)
    {
        CSingleUserRecipientFilter user(cPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "Timer_StageCount");
        WRITE_LONG(m_iStageCount);
        MessageEnd();
    }
}

CON_COMMAND_F(hud_timer_request_stages, "", FCVAR_DONTRECORD | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_Timer.DispatchStageCountMessage();
}

bool CTimer::IsRunning()
{
    return m_bIsRunning;
}

void CTimer::SetRunning(bool running)
{
    m_bIsRunning = running;
}

CTriggerTimerStart *CTimer::GetStartTrigger()
{
    return m_pStartTrigger.Get();
}

void CTimer::SetGameModeConVars()
{
    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt())
    {
    case MOMGM_SURF:
        sv_maxvelocity.SetValue(3500);
        break;
    case MOMGM_BHOP:
        sv_maxvelocity.SetValue(10000);
        break;
    case MOMGM_SCROLL:
        sv_maxvelocity.SetValue(3500);
        break;
    case MOMGM_UNKNOWN:
        //sv_maxvelocity.SetValue(10000);
        //break;
    default:
        DevWarning("[%i] GameMode not defined.\n", gm.GetInt());
        break;
    }
}

//--------- CPMenu stuff --------------------------------

void CTimer::CreateCheckpoint(CBasePlayer *pPlayer)
{
    if (!pPlayer) return;
    Checkpoint c;
    c.ang = pPlayer->GetAbsAngles();
    c.pos = pPlayer->GetAbsOrigin();
    c.vel = pPlayer->GetAbsVelocity();
    checkpoints.AddToTail(c);
    // MOM_TODO: Check what gametype we're in, so we can determine if we should stop the timer or not
    g_Timer.Stop(false);
    //SetUsingCPMenu(true);
    m_iCurrentStepCP++;
    //  DispatchCheckpointMessage();
}

void CTimer::RemoveLastCheckpoint()
{
    if (checkpoints.IsEmpty()) return;
    checkpoints.Remove(m_iCurrentStepCP);
    m_iCurrentStepCP--;//If there's one element left, we still need to decrease currentStep to -1
    // DispatchCheckpointMessage();
    //if (m_iCurrentStepCP <= -1)
    //    SetUsingCPMenu(false);
}

void CTimer::TeleportToCP(CBasePlayer* cPlayer, int cpNum)
{
    if (checkpoints.IsEmpty() || !cPlayer) return;
    Checkpoint c = checkpoints[cpNum];
    cPlayer->Teleport(&c.pos, &c.ang, &c.vel);
    // DispatchCheckpointMessage();
}

//MOM_TODO: This function isn't called, CTimer is not an entity
//Rethink cheat detection
void CTimer::Think()
{
    m_cCheats = cvar->FindVar("sv_cheats");
    if (!m_bWereCheatsActivated && m_cCheats && (m_cCheats->GetInt() == 1))
    {
        m_bWereCheatsActivated = true;
        DevMsg("CHEATS ENEABLED");
    }
}

void CTimer::SetUsingCPMenu(bool pIsUsingCPMenu)
{
    m_bUsingCPMenu = pIsUsingCPMenu;
    // We notify the HUD that we've changed the status
    // (Or attemped to)

}

void CTimer::SetCurrentCPMenuStep(int pNewNum)
{
    m_iCurrentStepCP = pNewNum;
    //  DispatchCheckpointMessage();
}

//--------- CTriggerOnehop stuff --------------------------------

int CTimer::AddOnehopToListTail(CTriggerOnehop *pTrigger)
{
    return onehops.AddToTail(pTrigger);
}

bool CTimer::RemoveOnehopFromList(CTriggerOnehop *pTrigger)
{
    return onehops.FindAndRemove(pTrigger);
}

int CTimer::FindOnehopOnList(CTriggerOnehop *pTrigger)
{
    return onehops.Find(pTrigger);
}

CTriggerOnehop *CTimer::FindOnehopOnList(int pIndexOnList)
{
    return onehops.Element(pIndexOnList);
}

//--------- Commands --------------------------------

class CTimerCommands
{
public:
    static void ResetToStart()
    {
        CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
        CTriggerTimerStart *start;
        if ((start = g_Timer.GetStartTrigger()) != NULL && cPlayer)
        {
            // Don't set angles if still in start zone.
            if ( g_Timer.IsRunning() && start->GetHasLookAngles() )
            {
                QAngle ang = start->GetLookAngles();

                cPlayer->Teleport(&start->WorldSpaceCenter(), &ang, &vec3_origin);
            }
            else
            {
                cPlayer->Teleport(&start->WorldSpaceCenter(), NULL, &vec3_origin);
            }
        }
    }

    static void ResetToCheckpoint()
    {
        CTriggerStage *stage;
        CBaseEntity* pPlayer = UTIL_GetLocalPlayer();
        if ((stage = g_Timer.GetCurrentStage()) != NULL && pPlayer)
        {
            pPlayer->Teleport(&stage->WorldSpaceCenter(), NULL, &vec3_origin);
        }
    }

    static void CPMenu(const CCommand &args)
    {
        if (!g_Timer.IsUsingCPMenu())
            g_Timer.SetUsingCPMenu(true);

        if (g_Timer.IsRunning())
        {
            g_Timer.Stop(false); //Following original intentions of stopping, see MOM_TODO below

            // MOM_TODO: consider
            // 1. having a local timer running, as people may want to time their routes they're using CP menu for
            // 2. gamemodes (KZ) where this is allowed
        }
        if (args.ArgC() > 1)
        {
            int sel = Q_atoi(args[1]);
            CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
            switch (sel)
            {
            case 1://create a checkpoint
                g_Timer.CreateCheckpoint(cPlayer);
                break;

            case 2://load previous checkpoint
                g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
                break;

            case 3://cycle through checkpoints forwards (+1 % length)
                if (g_Timer.GetCPCount() > 0)
                {
                    g_Timer.SetCurrentCPMenuStep((g_Timer.GetCurrentCPMenuStep() + 1) % g_Timer.GetCPCount());
                    g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
                }
                break;

            case 4://cycle backwards through checkpoints
                if (g_Timer.GetCPCount() > 0)
                {
                    g_Timer.SetCurrentCPMenuStep(g_Timer.GetCurrentCPMenuStep() == 0 ? g_Timer.GetCPCount() - 1 : g_Timer.GetCurrentCPMenuStep() - 1);
                    g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
                }
                break;

            case 5://remove current checkpoint
                g_Timer.RemoveLastCheckpoint();
                break;
            case 6://remove every checkpoint
                g_Timer.RemoveAllCheckpoints();
                break;
            case 0://They closed the menu
                g_Timer.SetUsingCPMenu(false);
                break;
            default:
                if (cPlayer != NULL)
                {
                    cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
                }
                break;
            }
        }
        g_Timer.DispatchCheckpointMessage();
    }
};
// MOM_TODO: Command flags?

static ConCommand mom_reset_to_start("mom_restart", CTimerCommands::ResetToStart, "Restarts the player to the start trigger.\n", FCVAR_NONE);
static ConCommand mom_reset_to_checkpoint("mom_reset", CTimerCommands::ResetToCheckpoint, "Teleports the player back to the start of the current stage.\n", FCVAR_NONE);
static ConCommand mom_cpmenu("cpmenu", CTimerCommands::CPMenu, "", FCVAR_HIDDEN | FCVAR_SERVER_CAN_EXECUTE);

CTimer g_Timer;