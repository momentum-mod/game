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
RecvPropInt(RECVINFO(m_iShotsFired), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iDirection), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iLastZoomFOV), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_afButtonDisabled)),
RecvPropEHandle(RECVINFO(m_CurrentSlideTrigger)),
RecvPropBool(RECVINFO(m_bAutoBhop)),
RecvPropDataTable(RECVINFO_DT(m_Data), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunEntityData)),
RecvPropDataTable(RECVINFO_DT(m_RunStats), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunStats)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA(C_MomentumPlayer)
DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

static C_MomentumPlayer *s_pLocalPlayer;

C_MomentumPlayer::C_MomentumPlayer(): m_pSpecTarget(nullptr)
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
    m_RunStats.Init();
}

C_MomentumPlayer::~C_MomentumPlayer()
{
    if (this == s_pLocalPlayer)
        s_pLocalPlayer = nullptr;
}

C_MomentumPlayer *C_MomentumPlayer::GetLocalMomPlayer()
{
    return s_pLocalPlayer;
}

CMomRunEntity *C_MomentumPlayer::GetCurrentUIEntity()
{
    return m_pSpecTarget ? m_pSpecTarget : this;
}

CMomRunEntityData *C_MomentumPlayer::GetCurrentUIEntData()
{
    return m_pSpecTarget ? m_pSpecTarget->GetRunEntData() : GetRunEntData();
}

CMomRunStats *C_MomentumPlayer::GetCurrentUIEntStats()
{
    return m_pSpecTarget ? m_pSpecTarget->GetRunStats() : GetRunStats();
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

    if (m_hObserverTarget.IsValid() && !m_pSpecTarget)
    {
        m_pSpecTarget = dynamic_cast<CMomRunEntity*>(m_hObserverTarget.Get());
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
    if (updateType == DATA_UPDATE_CREATED)
    {
        if (engine->GetLocalPlayer() == index)
        {
            Assert(s_pLocalPlayer == NULL);
            s_pLocalPlayer = this;
        }
    }

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

void C_MomentumPlayer::OnObserverTargetUpdated()
{
    m_pSpecTarget = nullptr; // Hard-set to null upon observer change

    BaseClass::OnObserverTargetUpdated();
}

float C_MomentumPlayer::GetCurrentRunTime()
{
    int iTotalTicks = 0;
    if (m_Data.m_bTimerRunning)
        iTotalTicks = gpGlobals->tickcount - m_Data.m_iStartTick;    
    else if (m_Data.m_bMapFinished)
        iTotalTicks = m_Data.m_iRunTimeTicks;

    return float(iTotalTicks) * m_Data.m_flTickRate;
}
