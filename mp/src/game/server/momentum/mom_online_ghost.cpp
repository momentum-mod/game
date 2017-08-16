#include "cbase.h"
#include "mom_online_ghost.h"
#include "util/mom_util.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

ConVar mm_ghostTesting("mom_ghost_testing", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, ".");

LINK_ENTITY_TO_CLASS(mom_online_ghost, CMomentumOnlineGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumOnlineGhostEntity, DT_MOM_OnlineGhost)
    SendPropString(SENDINFO(m_pszGhostName)),
    SendPropInt(SENDINFO(m_uiAccountID), -1, SPROP_UNSIGNED),
    SendPropInt(SENDINFO(m_nGhostButtons)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumOnlineGhostEntity)
END_DATADESC();

#define MOM_GHOST_LERP 0.1f // MOM_TODO: Change this to a convar

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
    m_vecFrames.Purge();
}

void CMomentumOnlineGhostEntity::SetCurrentNetFrame(ghostNetFrame_t newFrame)
{
    m_vecFrames.Insert(new ReceivedFrame_t(gpGlobals->curtime, newFrame));
}

CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity(): m_pCurrentFrame(nullptr), m_pNextFrame(nullptr)
{
    ListenForGameEvent("mapfinished_panel_closed");
    m_nGhostButtons = 0;
}

void CMomentumOnlineGhostEntity::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
        {
            m_pCurrentSpecPlayer->StopSpectating();
        }
        m_pCurrentSpecPlayer = nullptr;
    }
}
void CMomentumOnlineGhostEntity::Precache(void)
{
    BaseClass::Precache();
}

void CMomentumOnlineGhostEntity::SetGhostAppearance(LobbyGhostAppearance_t app)
{
    if (!FStrEq(m_CurrentAppearance.base64, app.base64))
    {
        //DevLog("Got a new appearance!\n Bodygroup: %i, Color: %lu, Model: %s\n", app.appearance.GhostModelBodygroup, app.appearance.GhostModelRGBAColorAsHex, app.appearance.GhostModel);
        m_CurrentAppearance = app;
        BaseClass::SetGhostAppearance(app.appearance);
    }
}

void CMomentumOnlineGhostEntity::Spawn()
{
    BaseClass::Spawn();
    SetNextThink(gpGlobals->curtime);
}
void CMomentumOnlineGhostEntity::Think()
{
    BaseClass::Think();
    HandleGhost();
    if (m_pCurrentSpecPlayer)
        HandleGhostFirstPerson();
    // Emulate at the update rate for the smoothest interpolation
    SetNextThink(gpGlobals->curtime + 1.0f / mm_updaterate.GetFloat());
}
void CMomentumOnlineGhostEntity::HandleGhost()
{
    float flCurtime = gpGlobals->curtime - MOM_GHOST_LERP; // Render in a 100 ms past buffer (allow some dropped packets)

    if (!m_vecFrames.IsEmpty())
    {
        // The fast-forward logic:
        // Realistically, we're going to have a buffer of about MOM_GHOST_LERP * update rate. So for 25 updates
        // in a second, a lerp of 0.1 seconds would make there be about 2.5 packets in the queue at all times.
        // If they pause, this increases to some arbitrary number of frames that we need to get rid of, immediately.
        int upperBound = 3 + static_cast<int>(ceil(MOM_GHOST_LERP * mm_updaterate.GetFloat()));
        while (m_vecFrames.Count() > upperBound)
        {
            ReceivedFrame_t *pTemp = m_vecFrames.RemoveAtHead();
            delete pTemp;
        }

        if (!m_pCurrentFrame)
        {
            ReceivedFrame_t *pHead = m_vecFrames.Head();
            if (flCurtime > pHead->recvTime)
                m_pCurrentFrame = m_vecFrames.RemoveAtHead();
        }
    }

    if (m_pCurrentFrame)
    {
        SetAbsOrigin(m_pCurrentFrame->frame.Position);
        SetAbsAngles(m_pCurrentFrame->frame.EyeAngle);
        SetViewOffset(Vector(0, 0, m_pCurrentFrame->frame.ViewOffset));
        SetAbsVelocity(m_pCurrentFrame->frame.Velocity);

        m_nGhostButtons = m_pCurrentFrame->frame.Buttons;

        // Get rid of our current frame
        delete m_pCurrentFrame;
        m_pCurrentFrame = nullptr;
    }
}

void CMomentumOnlineGhostEntity::HandleGhostFirstPerson()
{
    if (m_pCurrentSpecPlayer)
    {
        if (m_pCurrentSpecPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
        {
            // don't render the model when we're in first person mode
            if (GetRenderMode() != kRenderNone)
            {
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
            }
            bool isDucking = (GetFlags() & FL_DUCKING) != 0;
            if (m_nGhostButtons & IN_DUCK)
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
