#ifndef _MOM_BLOCKFIX_H_
#define _MOM_BLOCKFIX_H_

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL
	#define CMomentumBhopBlockFixSystem C_MomentumBhopBlockFixSystem
#endif

#define MAX_BHOPBLOCKS 1024
#define BLOCK_TELEPORT 0.11
#define BLOCK_COOLDOWN 1.0

class CMomentumBhopBlockFixSystem : CAutoGameSystem
{
  public:
    CMomentumBhopBlockFixSystem(const char *pName);

    // GameSystem overrides
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    // Called from player
    bool IsBhopBlock(const int &entIndex) const { return (m_mapBlocks.Find(entIndex) != m_mapBlocks.InvalidIndex()); }
    void PlayerTouch(CBaseEntity *pPlayerEnt, CBaseEntity *pBlock);

    // Function
    void FindBhopBlocks();
    void FindTeleport(CBaseEntity *pBlockEnt, bool isDoor);
    void AddBhopBlock(CBaseEntity *pBlockEnt, CBaseEntity *pTeleportEnt, bool isDoor);

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

    bool EnumEntity(IHandleEntity *pHandleEntity) OVERRIDE;

  private:
    bool bIsDoor;
    CBaseEntity *pEntBlock;
    Ray_t *m_pRay;
};

extern CMomentumBhopBlockFixSystem *g_MomentumBlockFixer;

#endif