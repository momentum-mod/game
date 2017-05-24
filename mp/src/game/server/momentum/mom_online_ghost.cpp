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
}
CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity()
{
    hasSpawned = false;
}
void CMomentumOnlineGhostEntity::Precache(void)
{
    BaseClass::Precache();
}
void CMomentumOnlineGhostEntity::Spawn()
{
    BaseClass::Spawn();
    hasSpawned = true;
    SetSolid(SOLID_BBOX);
    SetNextThink(gpGlobals->curtime);
}
void CMomentumOnlineGhostEntity::Think()
{
    BaseClass::Think();
    HandleGhost();
    //We have to wait some time after getting the new packets to allow the client some "space", i.e a few saved packets, in order to interpolate
    float finalLerp = gpGlobals->interval_per_tick / (mm_lerpRatio.GetFloat() / mm_updaterate.GetFloat());
    SetNextThink(gpGlobals->curtime + finalLerp);
}
void CMomentumOnlineGhostEntity::HandleGhost()
{
    if (hasSpawned)
    {
        if (mm_ghostTesting.GetBool())
        {
            SetAbsOrigin(Vector(m_currentFrame.Position.x + 50, m_currentFrame.Position.y + 50, m_currentFrame.Position.z));
        }
        else
        {
            SetAbsOrigin(m_currentFrame.Position);
        }
        QAngle newAngles = QAngle(m_currentFrame.EyeAngle.x / 10, m_currentFrame.EyeAngle.y, m_currentFrame.EyeAngle.z);
        SetAbsAngles(newAngles);
        SetViewOffset(m_currentFrame.ViewOffset);
        //SetAbsVelocity(GetSmoothedVelocity());
        //MOM_TODO: Fix this
        
        const Vector &ghostPrevOrigin = m_previousFrame.Position;
        const Vector &ghostCurrentOrigin = m_currentFrame.Position;
        const float distX = fabs(ghostPrevOrigin.x - ghostCurrentOrigin.x);
        const float distY = fabs(ghostPrevOrigin.y - ghostCurrentOrigin.y);
        const float distZ = fabs(ghostPrevOrigin.z - ghostCurrentOrigin.z);

        float finalLerp = gpGlobals->interval_per_tick / (mm_lerpRatio.GetFloat() / mm_updaterate.GetFloat());
        const Vector interpolatedVel = Vector(distX, distY, distZ) / finalLerp;

        // Fixes an issue with teleporting
        int maxvel = sv_maxvelocity.GetInt();
        if (interpolatedVel.x <= maxvel && interpolatedVel.y <= maxvel && interpolatedVel.z <= maxvel)
        {
            SetAbsVelocity(interpolatedVel);
        }
        UpdateStats(interpolatedVel);

        Q_strncpy(m_pszGhostName.GetForModify(), m_currentFrame.PlayerName, sizeof(m_pszGhostName));
    }
    m_previousFrame = m_currentFrame;
}
void CMomentumOnlineGhostEntity::HandleGhostFirstPerson()
{
    if (m_pCurrentSpecPlayer)
    {
        if (m_pCurrentSpecPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
        {
            SetAbsAngles(m_currentFrame.EyeAngle);
            // don't render the model when we're in first person mode
            if (GetRenderMode() != kRenderNone)
            {
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
            }
            bool isDucking = (GetFlags() & FL_DUCKING) != 0;
            if (m_currentFrame.Buttons & IN_DUCK)
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
            SetAbsAngles(this->GetAbsAngles());

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
