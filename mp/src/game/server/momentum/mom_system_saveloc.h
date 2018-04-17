#pragma once

#include "cbase.h"

class CMomentumPlayer;

// Saved Location used in the "Saveloc menu"
struct SavedLocation_t
{
    bool crouched;
    Vector pos;
    Vector vel;
    QAngle ang;
    char targetName[512];
    char targetClassName[512];
    float gravityScale;
    float movementLagScale;
    int disabledButtons;

    SavedLocation_t();

    // Called when loading this checkpoint from file
    SavedLocation_t(KeyValues* pKv);

    // Called when the player creates a checkpoint
    SavedLocation_t(CMomentumPlayer* pPlayer);

    // Called when saving the checkpoint to file
    void Save(KeyValues* kvCP) const;

    // Called when the player wants to teleport to this checkpoint 
    void Teleport(CMomentumPlayer* pPlayer);
};

class CMOMSaveLocSystem : CAutoGameSystem
{
public:
    CMOMSaveLocSystem(const char* pName) : CAutoGameSystem(pName)
    {
        m_pSavedLocsKV = new KeyValues(pName);
    }
    ~CMOMSaveLocSystem()
    {
        if (m_pSavedLocsKV)
            m_pSavedLocsKV->deleteThis();
        m_pSavedLocsKV = nullptr;
    }

    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;

    void LoadMapSaveLocs(CMomentumPlayer *pPlayer) const;

private:

    KeyValues *m_pSavedLocsKV;
};

extern CMOMSaveLocSystem *g_MOMSavelocSystem;