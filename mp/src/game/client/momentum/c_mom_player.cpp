#include "cbase.h"
#include "c_mom_player.h"
#include "c_mom_replay_entity.h"
#include "c_mom_online_ghost.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumPlayer, DT_MOM_Player, CMomentumPlayer)
RecvPropBool(RECVINFO(m_bHasPracticeMode)),
RecvPropBool(RECVINFO(m_bPreventPlayerBhop)),
RecvPropInt(RECVINFO(m_iLandTick)),
RecvPropBool(RECVINFO(m_bResumeZoom)),
RecvPropInt(RECVINFO(m_iShotsFired)),
RecvPropInt(RECVINFO(m_iDirection)),
RecvPropInt(RECVINFO(m_iLastZoom)),
RecvPropInt(RECVINFO(m_afButtonDisabled)),
RecvPropEHandle(RECVINFO(m_CurrentSlideTrigger)),
RecvPropBool(RECVINFO(m_bAutoBhop)),
RecvPropDataTable(RECVINFO_DT(m_Data), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunEntityData)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA(C_MomentumPlayer)
DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

C_MomentumPlayer::C_MomentumPlayer() : m_RunStats(&m_SrvData.m_RunStatsData), m_pViewTarget(nullptr), m_pSpectateTarget(nullptr)
{
    ConVarRef scissor("r_flashlightscissor");
    scissor.SetValue("0");
    m_Data.m_bMapFinished = false;
    m_Data.m_flLastJumpTime = 0.0f;
    m_bHasPracticeMode = false;
    m_afButtonDisabled = 0;
    m_flStartSpeed = 0.0f;
    m_flEndSpeed = 0.0f;
    m_duckUntilOnGround = false;
    m_flStamina = 0.0f;
    m_flGrabbableLadderTime = 0.0f;

    m_bAutoBhop = true;
    m_CurrentSlideTrigger = nullptr;
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

    if (IsObserver())
    {
        C_MomentumOnlineGhostEntity *pOnlineSpec = GetOnlineGhostEnt();
        if (pOnlineSpec)
        {
            // Changed to spectate another ghost
            if (m_pSpectateTarget && m_pSpectateTarget != pOnlineSpec)
                m_pSpectateTarget->m_bSpectated = false;

            m_pSpectateTarget = pOnlineSpec;
            m_pSpectateTarget->m_bSpectated = true;
        }

    }
    else
    {
        if (m_pSpectateTarget)
        {
            m_pSpectateTarget->m_bSpectated = false;
            m_pSpectateTarget = nullptr;
        }
    }
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

int C_MomentumPlayer::GetSpecEntIndex() const
{
    return m_hObserverTarget.GetEntryIndex();
}

C_MomentumReplayGhostEntity* C_MomentumPlayer::GetReplayEnt() const
{
    return dynamic_cast<C_MomentumReplayGhostEntity *>(m_hObserverTarget.Get());
}

C_MomentumOnlineGhostEntity* C_MomentumPlayer::GetOnlineGhostEnt() const
{
    return dynamic_cast<C_MomentumOnlineGhostEntity *>(m_hObserverTarget.Get());
}

// Overridden for Ghost entity
Vector C_MomentumPlayer::GetChaseCamViewOffset(C_BaseEntity* target)
{
    C_MomentumGhostBaseEntity *pGhost = dynamic_cast<C_MomentumGhostBaseEntity*>(target);
    if (pGhost)
    {
        if (pGhost->GetFlags() & FL_DUCKING)
            return VEC_DUCK_VIEW_SCALED(pGhost);

        return VEC_VIEW_SCALED(pGhost);
    }

    // Resort to base class for player code
    return BaseClass::GetChaseCamViewOffset(target);
}