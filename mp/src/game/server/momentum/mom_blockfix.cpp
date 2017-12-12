#include "cbase.h"
#include "mom_blockfix.h"
#include "doors.h"
#include "buttons.h"
#include "mom_player.h"

#include "tier0/memdbgon.h"

CMOMBhopBlockFixSystem::CMOMBhopBlockFixSystem(const char* pName) : CAutoGameSystem(pName)
{
    SetDefLessFunc(m_mapBlocks);
}

void CMOMBhopBlockFixSystem::LevelInitPostEntity()
{
    FindBhopBlocks();
}

void CMOMBhopBlockFixSystem::LevelShutdownPostEntity()
{
    m_mapBlocks.RemoveAll();
}

void CMOMBhopBlockFixSystem::FindBhopBlocks()
{
    //  ---- func_door ----
    CBaseEntity *ent = nullptr;
    while ((ent = gEntList.FindEntityByClassname(ent, "func_door")) != nullptr)
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
    ent = nullptr;

    // ---- func_button ----
    while ((ent = gEntList.FindEntityByClassname(ent, "func_button")) != nullptr)
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
        pDoorEnt->AcceptInput("Lock", nullptr, nullptr, emptyvarient, 0); // Lock the door bhop block

        // Plays the sound like normal (makes the player aware they jumped it)
        pDoorEnt->m_ls.sLockedSound = pDoorEnt->m_NoiseMoving;

        // Let the entity know that it's a bhop block, so mom_bhop_playblocksound is able to control if we should make noises
        pDoorEnt->m_bIsBhopBlock = true;

        // Fix blocks randomly stopping the player
        pDoorEnt->SetSolid(SOLID_BSP);
    }
    else
    { // func_button block

        CBaseButton *pEntButton = static_cast<CBaseButton *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());
        pEntButton->m_vecPosition2 = pEntButton->m_vecPosition1;

        pEntButton->m_flSpeed = 0.0f;
        pEntButton->ClearSpawnFlags();

        pEntButton->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);

        // Let the entity know that it's a bhop block, so mom_bhop_playblocksound is able to control if we should make noises
        pEntButton->m_bIsBhopBlock = true;
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
    if (!pBlockEnt->CollisionProp()->IsBoundsDefinedInEntitySpace())
        return;
    // Create Vectors for the start, stop, and direction
    Vector vecAbsStart, vecAbsEnd, vecDir;

    vecDir = Vector(0, 0, -1); // Straight down

    // Get the Start/End
    vecAbsStart = pBlockEnt->WorldSpaceCenter();
    // move vector to top of bhop block
    vecAbsStart.z += pBlockEnt->CollisionProp()->OBBMaxs().z;

    // ray is as long as the bhop block is tall
    vecAbsEnd = vecAbsStart + (vecDir * (pBlockEnt->CollisionProp()->OBBMaxs().z - pBlockEnt->CollisionProp()->OBBMins().z));

    // Do the TraceLine, and write our results to our trace_t class, tr.
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd);
    CTeleportTriggerTraceEnum triggerTraceEnum(&ray, pBlockEnt, isDoor);

    enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);
}

void CMOMBhopBlockFixSystem::AddBhopBlock(CBaseEntity* pBlockEnt, CBaseEntity* pTeleportEnt, bool isDoor)
{
    bhop_block_t block = bhop_block_t();
    block.m_pBlockEntity = pBlockEnt;
    block.m_pTeleportTrigger = pTeleportEnt;
    block.m_bIsDoor = isDoor;
    AlterBhopBlock(block);
    m_mapBlocks.Insert(pBlockEnt->entindex(), block);
}

// override of IEntityEnumerator's EnumEntity() for our trigger teleport filter
bool CTeleportTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    // store entity that we found on the trace
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Done to avoid hitting an entity that's both solid & a trigger.
    if (pEnt->IsSolid())
        return false;

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);

    if (tr.fraction < 1.0f) // tr.fraction = 1.0 means the trace completed
    {
        // arguments are initilized in the constructor of CTeleportTriggerTraceEnum
        g_MOMBlockFixer->AddBhopBlock(pEntBlock, pEnt, bIsDoor);
        return false;//Stop, we hit our target.
    }
    //Continue until fraction == 1.0f
    return true;
}

static CMOMBhopBlockFixSystem s_MOMBlockFixer("CMOMBhopBlockFixSystem");
CMOMBhopBlockFixSystem *g_MOMBlockFixer = &s_MOMBlockFixer;