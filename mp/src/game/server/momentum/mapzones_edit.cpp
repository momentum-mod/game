#include "cbase.h"

#include "mom_triggers.h"
#include "mom_player.h"
#include "mapzones.h"
#include "mapzones_edit.h"

#include "tier0/memdbgon.h"


ConVar mom_zone_edit("mom_zone_edit", "0", FCVAR_CHEAT, "Toggle zone editing.\n", true, 0, true, 1);
static ConVar mom_zone_ignorewarning("mom_zone_ignorewarning", "0", FCVAR_CHEAT, "Lets you create zones despite map already having start and end.\n", true, 0, true, 1);
static ConVar mom_zone_grid("mom_zone_grid", "8", FCVAR_CHEAT, "Set grid size. 0 to disable.", true, 0, false, 0);
static ConVar mom_zone_defzone("mom_zone_defzone", "start", FCVAR_CHEAT, "If no zone type is passed to mom_zone_mark, use this.\n");
static ConVar mom_zone_start_limitspdmethod("mom_zone_start_limitspdmethod", "1", FCVAR_CHEAT, "0 = Take into account player z-velocity, 1 = Ignore z-velocity.\n", true, 0, true, 1);
static ConVar mom_zone_stage_num("mom_zone_stage_num", "0", FCVAR_CHEAT, "Set stage number. Should start from 2. 0 to automatically find one.\n", true, 0, false, 0);
static ConVar mom_zone_start_maxbhopleavespeed("mom_zone_start_maxbhopleavespeed", "250", FCVAR_CHEAT, "Max leave speed if player bhopped. 0 to disable.\n", true, 0, false, 0);
//static ConVar mom_zone_cp_num( "mom_zone_cp_num", "0", FCVAR_CHEAT, "Checkpoint number. 0 to automatically find one." );
ConVar mom_zone_debug("mom_zone_debug", "0", FCVAR_CHEAT);
ConVar mom_zone_usenewmethod("mom_zone_usenewmethod", "0", FCVAR_CHEAT, "Use the fancy new zone building method?\n");


bool CMomZoneEdit::m_bFirstEdit = false;


CMomZoneEdit g_MomZoneEdit;


void CC_Mom_ZoneZoomIn()
{
    g_MomZoneEdit.DecreaseZoom((float) mom_zone_grid.GetInt());
}

static ConCommand mom_zone_zoomin("mom_zone_zoomin", CC_Mom_ZoneZoomIn, "Decrease reticle maximum distance.\n", FCVAR_CHEAT);


void CC_Mom_ZoneZoomOut()
{
    g_MomZoneEdit.IncreaseZoom((float) mom_zone_grid.GetInt());
}

static ConCommand mom_zone_zoomout("mom_zone_zoomout", CC_Mom_ZoneZoomOut, "Increase reticle maximum distance.\n", FCVAR_CHEAT);


void CC_Mom_ZoneDelete(const CCommand &args)
{
    if (!mom_zone_edit.GetBool()) return;


    if (args.ArgC() > 1)
    {
        DevMsg("Attempting to delete '%s'\n", args[1]);

        int entindex = atoi(args[1]);

        if (entindex != 0)
        {
            CBaseEntity *pEnt = CBaseEntity::Instance(INDEXENT(entindex));

            if (pEnt && g_MomZoneEdit.GetEntityZoneType(pEnt) != -1)
            {
                UTIL_Remove(pEnt);
            }
        }
        else
        {
            char szDelete[64];
            if (ZoneTypeToClass(g_MomZoneEdit.ShortNameToZoneType(args[1]), szDelete))
            {
                CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, szDelete);
                while (pEnt)
                {
                    UTIL_Remove(pEnt);
                    pEnt = gEntList.FindEntityByClassname(pEnt, szDelete);
                }
            }
        }
    }
}

static ConCommand mom_zone_delete("mom_zone_delete", CC_Mom_ZoneDelete, "Delete zone types. Accepts start/stop/stage or an entity index.\n", FCVAR_CHEAT);


void CC_Mom_ZoneSetLook(const CCommand &args)
{
    if (!mom_zone_edit.GetBool()) return;

    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer) return;


    float yaw;

    if (args.ArgC() > 1)
    {
        yaw = atof(args[1]);
    }
    else
    {
        yaw = pPlayer->EyeAngles()[1];
    }

    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_start");
    CTriggerTimerStart *pStart;

    while (pEnt)
    {
        pStart = static_cast<CTriggerTimerStart *>(pEnt);

        if (pStart)
        {
            pStart->SetHasLookAngles(true);
            pStart->SetLookAngles(QAngle(0, yaw, 0));

            DevMsg("Set start zone angles to: %.1f, %.1f, %.1f\n", pStart->GetLookAngles()[0], pStart->GetLookAngles()[1], pStart->GetLookAngles()[2]);
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_start");
    }
}

static ConCommand mom_zone_start_setlook("mom_zone_start_setlook", CC_Mom_ZoneSetLook, "Sets start zone teleport look angles. Will take yaw in degrees or use your angles if no arguments given.\n", FCVAR_CHEAT);


void CC_Mom_ZoneMark(const CCommand &args)
{
    if (!mom_zone_edit.GetBool()) return;

    int zonetype = -1;

    //if (g_MapzoneEdit.GetBuildStage() >= BUILDSTAGE_END)
    {
        if (args.ArgC() > 1)
        {
            zonetype = g_MomZoneEdit.ShortNameToZoneType(args[1]);
        }
        else
        {
            zonetype = g_MomZoneEdit.ShortNameToZoneType(mom_zone_defzone.GetString());
        }

        if (zonetype == MOMZONETYPE_START || zonetype == MOMZONETYPE_STOP)
        {
            // Count zones to make sure we don't create multiple instances.
            int startnum = 0;
            int endnum = 0;

            CBaseEntity *pEnt;

            pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_start");
            while (pEnt)
            {
                startnum++;
                pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_start");
            }

            pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_stop");
            while (pEnt)
            {
                endnum++;
                pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_stop");
            }

            DevMsg("Found %i starts and %i ends (previous)\n", startnum, endnum);

            if (!mom_zone_ignorewarning.GetBool() && startnum && endnum)
            {
                //g_MapzoneEdit.SetBuildStage(BUILDSTAGE_NONE);

                ConMsg("Map already has a start and an end! Use mom_zone_defzone to set another type.\n");

                return;
            }

            //The user is trying to make multiple starts?
            if (zonetype == MOMZONETYPE_START)
            {
                 // Switch between start and end.
                 zonetype = (startnum <= endnum) ? MOMZONETYPE_START : MOMZONETYPE_STOP;
            }
            //else the zonetype can be STOP, allowing for multiple stop triggers to be created
        }
    }

    g_MomZoneEdit.OnMark(zonetype);
}

static ConCommand mom_zone_mark("mom_zone_mark", CC_Mom_ZoneMark, "Starts building a zone.\n", FCVAR_CHEAT);


void CC_Mom_ZoneCancel()
{
    if (!mom_zone_edit.GetBool()) return;


    g_MomZoneEdit.OnCancel();
}

static ConCommand mom_zone_cancel("mom_zone_cancel", CC_Mom_ZoneCancel, "Cancel the zone building.\n", FCVAR_CHEAT);


void CC_Mom_ZoneBack()
{
    if (!mom_zone_edit.GetBool()) return;


    g_MomZoneEdit.OnRemove();
}

static ConCommand mom_zone_back("mom_zone_back", CC_Mom_ZoneBack, "Go back a step when zone building.\n", FCVAR_CHEAT);


void CC_Mom_ZoneCreate()
{
    if (!mom_zone_edit.GetBool()) return;


    g_MomZoneEdit.OnCreate(-1);
}

static ConCommand mom_zone_create("mom_zone_create", CC_Mom_ZoneCreate, "Create the zone.\n", FCVAR_CHEAT);


CMomZoneEdit::CMomZoneEdit() : CAutoGameSystemPerFrame("MomentumZoneBuilder")
{
    m_bEditing = false;


    m_flReticleDist = 1024.0f;


    m_iPrevBuilder = -1;
    m_pBuilder = nullptr;
}

CMomZoneEdit::~CMomZoneEdit()
{
}

void CMomZoneEdit::StopEditing()
{
    m_bEditing = false;


    mom_zone_edit.SetValue( 0 );


    m_iPrevBuilder = -1;
    delete m_pBuilder;
    m_pBuilder = nullptr;
}

void CMomZoneEdit::OnMark(int zonetype)
{
    // Player wants to mark a point (ie. "next build step" in the old method)

    auto pPlayer = GetPlayerBuilder();
    if (!pPlayer)
        return;


    auto pBuilder = GetBuilder();

    Vector pos;
    GetCurrentBuildSpot(pPlayer, pos);


    

    if (pBuilder->IsDone())
        pBuilder->Reset();

    pBuilder->Add(pPlayer, pos);

    // Builder may want to create the zone NOW, instead of manually.
    if (pBuilder->CheckOnMark() && pBuilder->IsReady())
    {
        OnCreate(zonetype);
    }
}

void CMomZoneEdit::OnCreate(int zonetype)
{
    // Player wants to create the zone.

    auto pPlayer = GetPlayerBuilder();
    Vector pos;
    GetCurrentBuildSpot(pPlayer, pos);


    auto pBuild = GetBuilder();
    if (!pBuild->BuildZone(pPlayer, &pos))
    {
        return;
    }



    DevMsg("Creating entity...\n");


    int type = zonetype != -1 ? zonetype : ShortNameToZoneType(mom_zone_defzone.GetString());


    auto pEnt = CreateZoneEntity(type);
    if (!pEnt)
    {
        Warning("Couldn't create zone ent!\n");
        return;
    }

    pEnt->AddSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS);
    pEnt->Spawn();


    pBuild->FinishZone(pEnt);

    
    pEnt->Activate();


    SetZoneProps(pEnt);


    DevMsg("Created zone entity %i.\n", pEnt->entindex());
}

void CMomZoneEdit::OnRemove()
{
    // Player wants to go back a step.
    auto pPlayer = GetPlayerBuilder();
    if (!pPlayer)
        return;


    Vector pos;
    GetCurrentBuildSpot(pPlayer, pos);

    GetBuilder()->Remove(pPlayer, pos);
}


void CMomZoneEdit::OnCancel()
{
    // Remove it completely
    GetBuilder()->Reset();
}

bool CMomZoneEdit::GetCurrentBuildSpot(CMomentumPlayer *pPlayer, Vector &vecPos)
{
    trace_t tr;
    Vector fwd;

    pPlayer->EyeVectors(&fwd);

    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + fwd * m_flReticleDist, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    vecPos = tr.endpos;


    if (mom_zone_grid.GetInt() > 0)
        VectorSnapToGrid(vecPos, (float)mom_zone_grid.GetInt());

    return true;
}

CMomBaseZoneBuilder *CMomZoneEdit::GetBuilder()
{
    int method = mom_zone_usenewmethod.GetInt();

    if (m_iPrevBuilder != method || !m_pBuilder)
    {
        delete m_pBuilder;

        if (method)
        {
            m_pBuilder = new CMomPointZoneBuilder;
        }
        else
        {
            m_pBuilder = new CMomBoxZoneBuilder;
        }
    }

    m_iPrevBuilder = method;

    return m_pBuilder;
}

CMomentumPlayer *CMomZoneEdit::GetPlayerBuilder() const
{
    return static_cast<CMomentumPlayer *>(UTIL_GetLocalPlayer());
}

void CMomZoneEdit::LevelInitPostEntity()
{
    StopEditing();
}

void CMomZoneEdit::FrameUpdatePostEntityThink()
{
    if (mom_zone_edit.GetBool())
    {
        if (!IsEditing())
        {
            m_bEditing = true;

            // Send message to client to excuse bad zone building
            if (!m_bFirstEdit)
            {
                CSingleUserRecipientFilter filter(UTIL_GetLocalPlayer());
                filter.MakeReliable();
                UserMessageBegin(filter, "MB_EditingZone");
                MessageEnd();
            }
            m_bFirstEdit = true;
        }
    }
    else
    {
        if ( IsEditing() )
            StopEditing();

        return;
    }



    auto pPlayer = GetPlayerBuilder();
    if (!pPlayer)
        return;

    Vector vecAim;
    if (!GetCurrentBuildSpot(pPlayer, vecAim))
        return;


    GetBuilder()->OnFrame(pPlayer, vecAim);
    


    DrawReticle(vecAim,8.0f);
}

void CMomZoneEdit::VectorSnapToGrid(Vector &dest, float gridsize)
{
    dest.x = SnapToGrid(dest.x, gridsize);
    dest.y = SnapToGrid(dest.y, gridsize);
    dest.z = SnapToGrid(dest.z, gridsize);
}

float CMomZoneEdit::SnapToGrid(float fl, float gridsize)
{
    float closest;
    float dif;

    closest = fl - fmodf(fl, gridsize);

    dif = fl - closest;

    if (dif > (gridsize / 2.0f))
    {
        closest += gridsize;
    }
    else if (dif < (-gridsize / 2.0f))
    {
        closest -= gridsize;
    }

    return closest;
}

void CMomZoneEdit::DrawReticle(const Vector &pos, float retsize)
{
    Vector p1, p2, p3, p4, p5, p6;

    p1 = pos;
    p1.x = pos.x + retsize;

    p2 = pos;
    p2.x = pos.x - retsize;

    p3 = pos;
    p3.y = pos.y + retsize;

    p4 = pos;
    p4.y = pos.y - retsize;

    p5 = pos;
    p5.z = pos.z + retsize;

    p6 = pos;
    p6.z = pos.z - retsize;

    DebugDrawLine(p1, p2, 255, 0, 0, true, -1.0f);
    DebugDrawLine(p3, p4, 0, 255, 0, true, -1.0f);
    DebugDrawLine(p5, p6, 0, 0, 255, true, -1.0f);
}

extern bool ZoneTypeToClass(int type, char *dest);

CBaseMomentumTrigger* CMomZoneEdit::CreateZoneEntity(int type)
{
    char szClass[64];
    if (!ZoneTypeToClass(type, szClass))
    {
        return nullptr;
    }

    auto pEnt = CreateEntityByName(szClass);
    auto pRet = dynamic_cast<CBaseMomentumTrigger*>(pEnt);

    // Not a valid momentum trigger, delete it.
    if (!pRet && pEnt)
        UTIL_RemoveImmediate(pEnt);

    return pRet;
}

void CMomZoneEdit::SetZoneProps(CBaseEntity *pEnt)
{
    CTriggerTimerStart *pStart = dynamic_cast<CTriggerTimerStart *>(pEnt);
    //validate pointers
    if (pStart)
    {
        ConVarRef ref("mom_zone_start_maxbhopleavespeed");
        //bhop speed limit
        if (ref.GetFloat() > 0.0)
        {
            pStart->SetMaxLeaveSpeed(ref.GetFloat());
            pStart->SetIsLimitingSpeed(true);
        }
        else
        {
            pStart->SetIsLimitingSpeed(false);
        }
        return;
    }

    CTriggerStage *pStage = dynamic_cast<CTriggerStage *>(pEnt);
    if (pStage)
    {
        ConVarRef ref("mom_zone_stage_num");
        if (ref.GetInt() > 0)
        {
            pStage->SetStageNumber(ref.GetInt());
        }
        else
        {
            int higheststage = 1;
            CTriggerStage *pTempStage;

            CBaseEntity *pTemp = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_stage");
            while (pTemp)
            {
                pTempStage = static_cast<CTriggerStage *>(pTemp);

                if (pTempStage && pTempStage->GetStageNumber() > higheststage)
                {
                    higheststage = pTempStage->GetStageNumber();
                }

                pTemp = gEntList.FindEntityByClassname(pTemp, "trigger_momentum_timer_stage");
            }

            pStage->SetStageNumber(higheststage + 1);
        }

        return;
    }
}

int CMomZoneEdit::GetEntityZoneType(CBaseEntity *pEnt)
{
    CTriggerTimerStart *pStart = dynamic_cast<CTriggerTimerStart *>(pEnt);
    if (pStart) return MOMZONETYPE_START;

    /*CTriggerTeleportCheckpoint *pCP = dynamic_cast<CTriggerTeleportCheckpoint *>( pEnt );
    if ( pCP ) return 1;*/

    CTriggerTimerStop *pStop = dynamic_cast<CTriggerTimerStop *>(pEnt);
    if (pStop) return MOMZONETYPE_STOP;

    CTriggerStage *pStage = dynamic_cast<CTriggerStage *>(pEnt);
    if (pStage) return MOMZONETYPE_STAGE;

    return -1;
}

int CMomZoneEdit::ShortNameToZoneType(const char *in)
{
    if (Q_stricmp(in, "start") == 0)
    {
        return MOMZONETYPE_START;
    }
    else if (Q_stricmp(in, "end") == 0 || Q_stricmp(in, "stop") == 0)
    {
        return MOMZONETYPE_STOP;
    }
    else if (Q_stricmp(in, "stage") == 0)
    {
        return MOMZONETYPE_STAGE;
    }

    return -1;
}
