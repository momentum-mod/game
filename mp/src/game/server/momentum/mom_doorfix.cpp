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
void CMOMBhopBlockFixSystem::AlterBhopBlock(bhop_block_t block)
{
    if (block.m_bIsDoor)
    {
        // And now the settings begin
        CBaseDoor *pDoorEnt = static_cast<CBaseDoor *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());

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

        CBaseButton *pEntButton = static_cast<CBaseButton *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());
        pEntButton->m_vecPosition2 = pEntButton->m_vecPosition1;

        pEntButton->m_flSpeed = 0.0f;
        pEntButton->ClearSpawnFlags();

        pEntButton->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);
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
        int idx = m_mapBlocks.Find(pBlock->entindex());
        if (m_mapBlocks.IsValidIndex(idx))
        {
            CBaseEntity *pEntTeleport = m_mapBlocks.Element(idx).m_pTeleportTrigger;
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
    //move vector to top of door
    vecAbsStart.z += pBlockEnt->WorldAlignMaxs().z;

    vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);

    // Do the TraceLine, and write our results to our trace_t class, tr.
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd);
    CTeleportTriggerTraceEnum triggerTraceEnum(&ray, pBlockEnt, isDoor);

    enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);
}

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
        DevLog("Entindex of TP: %i", pEnt->entindex());
        DevLog("Entindex of door: %i", pEntBlock->entindex());
        g_MOMBlockFixer->AddBhopBlock(pEntBlock, pEnt, bIsDoor);
    }
    return true;
}

static CMOMBhopBlockFixSystem s_MOMBlockFixer("CMOMBhopBlockFixSystem");
CMOMBhopBlockFixSystem *g_MOMBlockFixer = &s_MOMBlockFixer;