#include "cbase.h"
#include "c_mom_player.h"

#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_MomentumPlayer, DT_MOM_Player, CMomentumPlayer)
RecvPropInt(RECVINFO(m_afButtonDisabled)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA(C_MomentumPlayer)
DEFINE_PRED_FIELD(m_SrvData.m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_SrvData.m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

C_MomentumPlayer::C_MomentumPlayer() : m_RunStats(&m_SrvData.m_RunStatsData)
{
    ConVarRef scissor("r_flashlightscissor");
    scissor.SetValue("0");
    m_SrvData.m_RunData.m_bMapFinished = false;
    m_SrvData.m_RunData.m_flLastJumpTime = 0.0f;
    m_SrvData.m_bHasPracticeMode = false;
    m_afButtonDisabled = 0;
    m_fSliding = 0;
    m_flStartSpeed = 0.0f;
    m_flEndSpeed = 0.0f;
    m_duckUntilOnGround = false;
    m_flStamina = 0.0f;
}

C_MomentumPlayer::~C_MomentumPlayer()
{

}
void C_MomentumPlayer::Spawn()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
}
//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_MomentumPlayer::CreateMove(float flInputSampleTime, CUserCmd *pCmd)
{
	// Bleh... we will wind up needing to access bones for attachments in here.
	C_BaseAnimating::AutoAllowBoneAccess boneaccess(true, true);

	return BaseClass::CreateMove(flInputSampleTime, pCmd);
}


void C_MomentumPlayer::ClientThink()
{
	SetNextClientThink(CLIENT_THINK_ALWAYS);
    FetchStdData(this);
}

void C_MomentumPlayer::OnDataChanged(DataUpdateType_t type)
{
	//SetNextClientThink(CLIENT_THINK_ALWAYS);

	BaseClass::OnDataChanged(type);

	UpdateVisibility();
}


void C_MomentumPlayer::PostDataUpdate(DataUpdateType_t updateType)
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles(GetLocalAngles());

	//SetNextClientThink(CLIENT_THINK_ALWAYS);

	BaseClass::PostDataUpdate(updateType);
}


void C_MomentumPlayer::SurpressLadderChecks(const Vector& pos, const Vector& normal)
{
    m_ladderSurpressionTimer.Start(1.0f);
    m_lastLadderPos = pos;
    m_lastLadderNormal = normal;
}

bool C_MomentumPlayer::CanGrabLadder(const Vector& pos, const Vector& normal)
{
    if (m_ladderSurpressionTimer.GetRemainingTime() <= 0.0f)
    {
        return true;
    }

    const float MaxDist = 64.0f;
    if (pos.AsVector2D().DistToSqr(m_lastLadderPos.AsVector2D()) < MaxDist * MaxDist)
    {
        return false;
    }

    if (normal != m_lastLadderNormal)
    {
        return true;
    }

    return false;
}

// Overridden for Ghost entity
Vector C_MomentumPlayer::GetChaseCamViewOffset(C_BaseEntity* target)
{
    C_MomentumReplayGhostEntity *pGhost = dynamic_cast<C_MomentumReplayGhostEntity*>(target);

    if (pGhost)
    {
        if (pGhost->GetFlags() & FL_DUCKING)
        {
            return VEC_DUCK_VIEW_SCALED(pGhost);
        }

        return VEC_VIEW_SCALED(pGhost);
    }

    // Resort to base class for player code
    return BaseClass::GetChaseCamViewOffset(target);
}
