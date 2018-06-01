#pragma once

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

class CMOMSaveLocSystem : public CAutoGameSystem
{
public:
    CMOMSaveLocSystem(const char* pName);
    ~CMOMSaveLocSystem();

    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;

    // Online
    // Called when the UI wants to request savelocs
    void OnSavelocRequestEvent(KeyValues *pKv);
    // Called when somebody requests our savelocs, keep track of them
    void AddSavelocRequester(const uint64 &newReq);
    // Called when a potential requester leaves the lobby/map
    void RequesterLeft(const uint64 &requester);
    // Called when we are requesting savelocs from someone
    void SetRequestingSavelocsFrom(const uint64 &from);
    // Called when a saveloc request is completed.
    // When sending (our savelocs to them), input = the nums, output = binary savelocs
    // When !sending (recv savelocs from them), input = the savelocs, output = not needed
    bool FillSavelocReq(bool sending, SavelocReqPacket_t *input, SavelocReqPacket_t *outputBuf);

    // Local
    // Gets the current menu Saveloc index
    uint32 GetCurrentSavelocMenuIndex() const { return m_iCurrentSavelocIndx; }
    // Is the player currently using the saveloc menu?
    bool IsUsingSaveLocMenu() const { return m_bUsingSavelocMenu; }
    // Creates a saveloc on the location of the player
    SavedLocation_t *CreateSaveloc();
    // Creates and saves a checkpoint to the saveloc menu
    void CreateAndSaveLocation();
    // Add a Saveloc to the list
    void AddSaveloc(SavedLocation_t *saveloc);
    // Removes last saveloc (menu) form the saveloc lists
    void RemoveCurrentSaveloc();
    // Removes every saveloc (menu) on the saveloc list
    void RemoveAllSavelocs();
    // Goes to the next saveloc in the list, wrapping around
    void GotoNextSaveloc();
    // Goes to the previous saveloc in the list, wrapping around
    void GotoPrevSaveloc();
    // Goes to the first saveloc (indx 0) in the list
    void GotoFirstSaveloc();
    // Goes to the last saveloc (count - 1) in the list
    void GotoLastSaveloc();
    // Teleports the player to the saveloc (menu) with the given index
    void TeleportToSavelocIndex(int);
    // Teleports the player to their current Saved Location
    void TeleportToCurrentSaveloc();
    // Sets the current saveloc (menu) to the desired one with that index
    void SetCurrentSavelocMenuIndex(int iNewNum) { m_iCurrentSavelocIndx = iNewNum; }
    // Gets the total amount of menu savelocs
    int GetSavelocCount() const { return m_rcSavelocs.Size(); }
    // Gets a saveloc given an index (number)
    SavedLocation_t *GetSaveloc(int indx) { return indx > -1 && indx < m_rcSavelocs.Count() ? m_rcSavelocs[indx] : nullptr; }
    // Sets wheter or not we're using the Saveloc Menu
    // WARNING! No verification is done. It is up to the caller to don't give false information
    void SetUsingSavelocMenu(bool bIsUsingSLMenu);

    // Sets the momentum player
    void SetPlayer(CMomentumPlayer* pPlayer) { m_pPlayer = pPlayer; }

private:
    void CheckTimer(); // Check the timer to see if we should stop it
    void FireUpdateEvent(); // Fire tan event to the UI when we change our saveloc vector in any way, or stop using the saveloc menu
    void UpdateRequesters(); // Update any requesters with the updated saveloc count

    KeyValues *m_pSavedLocsKV;
    CUtlVector<uint64> m_vecRequesters;
    uint64 m_iRequesting; // The Steam ID of the person we are requesting savelocs from, if any

    CMomentumPlayer *m_pPlayer;

    ConVarRef gamemode;

    CUtlVector<SavedLocation_t*> m_rcSavelocs;
    int m_iCurrentSavelocIndx;
    bool m_bUsingSavelocMenu;
};

extern CMOMSaveLocSystem *g_pMOMSavelocSystem;