#include "cbase.h"
#include "mom_online_ghost.h"
#include "in_buttons.h"
#include "fx_mom_shared.h"
#include "util/mom_util.h"
#include "weapon/cs_weapon_parse.h"
#include "mom_grenade_projectile.h"
#include "te_effect_dispatch.h"
#include "weapon/weapon_csbase.h"

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

static MAKE_CONVAR(mom_ghost_online_lerp, "0.5", FCVAR_REPLICATED | FCVAR_ARCHIVE, "The amount of time to render in the past (in seconds).\n", 0.1f, 2.0f);

static MAKE_TOGGLE_CONVAR(mom_ghost_online_rotations, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Allows wonky rotations of ghosts to be set.\n");
static MAKE_CONVAR(mom_ghost_online_interp_ticks, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Interpolation ticks to add to rendering online ghosts.\n", 0.0f, 100.0f);

extern ConVar mom_paintgun_shoot_sound;

CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity(): m_pCurrentFrame(nullptr), m_pNextFrame(nullptr)
{
    ListenForGameEvent("mapfinished_panel_closed");
    m_nGhostButtons = 0;
    m_bSpectating = false;
}

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
    m_vecPositionPackets.Purge();
    m_vecDecalPackets.Purge();
}

void CMomentumOnlineGhostEntity::AddPositionFrame(const PositionPacket_t &newFrame)
{
    m_vecPositionPackets.Insert(new ReceivedFrame_t<PositionPacket_t>(gpGlobals->curtime, newFrame));
}

void CMomentumOnlineGhostEntity::AddDecalFrame(const DecalPacket_t &decal)
{
    m_vecDecalPackets.Insert(new ReceivedFrame_t<DecalPacket_t>(gpGlobals->curtime, decal));
}

void CMomentumOnlineGhostEntity::FireDecal(const DecalPacket_t &decal)
{
    switch (decal.decal_type)
    {
    case DECAL_BULLET:
        if (decal.iWeaponID == WEAPON_GRENADE)
        {
            // Grenades behave differently
            ThrowGrenade(decal);
        }
        else
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
        break;
    case DECAL_PAINT:
        DoPaint(decal);
        break;
    case DECAL_KNIFE:
        DoKnifeSlash(decal);
    default:
        break;
    }
}

void CMomentumOnlineGhostEntity::FireGameEvent(IGameEvent *pEvent)
{
    if (FStrEq(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        m_pCurrentSpecPlayer = nullptr;
    }
}

void CMomentumOnlineGhostEntity::DoPaint(const DecalPacket_t& packet)
{
    Vector vecDirShooting;
    AngleVectors(packet.vAngle, &vecDirShooting);

    Vector vecEnd = packet.vOrigin + vecDirShooting * 8192.0f;

    trace_t tr; // main enter bullet trace

    Ray_t ray;
    ray.Init(packet.vOrigin, vecEnd);
    CTraceFilterSkipTwoEntities traceFilter(this, UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE);
    enginetrace->TraceRay(ray, MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, &traceFilter, &tr);
    if (r_visualizetraces.GetBool())
    {
        DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, true, -1.0f);
    }

    if (tr.fraction == 1.0f)
        return; // we didn't hit anything, stop tracing shoot

    CBaseEntity *pEntity = tr.m_pEnt;

    // Build the impact data
    CEffectData data;
    data.m_vOrigin = tr.endpos;
    data.m_vStart = tr.startpos;
    data.m_nSurfaceProp = tr.surface.surfaceProps;
    data.m_nHitBox = tr.hitbox;
    data.m_nEntIndex = pEntity->entindex();
    // Build the custom online ghost data
    data.m_bCustomColors = true; // Used to determine if an online entity
    data.m_nDamageType = packet.iWeaponID; // Color data
    data.m_flScale = packet.fSpread; // Scale of decal

    DispatchEffect("Painting", data);

    // Play the paintgun sound
    if (mom_paintgun_shoot_sound.GetBool())
    {
        CCSWeaponInfo *pWeaponInfo = GetWeaponInfo(WEAPON_PAINTGUN);
        if (pWeaponInfo)
        {
            // If we have some sounds from the weapon classname.txt file, play a random one of them
            const char *shootsound = pWeaponInfo->aShootSounds[SINGLE];
            if (!shootsound || !shootsound[0])
                return;

            CBroadcastRecipientFilter filter;
            if (!te->CanPredict())
                return;

            EmitSound(filter, entindex(), shootsound, &packet.vOrigin);
        }
    }
}

void CMomentumOnlineGhostEntity::DoKnifeSlash(const DecalPacket_t &packet)
{
    trace_t tr;
    Vector vForward;
    // Trace data here, play miss sound and do damage if hit
    g_pMomentumUtil->KnifeTrace(packet.vOrigin, packet.vAngle, packet.iWeaponID == 1, this, this, &tr, &vForward);
    // Play the smacking sounds and do the decal if it actually hit
    g_pMomentumUtil->KnifeSmack(tr, this, packet.vAngle, packet.iWeaponID == 1);
}

void CMomentumOnlineGhostEntity::ThrowGrenade(const DecalPacket_t& packet)
{
    CCSWeaponInfo *pGrenadeInfo = GetWeaponInfo(WEAPON_GRENADE);
    if (pGrenadeInfo)
    {
        // Vector values stored in a QAngle, shhh~
        Vector vecThrow(packet.vAngle.x, packet.vAngle.y, packet.vAngle.z);
        auto grenade = CMomGrenadeProjectile::Create(packet.vOrigin, vec3_angle, vecThrow, AngularImpulse(600, packet.iMode, 0), this, pGrenadeInfo->szWorldModel);
        grenade->SetDamage(0.0f); // These grenades should not do damage

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
    SetNextThink(gpGlobals->curtime + mom_ghost_online_lerp.GetFloat());
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
    float flCurtime = gpGlobals->curtime - mom_ghost_online_lerp.GetFloat(); // Render in a predetermined past buffer (allow some dropped packets)

    if (!m_vecDecalPackets.IsEmpty())
    {
        // Similar fast-forward code to the positions except we aren't jumping here,
        // we want to place these decals ASAP (sound spam incoming) and get them out of the queue.
        int upperBound = static_cast<int>(ceil(mom_ghost_online_lerp.GetFloat() * mm_updaterate.GetFloat()));
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
        // If there's ever any excess, we need to get rid of it, immediately.
        int upperBound = static_cast<int>(ceil(mom_ghost_online_lerp.GetFloat() * mm_updaterate.GetFloat()));
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

void CMomentumOnlineGhostEntity::UpdatePlayerSpectate()
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        m_pCurrentSpecPlayer->TravelSpectateTargets(true);
    }
}
