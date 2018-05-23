#pragma once

#include "cbase.h"
#include "mom_modulecomms.h"

class CMomentumPlayer;
struct SavelocReqPacket_t;

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

    // Called when the player creates a checkpoint
    SavedLocation_t(CMomentumPlayer* pPlayer);

    // Called when saving the checkpoint to file
    void Save(KeyValues* kvCP) const;

    void Load(KeyValues* kvCP);

    // Called when the player wants to teleport to this checkpoint 
    void Teleport(CMomentumPlayer* pPlayer);

    void Read(CUtlBuffer &mem);
    void Write(CUtlBuffer &mem);
};

class CMOMSaveLocSystem : public CAutoGameSystem, public EventListener
{
public:
    CMOMSaveLocSystem(const char* pName);
    ~CMOMSaveLocSystem();

    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;

    void FireEvent(KeyValues* pKv) OVERRIDE;


    void LoadMapSaveLocs(CMomentumPlayer *pPlayer) const;

    // Online
    // Called when somebody requests our savelocs, keep track of them
    void AddSavelocRequester(const uint64 &newReq);
    // Called when we need to get the number of savelocs the player has
    int GetSavelocCount();
    // Called when a potential requester leaves the lobby/map
    void RequesterLeft(const uint64 &requester);
    // Called when we are requesting savelocs from someone
    void SetRequestingSavelocsFrom(const uint64 &from);
    // Called when a saveloc request is completed.
    // When sending (our savelocs to them), input = the nums, output = binary savelocs
    // When !sending (recv savelocs from them), input = the savelocs, output = not needed
    bool FillSavelocReq(bool sending, SavelocReqPacket_t *input, SavelocReqPacket_t *outputBuf);

private:
    KeyValues *m_pSavedLocsKV;
    CUtlVector<uint64> m_vecRequesters;
    uint64 m_iRequesting; // The Steam ID of the person we are requesting savelocs from, if any
};

extern CMOMSaveLocSystem *g_pMOMSavelocSystem;