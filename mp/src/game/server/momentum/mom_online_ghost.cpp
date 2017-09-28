#include "cbase.h"
#include "mom_online_ghost.h"
#include "in_buttons.h"
#include "fx_cs_shared.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(mom_online_ghost, CMomentumOnlineGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumOnlineGhostEntity, DT_MOM_OnlineGhost)
    SendPropString(SENDINFO(m_pszGhostName)),
    SendPropInt(SENDINFO(m_uiAccountID), -1, SPROP_UNSIGNED),
    SendPropInt(SENDINFO(m_nGhostButtons)),
    SendPropBool(SENDINFO(m_bSpectating)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumOnlineGhostEntity)
END_DATADESC();

#define MOM_GHOST_LERP 0.1f // MOM_TODO: Change this to a convar

static MAKE_TOGGLE_CONVAR(mom_ghost_online_rotations, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Allows wonky rotations of ghosts to be set.\n");
static MAKE_CONVAR(mom_ghost_online_interp_ticks, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Interpolation ticks to add to rendering online ghosts.\n", 0.0f, 100.0f);

CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity(): m_pCurrentFrame(nullptr), m_pNextFrame(nullptr)
{
    ListenForGameEvent("mapfinished_panel_closed");
    m_nGhostButtons = 0;
    m_bSpectating = false;
}

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
    m_vecPositionPackets.Purge();
}

void CMomentumOnlineGhostEntity::AddPositionFrame(PositionPacket_t newFrame)
{
    m_vecPositionPackets.Insert(new ReceivedFrame_t<PositionPacket_t>(gpGlobals->curtime, newFrame));
}

void CMomentumOnlineGhostEntity::AddDecalFrame(DecalPacket_t decal)
{
    m_vecDecalPackets.Insert(new ReceivedFrame_t<DecalPacket_t>(gpGlobals->curtime, decal));
}

void CMomentumOnlineGhostEntity::FireDecal(DecalPacket_t decal)
{
    if (decal.decal_type == DECAL_BULLET)
    {
        FX_FireBullets(
            entindex(),
            decal.vOrigin,
            decal.vAngle,
            decal.iWeaponID,
            decal.iMode,
            decal.iSeed,
            decal.fSpread);
    }
    else if (decal.decal_type == DECAL_PAINT)
    {
      // MOM_TODO: Spawn/fire the paint decal here   
    }
}

void CMomentumOnlineGhostEntity::FireGameEvent(IGameEvent *pEvent)
{
    if (FStrEq(pEvent->GetName(), "mapfinished_panel_closed"))
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

void CMomentumOnlineGhostEntity::SetGhostAppearance(LobbyGhostAppearance_t app, bool bForceUpdate /*= false*/)
{
    if ((!FStrEq(app.base64, "") && !FStrEq(m_CurrentAppearance.base64, app.base64)) || bForceUpdate)
    {
        m_CurrentAppearance = app;
        BaseClass::SetGhostAppearance(app.appearance, bForceUpdate);
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

    // Emulate at the update rate (or slightly slower) for the smoothest interpolation
    SetNextThink(gpGlobals->curtime + 1.0f / mm_updaterate.GetFloat() + (gpGlobals->interval_per_tick * mom_ghost_online_interp_ticks.GetFloat()));
}
void CMomentumOnlineGhostEntity::HandleGhost()
{
    float flCurtime = gpGlobals->curtime - MOM_GHOST_LERP; // Render in a 100 ms past buffer (allow some dropped packets)

    if (!m_vecDecalPackets.IsEmpty())
    {
        // Similar fast-forward code to the positions except we aren't jumping here,
        // we want to place these decals ASAP (sound spam incoming) and get them out of the queue.
        int upperBound = 3 + static_cast<int>(ceil(MOM_GHOST_LERP * mm_updaterate.GetFloat()));
        while (m_vecDecalPackets.Count() > upperBound)
        {
            ReceivedFrame_t<DecalPacket_t> *fireMeImmedately = m_vecDecalPackets.RemoveAtHead();
            FireDecal(fireMeImmedately->frame);
            delete fireMeImmedately;
        }

        if (m_vecDecalPackets.Head()->recvTime < flCurtime)
        {
            ReceivedFrame_t<DecalPacket_t> *fireMe = m_vecDecalPackets.RemoveAtHead();
            FireDecal(fireMe->frame);
            delete fireMe;
        }
    }

    if (!m_vecPositionPackets.IsEmpty())
    {
        // The fast-forward logic:
        // Realistically, we're going to have a buffer of about MOM_GHOST_LERP * update rate. So for 25 updates
        // in a second, a lerp of 0.1 seconds would make there be about 2.5 packets in the queue at all times.
        // If they pause, this increases to some arbitrary number of frames that we need to get rid of, immediately.
        int upperBound = 3 + static_cast<int>(ceil(MOM_GHOST_LERP * mm_updaterate.GetFloat()));
        while (m_vecPositionPackets.Count() > upperBound)
        {
            ReceivedFrame_t<PositionPacket_t> *pTemp = m_vecPositionPackets.RemoveAtHead();
            delete pTemp;
        }

        if (!m_pCurrentFrame)
        {
            ReceivedFrame_t<PositionPacket_t> *pHead = m_vecPositionPackets.Head();
            if (flCurtime > pHead->recvTime)
                m_pCurrentFrame = m_vecPositionPackets.RemoveAtHead();
        }
    }

    if (m_pCurrentFrame)
    {
        SetAbsOrigin(m_pCurrentFrame->frame.Position);

        m_vecLookAngles = m_pCurrentFrame->frame.EyeAngle;
        if (m_pCurrentSpecPlayer || mom_ghost_online_rotations.GetBool())
            SetAbsAngles(m_vecLookAngles);
        else
            SetAbsAngles(QAngle(0, m_vecLookAngles.y, m_vecLookAngles.z));

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
            HideGhost();
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
            // remove the nodraw effects
            UnHideGhost();
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
