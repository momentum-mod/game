#pragma once

#include "eventqueue.h"

class CMomentumPlayer;
class SavelocReqPacket;

#define SAVELOC_FILE_NAME "savedlocs.txt"

enum SavedLocationComponent_t
{
    SAVELOC_NONE = 0,

    SAVELOC_POS = 1 << 0,
    SAVELOC_VEL = 1 << 1,
    SAVELOC_ANG = 1 << 2,
    SAVELOC_TARGETNAME = 1 << 3,
    SAVELOC_CLASSNAME = 1 << 4,
    SAVELOC_GRAVITY = 1 << 5,
    SAVELOC_MOVEMENTLAG = 1 << 6,
    SAVELOC_DISABLED_BTNS = 1 << 7,
    SAVELOC_EVENT_QUEUE = 1 << 8,
    SAVELOC_DUCKED = 1 << 9,
    SAVELOC_TRACK = 1 << 10,
    SAVELOC_ZONE = 1 << 11,
    SAVELOC_TOGGLED_BTNS = 1 << 12,

    SAVELOC_ALL = ~SAVELOC_NONE,
};

// Saved Location used in the "Saveloc menu"
struct SavedLocation_t
{
    bool m_bCrouched;
    Vector m_vecPos;
    Vector m_vecVel;
    QAngle m_qaAng;
    char m_szTargetName[512];
    char m_szTargetClassName[512];
    float m_fGravityScale;
    float m_fMovementLagScale;
    int m_iDisabledButtons;
    int m_iToggledButtons;
    int m_iTrack, m_iZone;
    CEventQueueState entEventsState;

    int m_savedComponents;

    SavedLocation_t();

    // Called when the player creates a checkpoint
    SavedLocation_t(CMomentumPlayer* pPlayer, int components = SAVELOC_ALL);

    // Called when saving the checkpoint to file
    void Save(KeyValues* kvCP) const;

    void Load(KeyValues* kvCP);

    // Called when the player wants to teleport to this checkpoint 
    void Teleport(CMomentumPlayer* pPlayer);

    bool Read(CUtlBuffer &mem);
    bool Write(CUtlBuffer &mem);
};

class CSaveLocSystem : public CAutoGameSystem
{
public:
    CSaveLocSystem(const char* pName);
    ~CSaveLocSystem();
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;

    // Online
    // Called when the UI wants to request savelocs
    void OnSavelocRequestEvent(KeyValues *pKv);
    // Called when somebody requests our savelocs, keep track of them
    bool AddSavelocRequester(const uint64 &newReq);
    // Called when a potential requester leaves the lobby/map
    void RequesterLeft(const uint64 &requester);
    // Called when we are requesting savelocs from someone
    void SetRequestingSavelocsFrom(const uint64 &from);
    uint64 GetRequestingSavelocsFrom() const { return m_iRequesting; }

    bool WriteRequestedSavelocs(SavelocReqPacket *input, SavelocReqPacket *output, const uint64 &requester);
    bool ReadReceivedSavelocs(SavelocReqPacket *input, const uint64 &sender);

    // Local
    // Loads start marks from saveloc file
    bool LoadStartMarks();
    // Gets the current menu Saveloc index
    uint32 GetCurrentSavelocMenuIndex() const { return m_iCurrentSavelocIndx; }
    // Is the player currently using the saveloc menu?
    bool IsUsingSaveLocMenu() const { return m_bUsingSavelocMenu; }
    // Creates a saveloc on the location of the player
    SavedLocation_t *CreateSaveloc(int components = SAVELOC_ALL);
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
    // Goes to the first saveloc (index 0) in the list
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
    // Sets whether or not we're using the Saveloc Menu
    // WARNING! No verification is done. It is up to the caller to don't give false information
    void SetUsingSavelocMenu(bool bIsUsingSLMenu);

private:
    void CheckTimer(); // Check the timer to see if we should stop it
    void FireUpdateEvent() const; // Fire tan event to the UI when we change our saveloc vector in any way, or stop using the saveloc menu
    void UpdateRequesters(); // Update any requesters with the updated saveloc count

    KeyValues *m_pSavedLocsKV;
    CUtlVector<uint64> m_vecRequesters;
    uint64 m_iRequesting; // The Steam ID of the person we are requesting savelocs from, if any

    CUtlVector<SavedLocation_t*> m_rcSavelocs;
    int m_iCurrentSavelocIndx;
    bool m_bUsingSavelocMenu;
};

extern CSaveLocSystem *g_pSavelocSystem;