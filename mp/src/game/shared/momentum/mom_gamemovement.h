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

    void SetGroundEntity(const trace_t *pm) override;

    bool CanAccelerate() override;
    bool CheckJumpButton() override;
    void PlayerMove() override;
    void AirMove() override;
    void WalkMove() override;

    int ClipVelocity(Vector in, Vector &normal, Vector &out, float overbounce) override;

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
    void StepMove(Vector &vecDestination, trace_t &trace) override;
    void CategorizePosition() override;

    void ProcessMovement(CBasePlayer *pBasePlayer, CMoveData *pMove) override;

    void Friction() override;

    float GetWaterWaistOffset() override;
    float GetWaterJumpUpZVelocity() override;
    float GetWaterJumpForward() override;
    void CalculateWaterWishVelocityZ(Vector &wishVel, const Vector &forward) override;

    // Duck
    void Duck() override;
    void DoDuck(int iButtonsPressed);
    void FinishDuck() override;
    bool CanUnduck() override;
    void DoUnduck(int iButtonsReleased);
    void FinishUnDuck() override;
    void HandleDuckingSpeedCrop() override;
    float GetTimeToDuck() override;
    float GetDuckTimer() override;

    void CheckParameters() override;
    void ReduceTimers() override;

    void StartGravity() override;
    float GetPlayerGravity() override;
    void FinishGravity() override;

    // Momentum-specific
    virtual void StuckGround();
    virtual void LimitStartZoneSpeed();

    // Validate tracerays
    bool IsValidMovementTrace(trace_t &tr);

    // Limited bunnyhopping in rocket jumping
    void PreventBunnyHopping();

    void CheckWaterJump() override;
    void WaterJump() override;
    void CheckVelocity() override;
    bool ShouldApplyGroundFriction() override;

    // ========== Parkour-only methods

    // Check if only touching wall with head/upper body
    void            CheckFeetCanReachWall();

    // Special friction for powersliding
    void            PowerSlideFriction();

    // Check if player should powerslide
    // Called when we duck or land on the ground while ducked
    virtual void    CheckPowerSlide();

    // End powerslide - reset the vars, stop the sound
    virtual void    EndPowerSlide();

    virtual void    AnticipateWallRun();

    virtual bool    CheckForSteps(const Vector &startpos, const Vector &vel);

    // Get the yaw angle between the player and the wall normal
    virtual float   GetWallRunYaw();

    // Check if player should start wallrunning,
    // i.e. hit a suitable wall while airborn.
    virtual void    CheckWallRun(Vector &vecWallNormal, trace_t &pm);

    // Check if player can scramble up on top of obstacle
    virtual void    CheckWallRunScramble(bool &steps);

    // Calculate the wallrun view roll angle based on the 
    // yaw angle between the player and the wall
    virtual float   GetWallRunRollAngle();

    // Handle wallrun movement
    virtual void    WallRunMove();

    // Handle step-like bits of the wall when wallrunning
    // (basically step move except wallnorm instead of up)
    virtual void	WallRunAnticipateBump();

    // Try not to get stuck moving in to a corner that is small
    // and we could easily step around it.
    virtual void    WallRunEscapeCorner(Vector &wishdir);
    virtual bool    TryEscape(Vector &posD, float rotation, Vector move);

    // Handle end of wallrun - set vars, stop sound
    virtual void    EndWallRun();

    // Parkour's version of WaterJump
    void WaterJumpParkour();

    void PerformLurchChecks();

    // Defrag movement functions
    void DFPlayerMove();
    void DFFullWalkMove();
    void DFDuck();
    void DFWalkMove();
    void DFFriction();
    bool DFCheckJumpButton();
    void DFAirMove();
    void DFAirAccelerate(Vector wishdir, float wishspeed, float accel, float maxspeed);
    void DFGroundTrace();
    void DFClipVelocity(Vector in, Vector &normal, Vector &out, float overbounce);
    bool DFSlideMove(bool inAir);
    void DFStepSlideMove(bool inAir);
    void DFSetGroundEntity(const trace_t *pm);
    void DFAirControl(Vector &wishdir, float wishspeed);

private:
    CMomentumPlayer *m_pPlayer;

    bool m_bCheckForGrabbableLadder;
};

extern CMomentumGameMovement *g_pMomentumGameMovement;