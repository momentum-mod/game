#ifndef DOORFIX_H
#define DOORFIX_H
#ifdef _WIN32
#pragma once
#endif

#include "buttons.h"
#include "cbase.h"
#include "doors.h"
#include "mom_player.h"

#define MAX_BHOPBLOCKS 1024
#define BLOCK_TELEPORT 0.11
#define BLOCK_COOLDOWN 1.0

class CMOMBhopBlockFixSystem : CAutoGameSystem
{

  public:
    CMOMBhopBlockFixSystem(const char *pName) : CAutoGameSystem(pName) {}

    virtual void LevelInitPostEntity()
    {
        FindBhopBlocks();
        AlterBhopBlocks();
    }

    virtual void LevelShutdownPostEntity() { m_mapBlocks.RemoveAll(); }

    bool IsBhopBlock(int index) { return (m_mapBlocks.Find(index) != m_mapBlocks.InvalidIndex()); }

    void PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity *pBlock);

    void FindBhopBlocks();
    void AlterBhopBlocks();

    void FindTeleport(CBaseEntity *, bool);

    void AddBhopBlock(CBaseEntity *pBlockEnt, CBaseEntity *pTeleportEnt, bool isDoor)
    {
        bhop_block_t block = bhop_block_t();
        block.m_hBlockEntity.Set(pBlockEnt);
        block.m_hTeleportTrigger.Set(pTeleportEnt);
        block.m_bIsDoor = isDoor;
        m_mapBlocks.Insert(pBlockEnt->entindex(), block);
    }

  private:
    struct bhop_block_t
    {
        CHandle<CBaseEntity> m_hBlockEntity; // func_door or func_button
        CHandle<CBaseEntity> m_hTeleportTrigger;
        bool m_bIsDoor;
    };
    CUtlMap<int, bhop_block_t> m_mapBlocks;
};

class CTeleportTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTeleportTriggerTraceEnum(Ray_t *pRay, CBaseEntity *block, bool isDoor)
        : m_pRay(pRay), pEntBlock(block), bIsDoor(isDoor)
    {
    }

    virtual bool EnumEntity(IHandleEntity *pHandleEntity);

  private:
    bool bIsDoor;
    CBaseEntity *pEntBlock;
    Ray_t *m_pRay;
};

extern CMOMBhopBlockFixSystem *g_MOMBlockFixer;

#endif // DOORFIX_H