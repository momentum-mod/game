//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"
#include "engine/ivdebugoverlay.h"

#if defined( HL2_DLL )
#include "c_basehlplayer.h"
#endif

#include "death_pose.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PING_MAX_TIME	2.0

IMPLEMENT_CLIENTCLASS_DT(C_AI_BaseNPC, DT_AI_BaseNPC, CAI_BaseNPC)
RecvPropInt(RECVINFO(m_lifeState)),
RecvPropBool(RECVINFO(m_bPerformAvoidance)),
RecvPropBool(RECVINFO(m_bIsMoving)),
RecvPropBool(RECVINFO(m_bFadeCorpse)),
RecvPropInt(RECVINFO(m_iDeathPose)),
RecvPropInt(RECVINFO(m_iDeathFrame)),
RecvPropInt(RECVINFO(m_iSpeedModRadius)),
RecvPropInt(RECVINFO(m_iSpeedModSpeed)),
RecvPropInt(RECVINFO(m_bSpeedModActive)),
RecvPropBool(RECVINFO(m_bImportanRagdoll)),
RecvPropFloat(RECVINFO(m_flTimePingEffect)),
END_RECV_TABLE()

extern ConVar cl_npc_speedmod_intime;

bool NPC_IsImportantNPC(C_BaseAnimating *pAnimating)
{
    C_AI_BaseNPC *pBaseNPC = dynamic_cast <C_AI_BaseNPC*> (pAnimating);

    if (pBaseNPC == NULL)
        return false;

    return pBaseNPC->ImportantRagdoll();
}

C_AI_BaseNPC::C_AI_BaseNPC()
{
}

//-----------------------------------------------------------------------------
// Makes ragdolls ignore npcclip brushes
//-----------------------------------------------------------------------------
unsigned int C_AI_BaseNPC::PhysicsSolidMaskForEntity(void) const
{
    // This allows ragdolls to move through npcclip brushes
    if (!IsRagdoll())
    {
        return MASK_NPCSOLID;
    }
    return MASK_SOLID;
}


void C_AI_BaseNPC::ClientThink(void)
{
    BaseClass::ClientThink();

#ifdef HL2_DLL
    C_BaseHLPlayer *pPlayer = dynamic_cast<C_BaseHLPlayer*>( C_BasePlayer::GetLocalPlayer() );

    if ( ShouldModifyPlayerSpeed() == true )
    {
        if ( pPlayer )
        {
            float flDist = (GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr();

            if ( flDist <= GetSpeedModifyRadius() )
            {
                if ( pPlayer->m_hClosestNPC )
                {
                    if ( pPlayer->m_hClosestNPC != this )
                    {
                        float flDistOther = (pPlayer->m_hClosestNPC->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

                        //If I'm closer than the other NPC then replace it with myself.
                        if ( flDist < flDistOther )
                        {
                            pPlayer->m_hClosestNPC = this;
                            pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
                        }
                    }
                }
                else
                {
                    pPlayer->m_hClosestNPC = this;
                    pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
                }
            }
        }
    }
#endif // HL2_DLL
}

void C_AI_BaseNPC::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    if ((ShouldModifyPlayerSpeed() == true) || (m_flTimePingEffect > gpGlobals->curtime))
    {
        SetNextClientThink(CLIENT_THINK_ALWAYS);
    }
}

void C_AI_BaseNPC::GetRagdollInitBoneArrays(matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt)
{
    ForceSetupBonesAtTime(pDeltaBones0, gpGlobals->curtime - boneDt);
    GetRagdollCurSequenceWithDeathPose(this, pDeltaBones1, gpGlobals->curtime, m_iDeathPose, m_iDeathFrame);
    float ragdollCreateTime = PhysGetSyncCreateTime();
    if (ragdollCreateTime != gpGlobals->curtime)
    {
        // The next simulation frame begins before the end of this frame
        // so initialize the ragdoll at that time so that it will reach the current
        // position at curtime.  Otherwise the ragdoll will simulate forward from curtime
        // and pop into the future a bit at this point of transition
        ForceSetupBonesAtTime(pCurrentBones, ragdollCreateTime);
    }
    else
    {
        SetupBones(pCurrentBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime);
    }
}

