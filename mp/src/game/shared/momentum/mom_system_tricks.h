#pragma once

#include "igamesystem.h"

#define TRICK_DATA_KEY "TrickData"

#ifdef CLIENT_DLL
#define CMomentumPlayer C_MomentumPlayer
#define CTriggerTrickZone C_TriggerTrickZone
#endif

class CMomentumPlayer;
class CTriggerTrickZone;
struct SavedLocation_t;

enum TrickTrackingDrawState_t
{
    TRICK_DRAW_NONE = 0,
    TRICK_DRAW_START,
    TRICK_DRAW_REQUIRED,
    TRICK_DRAW_OPTIONAL,
    TRICK_DRAW_END
};

enum TrickTrackingUpdateType_t
{
    TRICK_TRACK_UPDATE_TRICK = 0,
    TRICK_TRACK_UPDATE_STEP,
};

enum TrickConstraintType_t
{
    CONSTRAINT_SPEED_MAX = 0,
    CONSTRAINT_SPEED_AXIS,
    CONSTRAINT_NO_LANDING,


    // Add more constraints above me
    CONSTRAINT_COUNT,

    CONSTRAINT_FIRST = CONSTRAINT_SPEED_MAX,
    CONSTRAINT_LAST = CONSTRAINT_COUNT - 1,
    CONSTRAINT_INVALID = -1,
};

abstract_class ITrickStepConstraint
{
public:
    virtual bool PlayerPassesConstraint(CMomentumPlayer *pPlayer) = 0;
    virtual void SaveToKeyValues(KeyValues *pKvOut) = 0;
    virtual void LoadFromKeyValues(KeyValues *pKvIn) = 0;
    virtual TrickConstraintType_t GetType() = 0;
    virtual ~ITrickStepConstraint() {}
};

class TrickStepConstraint_MaxSpeed : public ITrickStepConstraint
{
public:
    TrickStepConstraint_MaxSpeed();

    TrickConstraintType_t GetType() override { return CONSTRAINT_SPEED_MAX; }
    bool PlayerPassesConstraint(CMomentumPlayer* pPlayer) override;

    void LoadFromKeyValues(KeyValues* pKvIn) override;
    void SaveToKeyValues(KeyValues* pKvOut) override;

private:
    float m_flMaxSpeed;
};

class CTrickStep
{
public:
    CTrickStep();
    ~CTrickStep();

    bool PlayerPassesConstraints(CMomentumPlayer *pPlayer);
    void AddConstraint(ITrickStepConstraint *pConstraint) { m_vecConstraints.AddToTail(pConstraint); }

    void SetTriggerID(int iTriggerID) { m_iTrickZoneID = iTriggerID; }
    CTriggerTrickZone *GetTrigger();

    void SetOptional(bool bOptional) { m_bOptional = bOptional; }
    bool IsOptional() const { return m_bOptional; }

    void SaveToKV(KeyValues *pKvOut);
    void LoadFromKV(KeyValues *pKvIn);

private:
    bool m_bOptional;
    int m_iTrickZoneID;
    CUtlVector<ITrickStepConstraint*> m_vecConstraints;
};

struct CTrickTag
{
    int m_iID;
    char m_szTagName[128];
};

struct CTrickInfo
{
    int m_iDifficulty;
    char m_szName[128];
    char m_szCreationDate[128];
    char m_szCreatorName[64];
    CUtlVector<CTrickTag*> m_vecTags;

    CTrickInfo();
    ~CTrickInfo();

    void SaveToKV(KeyValues *pKvOut);
    void LoadFromKV(KeyValues *pKvIn);
};

class CTrick
{
public:
    CTrick();

    void SetID(int iID) { m_iID = iID; }
    int GetID() const { return m_iID; }

    void SetName(const char *pName);
    const char *GetName() const { return m_Info.m_szName; }

    int StepCount() const { return m_vecSteps.Count(); }
    CTrickStep* Step(int iStepIndx);

    void AddStep(CTrickStep *pStep) { m_vecSteps.AddToTail(pStep); }

    int GetDifficulty() const { return m_Info.m_iDifficulty; }
    void SetDifficulty(int iDifficulty) { m_Info.m_iDifficulty = iDifficulty; }

    void SaveToKV(KeyValues *pKvOut);
    bool LoadFromKV(KeyValues *pKvIn);

private:
    int m_iID; // Website
    CTrickInfo m_Info;

    CUtlVector<CTrickStep*> m_vecSteps;
};

class CTrickAttempt
{
public:
    CTrickAttempt(CTrick *pTrick);

#ifdef GAME_DLL
    bool ShouldContinue(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer);
    void Complete(CMomentumPlayer *pPlayer);
#endif

    CTrick *GetTrick() const { return m_pTrick; }

    int GetCurrentStep() const { return m_iCurrentStep; }

    // In seconds
    float GetElapsed() const { return gpGlobals->interval_per_tick * static_cast<float>(gpGlobals->tickcount - m_iStartTick); }
private:
    int m_iStartTick; // Timer purposes
    int m_iCurrentStep; // Current trick step that we're on (last trigger that we entered)

    CTrick *m_pTrick;
};

struct CMapTeleport
{
    char m_szName[32];
#ifdef GAME_DLL
    SavedLocation_t *m_pLoc;
#endif

    CMapTeleport();
    void SaveToKV(KeyValues *pKvOut);
    void LoadFromKV(KeyValues *pKvIn);
};

class CTrickSystem : public CAutoGameSystem
{
public:
    CTrickSystem();

    void LevelShutdownPreEntity() override; // Stop recording, clear out tricks memory

    void LoadTrickDataFromFile(KeyValues *pKvTrickData);
    void LoadTrickDataFromSite(KeyValues *pKvTrickData);

    CTriggerTrickZone *GetTrickZone(int id);
    void AddZone(CTriggerTrickZone *pZone);

    int GetTrickCount() const { return m_llTrickList.Count(); }
    CTrick *GetTrick(int index) { return m_llTrickList[index]; }
    CTrick *GetTrickByID(int iID);

    int GetMapTeleCount() const { return m_vecMapTeleports.Count(); }
    CMapTeleport *GetMapTele(int index) { return m_vecMapTeleports[index]; }

#ifdef GAME_DLL
    CTrickAttempt *GetTrickAttemptForTrick(int iTrickID);

    void SetTrackedTrick(int iTrickID);
    int GetTrackedTrick() const { return m_iTrackedTrick; }

    void OnTrickTrackStart();
    void UpdateTrackedTrickTriggers();
    void ClearTrackedTrickTriggers();
    void OnTrickTrackTerminate();
    void SendTrickTrackEvent(TrickTrackingUpdateType_t type, int numeric);
    void UpdateTrackedTrickStep(CTrickAttempt *pAttempt, CMomentumPlayer *pPlayer);

    void TeleportToTrick(int iTrickID);

    void StartRecording();
    void StopRecording(const char *pTrickName);

    void OnTrickZoneEnter(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer);
    void OnTrickZoneExit(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer);

    void CompleteTrick(CTrickAttempt *pAttempt);
    void OnTrickFailed(CTrickAttempt *pAttempt);
    void ClearTrickAttempts();

    void PostPlayerManualTeleport(CMomentumPlayer *pPlayer);

    void SaveTrickDataToFile();

    void CreateMapTeleport(const char *pName);
    void GoToMapTeleport(int iTeleportNum);
#else
    void LevelInitPreEntity() override;
    void OnTrickDataReceived(KeyValues *pData);
#endif

private:
    CUtlVector<CTriggerTrickZone *> m_vecRecordedZones;

#ifdef GAME_DLL
    int m_iTrackedTrick;
    bool m_bRecording;
    CUtlVector<CTrickAttempt*> m_vecCurrentTrickAttempts;
#else
    void InitializeTrickData(KeyValues *pKvTrickData);
#endif

    // Every trick loaded for the map
    CUtlLinkedList<CTrick*> m_llTrickList;
    // Keeping track. ID is their index into the array.
    CUtlVector<CTriggerTrickZone*> m_vecTrickZones;
    CUtlVector<CMapTeleport*> m_vecMapTeleports;
};

extern CTrickSystem *g_pTrickSystem;