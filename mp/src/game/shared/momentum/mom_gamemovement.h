#pragma once

#include "gamemovement.h"

#ifdef CLIENT_DLL
#define CMomentumPlayer C_MomentumPlayer
#endif

class CMomentumPlayer;

#define NO_REFL_NORMAL_CHANGE -2.0f
#define BHOP_DELAY_TIME 15 // Time to delay successive bhops by, in ticks
#define STOP_EPSILON 0.1
#define MAX_CLIP_PLANES 5

#define STAMINA_MAX 100.0f
#define STAMINA_COST_JUMP 25.0f
#define STAMINA_COST_FALL 20.0f
#define STAMINA_RECOVER_RATE 19.0f
#define CS_WALK_SPEED 135.0f

#define DUCK_SPEED_MULTIPLIER 0.34f

#define GROUND_FACTOR_MULTIPLIER 301.99337741082998788946739227784f

class CMomentumGameMovement : public CGameMovement
{
    typedef CGameMovement BaseClass;

  public:
    CMomentumGameMovement();

    // Overrides
    virtual bool LadderMove(void); // REPLACED
    virtual void SetGroundEntity(trace_t *pm);

    virtual bool CanAccelerate(void)
    {
        BaseClass::CanAccelerate();
        return true;
    } // C+P from HL2GM
    virtual bool CheckJumpButton(void);
    virtual void PlayerMove(void);
    virtual void AirMove(void); // Overridden for rampboost fix
    virtual void WalkMove(void);

    // Override fall damage
    virtual void CheckFalling();

    virtual void PlayerRoughLandingEffects(float) OVERRIDE;

    // added ladder
    virtual float LadderDistance(void) const
    {
        if (player->GetMoveType() == MOVETYPE_LADDER)
            return 10.0f;
        return 2.0f;
    }

    virtual unsigned int LadderMask(void) const { return MASK_PLAYERSOLID & (~CONTENTS_PLAYERCLIP); }

    virtual float ClimbSpeed(void) const;
    virtual float LadderLateralMultiplier(void) const;

    // Validate tracerays
    bool IsValidMovementTrace(trace_t &tr);

    // Override for fixing punchangle
    virtual void DecayPunchAngle(void) OVERRIDE;

    // Overrides for fixing rampboost
    virtual int TryPlayerMove(Vector *pFirstDest = nullptr, trace_t *pFirstTrace = nullptr);
    virtual void FullWalkMove();
    virtual void CategorizePosition();

    void ProcessMovement(CBasePlayer *pBasePlayer, CMoveData *pMove) OVERRIDE;

    void Friction(void);

    // Duck
    virtual void Duck(void);
    virtual void FinishUnDuck(void);
    virtual void FinishDuck(void);
    virtual bool CanUnduck();
    virtual void HandleDuckingSpeedCrop();
    virtual void CheckParameters(void);
    virtual void ReduceTimers(void);
    virtual void StartGravity(void) OVERRIDE;
    virtual void FinishGravity(void) OVERRIDE;
    virtual void StuckGround(void);
    virtual void LimitStartZoneSpeed(void);
    virtual int ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce);

  private:
    CMomentumPlayer *m_pPlayer;

    bool m_bCheckForGrabbableLadder;
};

extern CMomentumGameMovement *g_pMomentumGameMovement;
