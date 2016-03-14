#include "cbase.h"
#include "mom_doorfix.h"

#include "tier0/memdbgon.h"


void CMOMBhopBlockFixSystem::FindBhopBlocks()
{
    SetDefLessFunc(m_mapBlocks);
    // func_doors
    CBaseEntity *ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "func_door")) != NULL)
    {
        CBaseDoor *pEntDoor = static_cast<CBaseDoor*>(ent);
        DevLog("FOUND A DOOR!\n");
        
        Vector startpos (pEntDoor->m_vecPosition1);
        Vector endpos (pEntDoor->m_vecPosition2);
        Vector mins = ent->WorldAlignMins();
        Vector maxs = ent->WorldAlignMaxs();

        if (startpos.z > endpos.z)
        {
            DevLog("CHECKING ABS BOX!\n");

            GetAbsBoundingBox(pEntDoor, mins, maxs);

            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(endpos.z + maxs.z)) != NULL)//Finds a teleport
            {
                DevLog("FOUND TELEPORT!\n");
                bhop_block_t block = bhop_block_t();
                block.m_hBlockEntity.Set(pEntDoor);
                block.m_hTeleportTrigger.Set(pEntTeleport);
                block.m_bIsDoor = true;
                //Assert(m_mapBlocks.Find(pEntDoor->entindex()) == m_mapBlocks.InvalidIndex());
                m_mapBlocks.Insert(pEntDoor->entindex(), block);
                
                if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
                    break;
            }
        }
    }
    ent = NULL;

    // ---- func_button ----
    while ((ent = gEntList.FindEntityByClassname(ent, "func_button")) != NULL)
    {
        Vector mins, maxs;
        CBaseButton* pEntButton = static_cast<CBaseButton*>(ent);
        DevLog("FOUND A BUTTON!\n");

        Vector startpos(pEntButton->m_vecPosition1);
        Vector endpos(pEntButton->m_vecPosition2);


        if (startpos.z > endpos.z && (pEntButton->HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES)))
        {
            DevLog("FOUND BUTTON 2\n");
            GetAbsBoundingBox(pEntButton, mins, maxs);

            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(endpos.z + maxs.z)) != NULL)
            {
                DevLog("FOUND TELEPORT!\n");
                bhop_block_t block = bhop_block_t();
                block.m_hBlockEntity.Set(pEntButton);
                block.m_hTeleportTrigger.Set(pEntTeleport);
                block.m_bIsDoor = false;

                m_mapBlocks.Insert(pEntButton->entindex(), block);

                if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
                    break;
            }
        }
    }

    AlterBhopBlocks();
}
void CMOMBhopBlockFixSystem::AlterBhopBlocks()
{
    FOR_EACH_MAP_FAST(m_mapBlocks, i)
    {
        bhop_block_t block = m_mapBlocks.Element(i);

        if (block.m_bIsDoor)
        {

            //And now the settings begin
            CBaseDoor *pDoorEnt = static_cast<CBaseDoor*>(block.m_hBlockEntity.Get());

            pDoorEnt->m_vecPosition2 = pDoorEnt->m_vecPosition1;//Set the end position to start (not allowed to move)

            pDoorEnt->m_flSpeed = 0.0;//set speed to 0 (further not allowed to move)

            pDoorEnt->ClearSpawnFlags();
            pDoorEnt->AddSpawnFlags(SF_DOOR_PTOUCH);//Player touch affects this

            variant_t emptyvarient;
            pDoorEnt->AcceptInput("Lock", NULL, NULL, emptyvarient, 0);//Lock the door bhop block

            pDoorEnt->m_ls.sLockedSound = pDoorEnt->m_NoiseMoving;//Plays the sound like normal (makes the player aware they jumped it)

            //SDKHook(ent, SDKHook_Touch, Entity_Touch) handled by mom_player
        }
        else
        {//func_button block

            CBaseDoor *pEntDoor = static_cast<CBaseDoor*>(block.m_hBlockEntity.Get());
            pEntDoor->m_vecPosition2 = pEntDoor->m_vecPosition1;

            pEntDoor->m_flSpeed = 0.0f;
            pEntDoor->ClearSpawnFlags();

            pEntDoor->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);

            //SDKHook(ent, SDKHook_Touch, Entity_Touch) handled by mom_player
        }
    }
}

//MOM_TODO: This may need more work/rewriting
CBaseEntity* CMOMBhopBlockFixSystem::FindTeleport(float endheight, float step)
{
    CBaseEntity* ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "trigger_teleport")) != NULL)
    {
        //DevLog("FOUND THE ACTUAL TRIGGER!\n");
        float origin[3];

        ent->GetLocalOrigin().CopyToArray(origin);
        Vector mins = ent->WorldAlignMins();
        Vector maxs = ent->WorldAlignMaxs();

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
        while (origin[2] >= endheight);
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


void CMOMBhopBlockFixSystem::PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity* pBlock)
{
    CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer*>(pPlayerEnt);
    DevLog("ENTERING PLAYERTOUCH!\n");
    float diff = gpGlobals->curtime - pPlayer->GetPunishTime();

    if (pPlayer->GetLastBlock() != pBlock->entindex() || diff > BLOCK_COOLDOWN)
    {
        pPlayer->SetLastBlock(pBlock->entindex());
        pPlayer->SetPunishTime(gpGlobals->curtime + BLOCK_TELEPORT);
    }
    else if (diff > BLOCK_TELEPORT)//We need to teleport the player.
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