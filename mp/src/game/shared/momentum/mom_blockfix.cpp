#include "cbase.h"
#include "mom_blockfix.h"
#include "mom_player_shared.h"

#ifdef CLIENT_DLL
#include "c_mom_doors.h"
#include "c_mom_buttons.h"
#else
#include "doors.h"
#include "buttons.h"
#endif

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
extern CBaseEntity *FindEntityByClassnameCRC(CBaseEntity *pEnt, const unsigned int iCRC);
#endif

CMomentumBhopBlockFixSystem::CMomentumBhopBlockFixSystem(const char* pName) : CAutoGameSystem(pName)
{
    SetDefLessFunc(m_mapBlocks);
}

void CMomentumBhopBlockFixSystem::LevelInitPostEntity()
{
    FindBhopBlocks();
}

void CMomentumBhopBlockFixSystem::LevelShutdownPostEntity()
{
    m_mapBlocks.RemoveAll();
}

void CMomentumBhopBlockFixSystem::FindBhopBlocks()
{
	//  ---- func_door ----
	CBaseEntity *ent = nullptr;
#ifdef CLIENT_DLL
	CRC32_t crc;
	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, "func_door", Q_strlen("func_door"));
	CRC32_Final(&crc);

    while ((ent = FindEntityByClassnameCRC(ent, crc)) != nullptr)
#else
    while ((ent = gEntList.FindEntityByClassname(ent, "func_door")) != nullptr)
#endif
    {
        CBaseDoor *pEntDoor = static_cast<CBaseDoor *>(ent);

        Vector startpos(pEntDoor->m_vecPosition1);
        Vector endpos(pEntDoor->m_vecPosition2);

        if (startpos.z > endpos.z)
        {
#ifdef CLIENT_DLL
			Msg("[Client] Found block!! %f, %f, %f\n", pEntDoor->GetAbsOrigin().x, pEntDoor->GetAbsOrigin().y, pEntDoor->GetAbsOrigin().z);
#else
			Msg("[Server] Found block!! %f, %f, %f\n", pEntDoor->GetAbsOrigin().x, pEntDoor->GetAbsOrigin().y, pEntDoor->GetAbsOrigin().z);
#endif
            FindTeleport(pEntDoor, true);
            if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
            {
                Error("MAX_BHOPBLOCKS achieved u wut mate\n");
                break;
            }
        }
    }
    ent = nullptr;

	// ---- func_button ----
#ifdef CLIENT_DLL
	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, "func_button", Q_strlen("func_button"));
	CRC32_Final(&crc);

    while ((ent = FindEntityByClassnameCRC(ent, crc)) != nullptr)
#else
    while ((ent = gEntList.FindEntityByClassname(ent, "func_button")) != nullptr)
#endif
    {
        CBaseButton *pEntButton = static_cast<CBaseButton *>(ent);
        Vector startpos(pEntButton->m_vecPosition1);
        Vector endpos(pEntButton->m_vecPosition2);

        if (startpos.z > endpos.z && (pEntButton->HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES)))
        {
#ifdef CLIENT_DLL
			Msg("[Client] Found block!! %f, %f, %f\n", pEntButton->GetAbsOrigin().x, pEntButton->GetAbsOrigin().y, pEntButton->GetAbsOrigin().z);
#else
			Msg("[Server] Found block!! %f, %f, %f\n", pEntButton->GetAbsOrigin().x, pEntButton->GetAbsOrigin().y, pEntButton->GetAbsOrigin().z);
#endif
            FindTeleport(pEntButton, false);
            
            if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
            {
                Error("MAX_BHOPBLOCKS achieved u wut mate\n");
                break;
            }
        }
    }
}
void CMomentumBhopBlockFixSystem::AlterBhopBlock(bhop_block_t block)
{
    if (block.m_bIsDoor)
    {
        // And now the settings begin
        CBaseDoor *pDoorEnt = static_cast<CBaseDoor *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());

        pDoorEnt->m_vecPosition2 = pDoorEnt->m_vecPosition1; // Set the end position to start (not allowed to move)

        pDoorEnt->m_flSpeed = 0.0; // set speed to 0 (further not allowed to move)

        pDoorEnt->ClearSpawnFlags();
        pDoorEnt->AddSpawnFlags(SF_DOOR_PTOUCH); // Player touch affects this

#ifndef CLIENT_DLL
        variant_t emptyvarient;
        pDoorEnt->AcceptInput("Lock", nullptr, nullptr, emptyvarient, 0); // Lock the door bhop block

        // Plays the sound like normal (makes the player aware they jumped it)
        pDoorEnt->m_ls.sLockedSound = pDoorEnt->m_NoiseMoving;
#endif

        // Let the entity know that it's a bhop block, so mom_bhop_playblocksound is able to control if we should make noises
        pDoorEnt->m_bIsBhopBlock = true;

        // Fix blocks randomly stopping the player
        pDoorEnt->SetSolid(SOLID_BSP);
    }
    else
    {
        CBaseButton *pEntButton = static_cast<CBaseButton *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());
        pEntButton->m_vecPosition2 = pEntButton->m_vecPosition1;

        pEntButton->m_flSpeed = 0.0f;
        pEntButton->ClearSpawnFlags();

        pEntButton->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);

        // Let the entity know that it's a bhop block, so mom_bhop_playblocksound is able to control if we should make noises
        pEntButton->m_bIsBhopBlock = true;
    }
}

void CMomentumBhopBlockFixSystem::PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity *pBlock)
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

void CMomentumBhopBlockFixSystem::FindTeleport(CBaseEntity *pBlockEnt, bool isDoor)
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

void CMomentumBhopBlockFixSystem::AddBhopBlock(CBaseEntity* pBlockEnt, CBaseEntity* pTeleportEnt, bool isDoor)
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
#ifdef CLIENT_DLL
	CBaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle(pHandleEntity->GetRefEHandle());
#else
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());
#endif

    // Done to avoid hitting an entity that's both solid & a trigger.
    if (pEnt->IsSolid())
        return false;

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);

    if (tr.fraction < 1.0f) // tr.fraction = 1.0 means the trace completed
    {
        // arguments are initilized in the constructor of CTeleportTriggerTraceEnum
        g_MomentumBlockFixer->AddBhopBlock(pEntBlock, pEnt, bIsDoor);
        return false;//Stop, we hit our target.
    }
    //Continue until fraction == 1.0f
    return true;
}

static CMomentumBhopBlockFixSystem s_MomentumBlockFixer("CMomentumBhopBlockFixSystem");
CMomentumBhopBlockFixSystem *g_MomentumBlockFixer = &s_MomentumBlockFixer;