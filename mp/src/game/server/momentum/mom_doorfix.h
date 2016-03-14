#ifndef DOORFIX_H
#define DOORFIX_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "buttons.h"
#include "doors.h"
#include "mom_player.h"

#define MAX_BHOPBLOCKS   1024
#define BLOCK_TELEPORT   0.11
#define BLOCK_COOLDOWN   1.0

class CMOMBhopBlockFixSystem : CAutoGameSystem
{

public:
    CMOMBhopBlockFixSystem(const char* pName) : CAutoGameSystem(pName)
    {
        m_bInitted = false;
    }

    virtual void LevelInitPostEntity()
    {
        if (!m_bInitted)
        {
            DevLog("LOOKING FOR THEM!\n");
            FindBhopBlocks();
            m_bInitted = true;
        }
        //FindBhopBlocks();
        //which calls alterBhopBlocks();
    }

    virtual void LevelShutdownPostEntity()
    {
        m_bInitted = false;
        m_mapBlocks.RemoveAll();
    }

    bool IsBhopBlock(int index)
    {
        return (m_mapBlocks.Find(index) != m_mapBlocks.InvalidIndex());
    }

    void PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity* pBlock);

    void FindBhopBlocks();
    
    void AlterBhopBlocks();
    CBaseEntity *FindTeleport(float endheight, float step = 1.0f);
    void GetAbsBoundingBox(CBaseEntity *ent, Vector &mins, Vector &maxs);

private:
    bool m_bInitted;
    
    struct bhop_block_t
    {
        CHandle<CBaseEntity> m_hBlockEntity;//func_door or func_button
        CHandle<CBaseEntity> m_hTeleportTrigger;
        bool m_bIsDoor;
    };
    CUtlMap<int, bhop_block_t> m_mapBlocks;

};

extern CMOMBhopBlockFixSystem* g_MOMBlockFixer;

#endif // DOORFIX_H