#include "cbase.h"
#include "mom_doorfix.h"

#include "tier0/memdbgon.h"

void CMOMBhopBlockFixSystem::FindBhopBlocks()
{
    SetDefLessFunc(m_mapBlocks);
    //  ---- func_door ----
    CBaseEntity *ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "func_door")) != NULL)
    {
        CBaseDoor *pEntDoor = static_cast<CBaseDoor *>(ent);

        Vector startpos(pEntDoor->m_vecPosition1);
        Vector endpos(pEntDoor->m_vecPosition2);

        if (startpos.z > endpos.z)
        {
            FindTeleport(pEntDoor, true);

            if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
                break;
        }
    }
    ent = NULL;

    // ---- func_button ----
    while ((ent = gEntList.FindEntityByClassname(ent, "func_button")) != NULL)
    {
        CBaseButton *pEntButton = static_cast<CBaseButton *>(ent);
        Vector startpos(pEntButton->m_vecPosition1);
        Vector endpos(pEntButton->m_vecPosition2);

        if (startpos.z > endpos.z && (pEntButton->HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES)))
        {
            FindTeleport(pEntButton, false);

            if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
                break;
        }
    }
}
void CMOMBhopBlockFixSystem::AlterBhopBlocks()
{
    FOR_EACH_MAP_FAST(m_mapBlocks, i)
    {
        bhop_block_t block = m_mapBlocks.Element(i);

        if (block.m_bIsDoor)
        {
            // And now the settings begin
            CBaseDoor *pDoorEnt = static_cast<CBaseDoor *>(block.m_hBlockEntity.Get());

            pDoorEnt->m_vecPosition2 = pDoorEnt->m_vecPosition1; // Set the end position to start (not allowed to move)

            pDoorEnt->m_flSpeed = 0.0; // set speed to 0 (further not allowed to move)

            pDoorEnt->ClearSpawnFlags();
            pDoorEnt->AddSpawnFlags(SF_DOOR_PTOUCH); // Player touch affects this

            variant_t emptyvarient;
            pDoorEnt->AcceptInput("Lock", NULL, NULL, emptyvarient, 0); // Lock the door bhop block

            pDoorEnt->m_ls.sLockedSound =
                pDoorEnt->m_NoiseMoving; // Plays the sound like normal (makes the player aware they jumped it)
        }
        else
        { // func_button block

            CBaseDoor *pEntDoor = static_cast<CBaseDoor *>(block.m_hBlockEntity.Get());
            pEntDoor->m_vecPosition2 = pEntDoor->m_vecPosition1;

            pEntDoor->m_flSpeed = 0.0f;
            pEntDoor->ClearSpawnFlags();

            pEntDoor->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);
        }
    }
}

void CMOMBhopBlockFixSystem::PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity *pBlock)
{
    CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer *>(pPlayerEnt);
    float diff = gpGlobals->curtime - pPlayer->GetPunishTime();

    if (pPlayer->GetLastBlock() != pBlock->entindex() || diff > BLOCK_COOLDOWN)
    {
        pPlayer->SetLastBlock(pBlock->entindex());
        pPlayer->SetPunishTime(gpGlobals->curtime + BLOCK_TELEPORT);
    }
    else if (diff > BLOCK_TELEPORT) // We need to teleport the player.
    {
        if (m_mapBlocks.IsValidIndex(pBlock->entindex()))
        {
            CBaseEntity *pEntTeleport = m_mapBlocks.Element(pBlock->entindex()).m_hTeleportTrigger.Get();
            if (pEntTeleport)
            {
                pEntTeleport->Touch(pPlayer);
            }
        }
    }
}

void CMOMBhopBlockFixSystem::FindTeleport(CBaseEntity *pBlockEnt, bool isDoor)
{
    // Create Vectors for the start, stop, and direction
    Vector vecAbsStart, vecAbsEnd, vecDir;

    vecDir = Vector(0, 0, -1); // Straight down

    // Get the Start/End
    vecAbsStart = pBlockEnt->GetAbsOrigin();

    vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);

    // Do the TraceLine, and write our results to our trace_t class, tr.
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd);
    CTeleportTriggerTraceEnum triggerTraceEnum(&ray, pBlockEnt, isDoor);

    enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);
}

// CON_COMMAND(mom_trace_test, "BLAH")
//{
//    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
//    if (!pPlayer)
//        return; //Always validate a pointer
//
//    //Create our trace_t class to hold the end result
//    trace_t tr;
//
//    //Create Vectors for the start, stop, and direction
//    Vector vecAbsStart, vecAbsEnd, vecDir;
//
//    //Take the Player's EyeAngles and turn it into a direction
//    AngleVectors(pPlayer->EyeAngles(), &vecDir);
//
//    //Get the Start/End
//    vecAbsStart = pPlayer->EyePosition();
//    vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);
//
//    //Do the TraceLine, and write our results to our trace_t class, tr.
//    //UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr);
//    Ray_t ray;
//    ray.Init(vecAbsStart, vecAbsEnd);
//    CTriggerTraceEnumBlah triggerTraceEnum(&ray, vecDir, MASK_ALL);
//
//    enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);
//
//    //Do something with the end results
//    /*if (tr.m_pEnt)
//    {
//        if (tr.m_pEnt->IsNPC())
//        {
//            Msg("TraceLine hit an NPC!\n");
//        }
//        if (tr.m_pEnt->IsPlayer())
//        {
//            Msg("TraceLine hit a Player!\n");
//        }
//        DevLog("Traceline ent: %s\n", tr.m_pEnt->GetClassname());
//    }  */
//}

bool CTeleportTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Done to avoid hitting an entity that's both solid & a trigger.
    if (pEnt->IsSolid())
        return true;

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);
    if (tr.fraction < 1.0f)
    {
        DevLog("HIT A TRIGGER: %s\n", pEnt->GetClassname());
        g_MOMBlockFixer->AddBhopBlock(pEntBlock, pEnt, bIsDoor);
    }
    return true;
}

static CMOMBhopBlockFixSystem s_MOMBlockFixer("CMOMBhopBlockFixSystem");
CMOMBhopBlockFixSystem *g_MOMBlockFixer = &s_MOMBlockFixer;