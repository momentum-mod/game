#pragma once

#include "cbase.h"
#include "gamemovement.h"
#include "func_ladder.h"
#include "mom_player_shared.h"
#include "baseplayer_shared.h"


struct surface_data_t;

class CMomentumPlayer;

#define NO_REFL_NORMAL_CHANGE -2.0f
#define	STOP_EPSILON		0.1
#define	MAX_CLIP_PLANES		5

#define STAMINA_MAX				100.0
#define STAMINA_COST_JUMP		25.0
#define STAMINA_COST_FALL		20.0
#define STAMINA_RECOVER_RATE	19.0
#define CS_WALK_SPEED			135.0f

#define DuckSpeedMultiplier 0.34f

class CMomentumGameMovement : public CGameMovement
{
    typedef CGameMovement BaseClass;
public:

    CMomentumGameMovement();

    // Overrides
    bool LadderMove(void) override; // REPLACED
    bool OnLadder(trace_t &trace) override; // REPLACED
    void SetGroundEntity(trace_t *pm) override;

    bool CanAccelerate(void) override
    { BaseClass::CanAccelerate(); return true; }//C+P from HL2GM
    bool CheckJumpButton(void) override;
    void PlayerMove(void) override;
    void AirMove(void) override;//Overridden for rampboost fix
    void WalkMove(void) override;
    virtual void CheckForLadders(bool);

    virtual void CategorizeGroundSurface(trace_t&);

    //Override fall damage
    void CheckFalling() override;

    //added ladder
    float LadderDistance(void) const override
    {
        if (player->GetMoveType() == MOVETYPE_LADDER)
            return 10.0f;
        return 2.0f;
    }

    unsigned int LadderMask(void) const override
    {
        return MASK_PLAYERSOLID & (~CONTENTS_PLAYERCLIP);
    }

    float ClimbSpeed(void) const override;
    float LadderLateralMultiplier(void) const override;
    //const float DuckSpeedMultiplier = 0.34f;

    //Overrides for fixing rampboost
    int TryPlayerMove(Vector *pFirstDest = nullptr, trace_t *pFirstTrace = nullptr) override;
    void FullWalkMove() override;
    void DoLateReflect();
    void CategorizePosition() override;

    // Duck
    void Duck(void) override;
    void FinishUnDuck(void) override;
    void FinishDuck(void) override;
    bool CanUnduck() override;
    void HandleDuckingSpeedCrop() override;
    void CheckParameters(void) override;
    void ReduceTimers(void) override;
private:

    float m_flReflectNormal;//Used by rampboost fix

    // Given a list of nearby ladders, find the best ladder and the "mount" origin
    void		Findladder(float maxdist, CFuncLadder **ppLadder, Vector& ladderOrigin, const CFuncLadder *skipLadder);

    // Debounce the +USE key
    void		SwallowUseKey();
    CMomentumPlayer *GetMomentumPlayer() const;
};
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline CMomentumPlayer	*CMomentumGameMovement::GetMomentumPlayer() const
{
    return static_cast<CMomentumPlayer *>(player);
}
