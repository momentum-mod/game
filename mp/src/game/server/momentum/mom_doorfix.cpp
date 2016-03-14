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
            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(pEntDoor)) != NULL)
                AddBhopBlock(pEntDoor, pEntTeleport, true);

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
            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(pEntButton)) != NULL)
                AddBhopBlock(pEntButton, pEntTeleport, true);

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
        CBaseDoor *pEntDoor = static_cast<CBaseDoor *>(block.m_pBlockEntity); //(block.m_hBlockEntity.Get());

        pEntDoor->m_vecPosition2 = pEntDoor->m_vecPosition1; // Set the end position to start (not allowed to move)

        pEntDoor->m_flSpeed = 0.0; // set speed to 0 (further not allowed to move)

        pEntDoor->ClearSpawnFlags();
        pEntDoor->AddSpawnFlags(SF_DOOR_PTOUCH); // Player touch affects this

        variant_t emptyvarient;
        pEntDoor->AcceptInput("Lock", NULL, NULL, emptyvarient, 0); // Lock the door bhop block

        pEntDoor->m_ls.sLockedSound =
            pEntDoor->m_NoiseMoving; // Plays the sound like normal (makes the player aware they jumped it)
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
CBaseEntity* CMOMBhopBlockFixSystem::FindTeleport(CBaseEntity *pOther, float step)
{
    CBaseEntity* ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "trigger_teleport")) != NULL)
    {
        //DevLog("FOUND THE ACTUAL TRIGGER!\n");
        float origin[3];

        pOther->GetLocalOrigin().CopyToArray(origin);
        Vector mins = ent->WorldAlignMins();
        Vector maxs = ent->WorldAlignMaxs();

        //double each member of mins and maxs since origin should be in the entitiy's world space center
        GetAbsBoundingBox(ent, mins, maxs);

        do
        {
            bool x = origin[0] >= mins.x && origin[0] <= maxs.x;
            bool y = origin[1] >= mins.y && origin[1] <= maxs.y;
            bool z = origin[2] >= mins.z && origin[2] <= maxs.z;

            if (x && y && z)
                return ent;

            origin[2] -= step;
        } 
        while (origin[2] >= ent->WorldAlignMins().z);
    }

    return NULL;
}
void CMOMBhopBlockFixSystem::GetAbsBoundingBox(CBaseEntity *ent, Vector &mins, Vector &maxs)
{
    if (!ent) return;

    Vector origin(ent->GetLocalOrigin());

    mins = ent->WorldAlignMins();
    maxs = ent->WorldAlignMaxs();

    mins.x += origin.x;
    mins.y += origin.y;
    mins.z += origin.z;

    maxs.x += origin.x;
    maxs.y += origin.y;
    maxs.z += origin.z;

}
static CMOMBhopBlockFixSystem s_MOMBlockFixer("CMOMBhopBlockFixSystem");
CMOMBhopBlockFixSystem *g_MOMBlockFixer = &s_MOMBlockFixer;