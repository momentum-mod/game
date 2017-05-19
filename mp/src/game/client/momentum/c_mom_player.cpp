#include "cbase.h"
#include "c_mom_player.h"
#include "view.h"
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_MomentumPlayer, DT_MOM_Player, CMomentumPlayer)
RecvPropInt(RECVINFO(m_afButtonDisabled)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA(C_MomentumPlayer)
DEFINE_PRED_FIELD(m_SrvData.m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_SrvData.m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

C_MomentumPlayer::C_MomentumPlayer()
{
    ConVarRef scissor("r_flashlightscissor");
    scissor.SetValue("0");
    m_SrvData.m_RunData.m_bMapFinished = false;
    m_SrvData.m_RunData.m_flLastJumpTime = 0.0f;
    m_SrvData.m_bHasPracticeMode = false;
    m_afButtonDisabled = 0;
    m_RunStats.m_pData = &m_SrvData.m_RunStatsData;
    m_RunStats.Init();
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
    UpdateIDTarget();
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
int C_MomentumPlayer::GetIDTarget() const
{
    return m_iIDEntIndex;
}
void C_MomentumPlayer::UpdateIDTarget()
{
    if (!IsLocalPlayer())
        return;

    // Clear old target and find a new one
    m_iIDEntIndex = 0;

    trace_t tr;
    Vector vecStart, vecEnd;
    VectorMA(MainViewOrigin(), MAX_TRACE_LENGTH, MainViewForward(), vecEnd);
    VectorMA(MainViewOrigin(), 10, MainViewForward(), vecStart);

    // If we're in observer mode, ignore our observer target. Otherwise, ignore ourselves.
    if (IsObserver())
    {
        UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, GetObserverTarget(), COLLISION_GROUP_NONE, &tr);
    }
    else
    {
        UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
    }
    if (!tr.startsolid && tr.DidHitNonWorldEntity())
    {
        C_BaseEntity *pEntity = tr.m_pEnt;

        if (pEntity && (pEntity != this))
        {
            m_iIDEntIndex = pEntity->entindex();
            DisplayHintsForTarget(pEntity);
        }
    }
}
void C_MomentumPlayer::DisplayHintsForTarget(C_BaseEntity *pTarget)
{
    // If the entity provides hints, ask them if they have one for this target
    ITargetIDProvidesHint *pHintInterface = dynamic_cast<ITargetIDProvidesHint*>(pTarget);
    if (pHintInterface)
    {
        pHintInterface->DisplayHintTo(this);
    }

}