#include "cbase.h"
#include "mom_online_ghost.h"
#include "util/mom_util.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(mom_online_ghost, CMomentumOnlineGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumOnlineGhostEntity, DT_MOM_OnlineGhost)
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumOnlineGhostEntity)
END_DATADESC();

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
}
CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity()
{
    SetGhostBodyGroup(BODY_PROLATE_ELLIPSE);
    SetGhostColor(COLOR_RED);
    SetGhostModel(GHOST_MODEL);
    hasSpawned = false;
}
void CMomentumOnlineGhostEntity::Precache(void)
{
    BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
}
void CMomentumOnlineGhostEntity::Spawn()
{
    Precache();
    BaseClass::Spawn();
    m_debugOverlays |= (OVERLAY_BBOX_BIT | OVERLAY_TEXT_BIT);

    hasSpawned = true;
    ConDColorMsg(Color(255, 0, 255, 255), "Ghost spawned!\n");

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}
void CMomentumOnlineGhostEntity::Think()
{
    BaseClass::Think();
    HandleGhost();

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}
void CMomentumOnlineGhostEntity::HandleGhost()
{
    if (hasSpawned)
    {
        SetAbsOrigin(Vector(m_pCurrentFrame->Position.x + 25, m_pCurrentFrame->Position.y + 25, m_pCurrentFrame->Position.z));
        QAngle newAngles = QAngle(m_pCurrentFrame->EyeAngle.x / 10, m_pCurrentFrame->EyeAngle.y, m_pCurrentFrame->EyeAngle.z);
        SetAbsAngles(newAngles);
        SetViewOffset(m_pCurrentFrame->ViewOffset);
        Q_strncpy(m_pszGhostName, m_pCurrentFrame->PlayerName, sizeof(m_pszGhostName));

        bool isDucking = (GetFlags() & FL_DUCKING) != 0;
        if (m_pCurrentFrame->Buttons & IN_DUCK)
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
        m_pPreviousFrame = m_pCurrentFrame;
        // remove the nodraw effects
        if (GetRenderMode() != kRenderTransColor)
        {
            SetRenderMode(kRenderTransColor);
            RemoveEffects(EF_NOSHADOW);
        }
    }

    ConDColorMsg(Color(255, 255, 0, 255), "Position of ghost %s: %f, %f, %f\n", m_pszGhostName, GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);

}
void CMomentumOnlineGhostEntity::HandleGhostFirstPerson()
{
    if (m_pCurrentSpecPlayer)
    {
        if (m_pCurrentSpecPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
        {
            SetAbsAngles(m_pCurrentFrame->EyeAngle);
            // don't render the model when we're in first person mode
            if (GetRenderMode() != kRenderNone)
            {
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
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
        
        if (m_pPreviousFrame != nullptr)
        {
            const Vector &ghostPrevOrigin = m_pPreviousFrame->Position;
            const Vector &ghostCurrentOrigin = m_pCurrentFrame->Position;
            const float distX = fabs(ghostPrevOrigin.x - ghostCurrentOrigin.x);
            const float distY = fabs(ghostPrevOrigin.y - ghostCurrentOrigin.y);
            const float distZ = fabs(ghostPrevOrigin.z - ghostCurrentOrigin.z);
            const Vector interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;

            // Fixes an issue with teleporting
            int maxvel = sv_maxvelocity.GetInt();
            if (interpolatedVel.x <= maxvel && interpolatedVel.y <= maxvel && interpolatedVel.z <= maxvel)
            {
                SetAbsVelocity(interpolatedVel);
            }

            UpdateStats(interpolatedVel);
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
void CMomentumOnlineGhostEntity::SetCurrentNetFrame(ghostNetFrame_t *newFrame)
{
    if (newFrame)
    {
        m_pCurrentFrame = newFrame;
    }
}
void CMomentumOnlineGhostEntity::SetGhostApperence(ghostAppearance_t app)
{
    SetGhostBodyGroup(app.GhostModelBodygroup);
    SetGhostModel(app.GhostModel);
    SetGhostColor(*g_pMomentumUtil->GetColorFromHex(app.GhostModelRGBAColorAsHex));
}
