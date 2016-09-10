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

#define STAMINA_MAX				100.0f
#define STAMINA_COST_JUMP		25.0f
#define STAMINA_COST_FALL		20.0f
#define STAMINA_RECOVER_RATE	19.0f
#define CS_WALK_SPEED			135.0f

#define DUCK_SPEED_MULTIPLIER 0.34f

class CMomentumGameMovement : public CGameMovement
{
    typedef CGameMovement BaseClass;
public:

    CMomentumGameMovement();

    // Overrides
	virtual bool LadderMove(void) ; // REPLACED
	virtual bool OnLadder(trace_t &trace) ; // REPLACED
	virtual void SetGroundEntity(trace_t *pm) ;

	virtual bool CanAccelerate(void)
    { BaseClass::CanAccelerate(); return true; }//C+P from HL2GM
	virtual bool CheckJumpButton(void);
	virtual void PlayerMove(void);
	virtual void AirMove(void);//Overridden for rampboost fix
	virtual void WalkMove(void);
    virtual void CheckForLadders(bool);

    virtual void CategorizeGroundSurface(trace_t&);

    //Override fall damage
	virtual void CheckFalling();

    //added ladder
	virtual float LadderDistance(void) const
    {
        if (player->GetMoveType() == MOVETYPE_LADDER)
            return 10.0f;
        return 2.0f;
    }

	virtual unsigned int LadderMask(void) const 
    {
        return MASK_PLAYERSOLID & (~CONTENTS_PLAYERCLIP);
    }


	virtual float ClimbSpeed(void) const ;
	virtual float LadderLateralMultiplier(void) const ;
    //const float DuckSpeedMultiplier = 0.34f;

    //Overrides for fixing rampboost
	virtual int TryPlayerMove(Vector *pFirstDest = nullptr, trace_t *pFirstTrace = nullptr);
	virtual void FullWalkMove();
	virtual void DoLateReflect();
	virtual void CategorizePosition();

    void ProcessMovement(CBasePlayer *pBasePlayer, CMoveData *pMove) override
    {
        m_pPlayer = ToCMOMPlayer(pBasePlayer);
        Assert(m_pPlayer);

        BaseClass::ProcessMovement(pBasePlayer, pMove);
    }

    // Duck
	virtual void Duck(void) ;
	virtual void FinishUnDuck(void) ;
	virtual void FinishDuck(void) ;
	virtual bool CanUnduck() ;
	virtual void HandleDuckingSpeedCrop() ;
	virtual void CheckParameters(void) ;
	virtual void ReduceTimers(void);
private:

    float m_flReflectNormal = NO_REFL_NORMAL_CHANGE;//Used by rampboost fix
    CMomentumPlayer *m_pPlayer;
};
