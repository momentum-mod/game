#pragma once

#include "gamemovement.h"

#ifdef CLIENT_DLL
#define CMomentumPlayer C_MomentumPlayer
#endif

class CMomentumPlayer;

class CMomentumGameMovement : public CGameMovement
{
    typedef CGameMovement BaseClass;

  public:
    CMomentumGameMovement();

    void SetGroundEntity(trace_t *pm) override;

    bool CanAccelerate() override;
    bool CheckJumpButton() override;
    void PlayerMove() override;
    void AirMove() override;
    void WalkMove() override;

    // Override fall damage
    void CheckFalling() override;

    void PlayerRoughLandingEffects(float) override;

    // Ladder
    float LadderDistance() const override;
    bool GameHasLadders() const override;
    unsigned int LadderMask() const override { return MASK_PLAYERSOLID & (~CONTENTS_PLAYERCLIP); }
    float LadderLateralMultiplier() const override;
    float ClimbSpeed() const override;
    bool LadderMove() override;

    // Override for fixing punchangle
    void DecayPunchAngle() override;

    int TryPlayerMove(Vector *pFirstDest = nullptr, trace_t *pFirstTrace = nullptr) override;
    void FullWalkMove() override;
    void CategorizePosition() override;

    void ProcessMovement(CBasePlayer *pBasePlayer, CMoveData *pMove) override;

    void Friction() override;

    void CheckWaterJump() override;
    bool CheckWater() override;

    // Duck
    void Duck() override;
    void FinishUnDuck() override;
    void FinishDuck() override;
    bool CanUnduck() override;
    void HandleDuckingSpeedCrop() override;

    void CheckParameters() override;
    void ReduceTimers() override;

    void StartGravity() override;
    void FinishGravity() override;

    int ClipVelocity(Vector in, Vector &normal, Vector &out, float overbounce) override;

    // Momentum-specific
    virtual void StuckGround();
    virtual void LimitStartZoneSpeed();

    // Validate tracerays
    bool IsValidMovementTrace(trace_t &tr);

    // Limited bunnyhopping in rocket jumping
    void PreventBunnyHopping();

  private:
    CMomentumPlayer *m_pPlayer;

    bool m_bCheckForGrabbableLadder;
};

extern CMomentumGameMovement *g_pMomentumGameMovement;
