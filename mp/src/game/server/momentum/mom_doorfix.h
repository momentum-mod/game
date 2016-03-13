#ifndef DOORFIX_H
#define DOORFIX_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "buttons.h"
#include "doors.h"

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

    void PlayerTouch(CBaseEntity *pPlayer, CBaseEntity* pBlock)
    {
        //MOM_TODO: should these move to mom_player ?
        static float flPunishTime = -1;
        static int iLastBlock = -1;

        float diff = gpGlobals->curtime - flPunishTime;

        if (iLastBlock != pBlock->entindex() || diff > BLOCK_COOLDOWN)
        {
            iLastBlock = pBlock->entindex();
            flPunishTime = gpGlobals->curtime + BLOCK_TELEPORT;
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

    void FindBhopBlocks();

    void AlterBhopBlocks();
    CBaseEntity *FindTeleport(float endheight, float step = 1.0f);
    void GetAbsBoundingBox(CBaseEntity *ent, Vector &mins, Vector &maxs);

private:
    bool m_bInitted;
    const unsigned int       MAX_BHOPBLOCKS = 1024;
    const double             BLOCK_TELEPORT = 0.11;
    const double             BLOCK_COOLDOWN = 1.0;

    struct bhop_block_t
    {
        CHandle<CBaseEntity> m_hBlockEntity;//func_door or func_button
        CHandle<CBaseEntity> m_hTeleportTrigger;
        bool m_bIsDoor;
    };
    CUtlMap<int, bhop_block_t> m_mapBlocks;

};

static CMOMBhopBlockFixSystem g_MOMBlockFixer("CMOMBhopBlockFixSystem");

#endif // DOORFIX_H