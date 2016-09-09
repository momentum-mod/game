#include "cbase.h"
#include "prediction.h"
#include "igamemovement.h"
#include "c_mom_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CMoveData g_MoveData;
CMoveData *g_pMoveData = &g_MoveData;


class CMOMPrediction : public CPrediction
{
	DECLARE_CLASS(CMOMPrediction, CPrediction);

public:
	void SetupMove(C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move) override
	{
		player->AvoidPhysicsProps(ucmd);

		BaseClass::SetupMove(player, ucmd, pHelper, move);
	}
};

// Expose interface to engine
static CMOMPrediction g_Prediction;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CMOMPrediction, IPrediction, VCLIENT_PREDICTION_INTERFACE_VERSION, g_Prediction);

CPrediction *prediction = &g_Prediction;