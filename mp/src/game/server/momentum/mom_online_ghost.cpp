#include "cbase.h"
#include "mom_online_ghost.h"
#include "util/mom_util.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

ConVar mm_ghostTesting("mom_ghost_testing", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, ".");

LINK_ENTITY_TO_CLASS(mom_online_ghost, CMomentumOnlineGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumOnlineGhostEntity, DT_MOM_OnlineGhost)
    SendPropString(SENDINFO(m_pszGhostName)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumOnlineGhostEntity)
END_DATADESC();

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
    m_vecFrames.Purge();
}

void CMomentumOnlineGhostEntity::SetCurrentNetFrame(ghostNetFrame_t newFrame)
{
    
    m_vecFrames.Insert(new ReceivedFrame(gpGlobals->curtime, newFrame));
}

CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity(): m_pCurrentFrame(nullptr), m_pNextFrame(nullptr)
{
}

void CMomentumOnlineGhostEntity::Precache(void)
{
    BaseClass::Precache();
}
void CMomentumOnlineGhostEntity::Spawn()
{
    BaseClass::Spawn();
    SetSolid(SOLID_BBOX);
    SetNextThink(gpGlobals->curtime);
}
void CMomentumOnlineGhostEntity::Think()
{
    BaseClass::Think();
    HandleGhost();
    if (m_pCurrentSpecPlayer)
        HandleGhostFirstPerson();
    //We have to wait some time after getting the new packets to allow the client some "space", i.e a few saved packets, in order to interpolate
    //float finalLerp = gpGlobals->interval_per_tick / (mm_lerpRatio.GetFloat() / mm_updaterate.GetFloat());
    // Emulate every millisecond (smooth interpolation MOM_TODO: Change this to be some convar?)
    SetNextThink(gpGlobals->curtime + 0.001f);
}
void CMomentumOnlineGhostEntity::HandleGhost()
{
    if (!m_vecFrames.IsEmpty() || (m_pCurrentFrame && m_pNextFrame))
    {
        float flCurtime = gpGlobals->curtime - 0.1f;

        // This is most likely going to be our first frame we ever get
        if (!m_pCurrentFrame)
            m_pCurrentFrame = m_vecFrames.RemoveAtHead();

        // When we finally get our second packet, we assign it here
        if (!m_pNextFrame && !m_vecFrames.IsEmpty())
            m_pNextFrame = m_vecFrames.RemoveAtHead();

        // Catch up if we hitched or something
        while (m_pNextFrame && flCurtime > m_pNextFrame->recvTime && !m_vecFrames.IsEmpty())
        {
            delete m_pCurrentFrame; // Get rid of our old one ASAP
            m_pCurrentFrame = m_pNextFrame; 
            m_pNextFrame = m_vecFrames.RemoveAtHead();
        }
        
        if (m_pNextFrame)
        {
            float maxvel = sv_maxvelocity.GetFloat();

            Vector curPos = m_pCurrentFrame->frame.Position;
            Vector nextPos = m_pNextFrame->frame.Position;
            
            // If you've gone a larger distance than what's expected
            if (curPos.DistTo(nextPos) > (maxvel * sqrt(3.0) / mm_updaterate.GetFloat()))
            {
                DevLog("I think the ghost with steamID %lld just teleported!\n", m_GhostSteamID.ConvertToUint64());
                return; // This causes an intentional hitch, making the while loop above handle the teleport
            }

            float percent = flCurtime - m_pCurrentFrame->recvTime / (m_pNextFrame->recvTime - m_pCurrentFrame->recvTime);
            // Interpolate between our frames
            // Position and Velocity
            Vector interPos, interVel;
            VectorLerp(curPos, nextPos, percent, interPos);
            VectorLerp(m_pCurrentFrame->frame.Velocity, m_pNextFrame->frame.Velocity, percent, interVel);
            // View offset (crouching)
            float flOffsetInterp = Lerp(percent, m_pCurrentFrame->frame.ViewOffset, m_pNextFrame->frame.ViewOffset);
            // View angles
            Quaternion currQuat, nextQuat, interpQuat;
            QAngle interpAngle;
            AngleQuaternion(m_pCurrentFrame->frame.EyeAngle, currQuat);
            AngleQuaternion(m_pNextFrame->frame.EyeAngle, nextQuat);
            QuaternionSlerp(currQuat, nextQuat, percent, interpQuat);
            QuaternionAngles(interpQuat, interpAngle);

            // Set our interpolated data
            SetAbsOrigin(interPos);
            // Fixes an issue with teleporting
            if (interVel.x <= maxvel && interVel.y <= maxvel && interVel.z <= maxvel)
            {
                SetAbsVelocity(interVel);
            }
            SetViewOffset(Vector(0, 0, flOffsetInterp));
            SetAbsAngles(interpAngle);

            // UpdateStats(interVel);

            if (flCurtime >= m_pNextFrame->recvTime)
            {
                delete m_pCurrentFrame; // Free the memory?
                m_pCurrentFrame = m_pNextFrame;
                m_pNextFrame = nullptr; // Set nextFrame to null so the loop resets it next time
            }
        }
        else
        {
            SetAbsOrigin(m_pCurrentFrame->frame.Position);
            QAngle newAngles = QAngle(m_pCurrentFrame->frame.EyeAngle.x / 10, m_pCurrentFrame->frame.EyeAngle.y, m_pCurrentFrame->frame.EyeAngle.z);
            SetAbsAngles(newAngles);
            SetViewOffset(Vector(0, 0, m_pCurrentFrame->frame.ViewOffset));
            SetAbsVelocity(m_pCurrentFrame->frame.Velocity);
            if (flCurtime > m_pCurrentFrame->recvTime) // For extrapolating, this packet will be the last info we have until we get more packets
            {
                delete m_pCurrentFrame;
                m_pCurrentFrame = nullptr;
            }
        }
    }
}

void CMomentumOnlineGhostEntity::HandleGhostFirstPerson()
{
    if (m_pCurrentSpecPlayer)
    {
        if (m_pCurrentSpecPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
        {
            //SetAbsAngles(m_pCurrentFrame->frame.EyeAngle);
            // don't render the model when we're in first person mode
            if (GetRenderMode() != kRenderNone)
            {
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
            }
            bool isDucking = (GetFlags() & FL_DUCKING) != 0;
            if (m_pCurrentFrame->frame.Buttons & IN_DUCK)
            {
                if (!isDucking)
                {
                    SetCollisionBounds(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
                    AddFlag(FL_DUCKING);
                }
            }
            else
            {
                if (CanUnduck(this) && isDucking)
                {
                    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
                    RemoveFlag(FL_DUCKING);
                }
            }
        }
        else
        {
            //SetAbsAngles(GetAbsAngles());

            // remove the nodraw effects
            if (GetRenderMode() != kRenderTransColor)
            {
                SetRenderMode(kRenderTransColor);
                RemoveEffects(EF_NOSHADOW);
            }
        }
    }
}
void CMomentumOnlineGhostEntity::UpdateStats(const Vector &vel)
{
    /*
    auto currentStep = GetCurrentNetFrame();
    float SyncVelocity = vel.Length2DSqr(); // we always want HVEL for checking velocity sync

    if (GetGroundEntity() == nullptr) // The ghost is in the air
    {
        m_bHasJumped = false;

        if (EyeAngles().y > m_angLastEyeAngle.y) // player turned left
        {
            m_nStrafeTicks++;
            if ((currentStep->PlayerButtons() & IN_MOVELEFT) && !(currentStep->PlayerButtons() & IN_MOVERIGHT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
        else if (EyeAngles().y < m_angLastEyeAngle.y) // player turned right
        {
            m_nStrafeTicks++;
            if ((currentStep->PlayerButtons() & IN_MOVERIGHT) && !(currentStep->PlayerButtons() & IN_MOVELEFT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
    }
    if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
    {
        m_SrvData.m_RunData.m_flStrafeSync =
            (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
        m_SrvData.m_RunData.m_flStrafeSync2 =
            (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
    }
    */
}
