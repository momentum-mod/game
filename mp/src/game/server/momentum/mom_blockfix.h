#ifndef DOORFIX_H
#define DOORFIX_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "buttons.h"
#include "doors.h"
#include "mom_player.h"

#define MAX_BHOPBLOCKS 1024
#define BLOCK_TELEPORT 0.11
#define BLOCK_COOLDOWN 1.0

class CMOMBhopBlockFixSystem : CAutoGameSystem
{

  public:
    CMOMBhopBlockFixSystem(const char *pName) : CAutoGameSystem(pName) {}

    void LevelInitPostEntity() override { FindBhopBlocks(); }

    void LevelShutdownPostEntity() override { m_mapBlocks.RemoveAll(); }

    bool IsBhopBlock(int entIndex) const { return (m_mapBlocks.Find(entIndex) != m_mapBlocks.InvalidIndex()); }

    void PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity *pBlock);

    void FindBhopBlocks();

    void FindTeleport(CBaseEntity *pBlockEnt, bool isDoor);

    void AddBhopBlock(CBaseEntity *pBlockEnt, CBaseEntity *pTeleportEnt, bool isDoor)
    {
        bhop_block_t block = bhop_block_t();
        block.m_pBlockEntity = pBlockEnt;
        block.m_pTeleportTrigger = pTeleportEnt;
        block.m_bIsDoor = isDoor;
        AlterBhopBlock(block);
        m_mapBlocks.Insert(pBlockEnt->entindex(), block);
    }

  private:
    struct bhop_block_t
    {
        CBaseEntity *m_pBlockEntity;     // func_door or func_button
        CBaseEntity *m_pTeleportTrigger; // trigger_teleport under it
        bool m_bIsDoor;
    };
    CUtlMap<int, bhop_block_t> m_mapBlocks;
    void AlterBhopBlock(bhop_block_t);
};

class CTeleportTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTeleportTriggerTraceEnum(Ray_t *pRay, CBaseEntity *block, bool isDoor)
        : bIsDoor(isDoor), pEntBlock(block), m_pRay(pRay)
    {
    }

    bool EnumEntity(IHandleEntity *pHandleEntity) override;

  private:
    bool bIsDoor;
    CBaseEntity *pEntBlock;
    Ray_t *m_pRay;
};

extern CMOMBhopBlockFixSystem *g_MOMBlockFixer;

#endif // DOORFIX_H