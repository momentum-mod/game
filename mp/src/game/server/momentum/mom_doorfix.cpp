#include "cbase.h"
#include "mom_doorfix.h"

#include "tier0/memdbgon.h"

void CMOMBhopBlockFixSystem::FindBhopBlocks()
{
    // func_doors
    CBaseEntity *ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "func_door")) != NULL)
    {
        float startpos[3], endpos[3], mins[3], maxs[3];
        CBaseDoor *pEntDoor = static_cast<CBaseDoor*>(ent);
        //DevLog("FOUND A DOOR!\n");
        //if (g_iDoorOffs_vecPosition1.Length2D() == -1)
        //{
        //g_iDoorOffs_vecPosition1 = pEntDoor->m_vecPosition1; // start pos
        //g_iDoorOffs_vecPosition2 = pEntDoor->m_vecPosition2; // end pos
        //g_iDoorOffs_flSpeed = pEntDoor->m_flSpeed;
        //g_iDoorOffs_spawnflags = pEntDoor->GetSpawnFlags();
        //g_iDoorOffs_NoiseMoving = pEntDoor->m_NoiseMoving;
        //g_iDoorOffs_sLockedSound = pEntDoor->m_ls.sLockedSound;
        //g_iDoorOffs_bLocked = pEntDoor->m_bLocked;
        //}

        pEntDoor->m_vecPosition1.CopyToArray(startpos);
        pEntDoor->m_vecPosition2.CopyToArray(endpos);

        //g_iDoorOffs_vecPosition1.CopyToArray(startpos);
        //g_iDoorOffs_vecPosition2.CopyToArray(endpos);

        if (startpos[2] > endpos[2])
        {
            DevLog("CHECKING ABS BOX!\n");
            GetAbsBoundingBox(pEntDoor, mins, maxs);
            //g_iOffs_vecMaxs.CopyToArray(maxs);
            //g_iOffs_vecMins.CopyToArray(mins);

            startpos[0] += (mins[0] + maxs[0]) * 0.5;
            startpos[1] += (mins[1] + maxs[1]) * 0.5;
            startpos[2] += maxs[2];

            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(startpos, endpos[2] + maxs[2])) != NULL)//Finds a teleport
            {
                DevLog("FOUND TELEPORT!\n");
                bhop_block_t block = bhop_block_t();
                block.m_hBlockEntity.Set(pEntDoor);
                block.m_hTeleportTrigger.Set(pEntTeleport);
                block.m_bIsDoor = true;

                m_mapBlocks.Insert(pEntDoor->entindex(), block);

                if (m_mapBlocks.Count() == MAX_BHOPBLOCKS)
                    break;
            }
        }
    }

    // func_buttons

    ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "func_button")) != NULL)
    {
        float startpos[3], endpos[3], mins[3], maxs[3];
        CBaseButton* pEntButton = static_cast<CBaseButton*>(ent);
        DevLog("FOUND A BUTTON!\n");
        //if (g_iButtonOffs_vecPosition1.Length2D() == -1)
        //{
        //    g_iButtonOffs_vecPosition1 = pEntButton->m_vecPosition1; // start pos
        //    g_iButtonOffs_vecPosition2 = pEntButton->m_vecPosition2; // end pos
        //    g_iButtonOffs_flSpeed = pEntButton->m_flSpeed;
        //    g_iDoorOffs_spawnflags = pEntButton->GetSpawnFlags();
        //}
        pEntButton->m_vecPosition1.CopyToArray(startpos);
        pEntButton->m_vecPosition2.CopyToArray(endpos);
        //g_iButtonOffs_vecPosition1.CopyToArray(startpos);
        //g_iButtonOffs_vecPosition2.CopyToArray(endpos);

        if (startpos[2] > endpos[2] && (pEntButton->HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES)))
        {
            DevLog("FOUND BUTTON 2\n");
            //g_iOffs_vecMaxs.CopyToArray(maxs);
            //g_iOffs_vecMins.CopyToArray(mins);
            GetAbsBoundingBox(pEntButton, mins, maxs);
            startpos[0] += (mins[0] + maxs[0]) * 0.5;
            startpos[1] += (mins[1] + maxs[1]) * 0.5;
            startpos[2] += maxs[2];

            CBaseEntity *pEntTeleport;
            if ((pEntTeleport = FindTeleport(startpos, endpos[2] + maxs[2])) != NULL)
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

    //float startpos[3];

    FOR_EACH_MAP_FAST(m_mapBlocks, i)
    {
        bhop_block_t block = m_mapBlocks.Element(i);

        if (block.m_bIsDoor)
        {

            //Stores old ent info MOM_TODO: maybe consider doing this?
            //GetEntDataVector(ent, g_iDoorOffs_vecPosition2, vecDoorPosition2[i]) stores its actual end vector
            //flDoorSpeed[i] = GetEntDataFloat(ent, g_iDoorOffs_flSpeed) stores its speed
            //iDoorSpawnflags[i] = GetEntData(ent, g_iDoorOffs_spawnflags, 4) stores spawn flags
            //bDoorLocked[i] = GetEntData(ent, g_iDoorOffs_bLocked, 1) ? true : false stores locked status

            //And now the settings begin
            CBaseDoor *pDoorEnt = static_cast<CBaseDoor*>(block.m_hBlockEntity.Get());

            //GetEntDataVector(ent, g_iDoorOffs_vecPosition1, startpos)
            //SetEntDataVector(ent, g_iDoorOffs_vecPosition2, startpos)
            pDoorEnt->m_vecPosition2 = pDoorEnt->m_vecPosition1;//Set the end position to start (not allowed to move)

            //SetEntDataFloat(ent, g_iDoorOffs_flSpeed, 0.0)
            pDoorEnt->m_flSpeed = 0.0;//set speed to 0 (further not allowed to move)


            //SetEntData(ent, g_iDoorOffs_spawnflags, SF_DOOR_PTOUCH, 4)
            pDoorEnt->ClearSpawnFlags();
            pDoorEnt->AddSpawnFlags(SF_DOOR_PTOUCH);//Player touch affects this

            //AcceptEntityInput(ent, "Lock")
            variant_t emptyvarient;
            pDoorEnt->AcceptInput("Lock", NULL, NULL, emptyvarient, 0);//Lock the door bhop block



            //SetEntData(ent, g_iDoorOffs_sLockedSound, GetEntData(ent, g_iDoorOffs_NoiseMoving, 4), 4)
            pDoorEnt->m_ls.sLockedSound = pDoorEnt->m_NoiseMoving;//Plays the sound like normal (makes the player aware they jumped it)

            //SDKHook(ent, SDKHook_Touch, Entity_Touch) handled by mom_player
        }
        else
        {//func_button block

            //Old data MOM_TODO: maybe consider keeping them? Incase of reverts?
            //GetEntDataVector(ent, g_iButtonOffs_vecPosition2, vecButtonPosition2[i])
            //flButtonSpeed[i] = GetEntDataFloat(ent, g_iButtonOffs_flSpeed)
            //iButtonSpawnflags[i] = GetEntData(ent, g_iButtonOffs_spawnflags, 4)
            CBaseDoor *pEntDoor = static_cast<CBaseDoor*>(block.m_hBlockEntity.Get());

            //    GetEntDataVector(ent, g_iButtonOffs_vecPosition1, startpos)
            //    SetEntDataVector(ent, g_iButtonOffs_vecPosition2, startpos)
            pEntDoor->m_vecPosition2 = pEntDoor->m_vecPosition1;

            // SetEntDataFloat(ent, g_iButtonOffs_flSpeed, 0.0)
            pEntDoor->m_flSpeed = 0.0f;
            //     SetEntData(ent, g_iButtonOffs_spawnflags, SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES, 4)
            pEntDoor->ClearSpawnFlags();

            //#define SF_BUTTON_DONTMOVE			(1<<0)		//dont move when fired
            //#define SF_BUTTON_TOUCH_ACTIVATES	(1<<8)		//button fires when touched
            pEntDoor->AddSpawnFlags(SF_BUTTON_DONTMOVE | SF_BUTTON_TOUCH_ACTIVATES);

            //SDKHook(ent, SDKHook_Touch, Entity_Touch) handled by mom_player
        }
    }
}

//MOM_TODO: This may need more work/rewriting
CBaseEntity* CMOMBhopBlockFixSystem::FindTeleport(const float startpos[3], float endheight, float step)
{
    CBaseEntity* ent = NULL;
    while ((ent = gEntList.FindEntityByClassname(ent, "trigger_teleport")) != NULL)
    {
        //DevLog("FOUND THE ACTUAL TRIGGER!\n");
        float origin[3], mins[3], maxs[3];

        origin[0] = startpos[0];
        origin[1] = startpos[1];
        origin[2] = startpos[2];
        GetAbsBoundingBox(ent, mins, maxs);

        do
        {
            bool x = origin[0] >= mins[0] && origin[0] <= maxs[0];
            bool y = origin[1] >= mins[1] && origin[1] <= maxs[1];
            bool z = origin[2] >= mins[2] && origin[2] <= maxs[2];

            if (x && y && z)
                return ent;

            origin[2] -= step;
        } while (origin[2] >= endheight);
    }

    return NULL;
}

void CMOMBhopBlockFixSystem::GetAbsBoundingBox(CBaseEntity *ent, float mins[3], float maxs[3])
{
    if (!ent) return;

    float origin[3];

    ent->GetLocalOrigin().CopyToArray(origin);
    ent->WorldAlignMins().CopyToArray(mins);
    ent->WorldAlignMaxs().CopyToArray(maxs);

    mins[0] += origin[0];
    mins[1] += origin[1];
    mins[2] += origin[2];

    maxs[0] += origin[0];
    maxs[1] += origin[1];
    maxs[2] += origin[2];
}