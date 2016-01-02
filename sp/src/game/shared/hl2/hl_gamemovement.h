//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Special handling for hl2 usable ladders
//
//=============================================================================//

#include "gamemovement.h"
#include "func_ladder.h"

#if defined( CLIENT_DLL )

#include "c_basehlplayer.h"
#define CHL2_Player C_BaseHLPlayer
#else

#include "hl2_player.h"

#endif

struct LadderMove_t;
class CInfoLadderDismount;

struct NearbyDismount_t
{
	CInfoLadderDismount		*dismount;
	float					distSqr;
};

//-----------------------------------------------------------------------------
// Purpose: HL2 specific movement code
//-----------------------------------------------------------------------------
class CHL2GameMovement : public CGameMovement
{
	typedef CGameMovement BaseClass;
public:

	CHL2GameMovement();

// Overrides
	virtual bool LadderMove( void ); // REPLACED
	virtual bool OnLadder( trace_t &trace ); // REPLACED
	virtual void	SetGroundEntity( trace_t *pm );
	virtual bool CanAccelerate( void );
	virtual bool CheckJumpButton(void);
	virtual void PlayerMove(void);
	virtual float AirMove(void);
	virtual void WalkMove(void);

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

	virtual float ClimbSpeed(void) const;
	virtual float LadderLateralMultiplier(void) const;
	const float DuckSpeedMultiplier = 0.34f;

	// Duck
	virtual void Duck(void);
	virtual void FinishUnDuck(void);
	virtual void FinishDuck(void);
	virtual bool CanUnduck();
	virtual void HandleDuckingSpeedCrop();
	virtual void CheckParameters(void);

private:


	//added for ladders
	void CheckForLadders(bool wasOnGround);

	// Given a list of nearby ladders, find the best ladder and the "mount" origin
	void		Findladder( float maxdist, CFuncLadder **ppLadder, Vector& ladderOrigin, const CFuncLadder *skipLadder );

	// Debounce the +USE key
	void		SwallowUseKey();

	CHL2_Player	*GetHL2Player();
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline CHL2_Player	*CHL2GameMovement::GetHL2Player()
{
	return static_cast< CHL2_Player * >( player );
}
