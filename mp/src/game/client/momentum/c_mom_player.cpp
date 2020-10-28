#include "cbase.h"
#include "c_mom_player.h"

#include <steam/isteamuser.h>

#include "c_mom_online_ghost.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumPlayer, DT_MOM_Player, CMomentumPlayer)
RecvPropBool(RECVINFO(m_bIsSprinting)),
RecvPropBool(RECVINFO(m_bIsWalking)),
RecvPropBool(RECVINFO(m_bHasPracticeMode)),
RecvPropBool(RECVINFO(m_bPreventPlayerBhop)),
RecvPropInt(RECVINFO(m_iJumpTick)),
RecvPropInt(RECVINFO(m_iLandTick)),
RecvPropBool(RECVINFO(m_bResumeZoom)),
RecvPropInt(RECVINFO(m_iShotsFired), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iDirection), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iLastZoomFOV), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_afButtonDisabled)),
RecvPropEHandle(RECVINFO(m_CurrentSlideTrigger)),
RecvPropBool(RECVINFO(m_bAutoBhop)),
RecvPropFloat(RECVINFO(m_fDuckTimer)),
RecvPropBool(RECVINFO(m_bSurfing)),
RecvPropInt(RECVINFO(m_nButtonsToggled)),
RecvPropVector(RECVINFO(m_vecRampBoardVel)),
RecvPropVector(RECVINFO(m_vecRampLeaveVel)),
RecvPropArray3(RECVINFO_ARRAY(m_iZoneCount), RecvPropInt(RECVINFO(m_iZoneCount[0]), SPROP_UNSIGNED)),
RecvPropArray3(RECVINFO_ARRAY(m_iLinearTracks), RecvPropInt(RECVINFO(m_iLinearTracks[0]), SPROP_UNSIGNED)),
RecvPropDataTable(RECVINFO_DT(m_Data), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunEntityData)),
RecvPropDataTable(RECVINFO_DT(m_RunStats), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunStats)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA(C_MomentumPlayer)
DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iDirection, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_fDuckTimer, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bSurfing, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vecRampBoardVel, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vecRampLeaveVel, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

static C_MomentumPlayer *s_pLocalPlayer = nullptr;

C_MomentumPlayer::C_MomentumPlayer(): m_pSpecTarget(nullptr)
{
    ConVarRef scissor("r_flashlightscissor");
    scissor.SetValue("0");
    m_bSurfing = false;
    m_vecRampBoardVel.Init();
    m_vecRampLeaveVel.Init();
    m_bHasPracticeMode = false;
    m_afButtonDisabled = 0;
    m_flStartSpeed = 0.0f;
    m_flEndSpeed = 0.0f;
    m_flStamina = 0.0f;
    m_flGrabbableLadderTime = 0.0f;

    m_iLandTick = 0;
    m_iJumpTick = 0;
    m_bIsSprinting = false;
    m_bIsWalking = false;
    m_bAutoBhop = true;
    m_CurrentSlideTrigger = nullptr;
    m_RunStats.Init();
    m_fDuckTimer = 0.0f;

    m_bIsPowerSliding = false;
    m_nWallRunState = WALLRUN_NOT;
    m_bWasSprinting = false;

    m_nButtonsToggled = 0;
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
    if (!m_hObserverTarget.Get())
        m_pSpecTarget = nullptr;
    else if (!m_pSpecTarget)
        m_pSpecTarget = dynamic_cast<CMomRunEntity*>(m_hObserverTarget.Get());

    return m_pSpecTarget ? m_pSpecTarget : this;
}

CMomRunEntityData *C_MomentumPlayer::GetCurrentUIEntData()
{
    return GetCurrentUIEntity()->GetRunEntData();
}

CMomRunStats *C_MomentumPlayer::GetCurrentUIEntStats()
{
    return GetCurrentUIEntity()->GetRunStats();
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

void C_MomentumPlayer::OnDataChanged(DataUpdateType_t type)
{
    // clear the sprint toggle if we just stopped not sprinting
    /*if (m_bWasSprinting && !m_bIsSprinting)
    {
        IN_ClearSpeedToggle();
    }

    m_bWasSprinting = m_bIsSprinting;*/


    BaseClass::OnDataChanged(type);

    UpdateVisibility();
}


void C_MomentumPlayer::PostDataUpdate(DataUpdateType_t updateType)
{
    if (updateType == DATA_UPDATE_CREATED)
    {
        if (engine->GetLocalPlayer() == m_index)
        {
            Assert(s_pLocalPlayer == nullptr);
            s_pLocalPlayer = this;
        }
    }

    // C_BaseEntity assumes we're networking the entity's angles, so pretend that it
    // networked the same value we already have.
    SetNetworkAngles(GetLocalAngles());

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
    if (m_Data.m_iTimerState != TIMER_STATE_NOT_RUNNING)
    {
        iTotalTicks = gpGlobals->tickcount - m_Data.m_iStartTick;
    }
    else if (m_Data.m_bMapFinished)
    {
        iTotalTicks = m_Data.m_iRunTime;
    }

    return float(iTotalTicks) * m_Data.m_flTickRate;
}

uint64 C_MomentumPlayer::GetSteamID()
{
    return SteamUser() ? SteamUser()->GetSteamID().ConvertToUint64() : 0;
}