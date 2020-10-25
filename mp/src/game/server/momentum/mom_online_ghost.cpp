#include "cbase.h"
#include "mom_online_ghost.h"

#include "mom_player_shared.h"

#include "fx_mom_shared.h"
#include "mom_grenade_projectile.h"
#include "mom_rocket.h"
#include "te_effect_dispatch.h"
#include "weapon/weapon_def.h"
#include "weapon/weapon_knife.h"
#include "ghost_client.h"
#include "mom_stickybomb.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(mom_online_ghost, CMomentumOnlineGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumOnlineGhostEntity, DT_MOM_OnlineGhost)
SendPropBool(SENDINFO(m_bSpectating)),
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 0), 8, SPROP_ROUNDDOWN, -32.0, 32.0f),
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 1), 8, SPROP_ROUNDDOWN, -32.0, 32.0f),
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 2), 20, SPROP_CHANGES_OFTEN, 0.0f, 256.0f),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumOnlineGhostEntity)
END_DATADESC();

static void RefreshGhostData(IConVar *var, const char *pValue, float oldValue)
{
    g_pMomentumGhostClient->ResetOtherAppearanceData();
}

static MAKE_CONVAR(mom_ghost_online_lerp, "0.5", FCVAR_REPLICATED | FCVAR_ARCHIVE, "The amount of time to render in the past (in seconds).\n", 0.1f, 2.0f);

static MAKE_TOGGLE_CONVAR(mom_ghost_online_rotations, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Allows wonky rotations of ghosts to be set.\n");
static MAKE_CONVAR(mom_ghost_online_interp_ticks, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Interpolation ticks to add to rendering online ghosts.\n", 0.0f, 100.0f);

static MAKE_TOGGLE_CONVAR(mom_ghost_online_sounds, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE,
                          "Toggle other player's flashlight sounds. 0 = OFF, 1 = ON.\n");

static MAKE_TOGGLE_CONVAR_C(mom_ghost_online_alpha_override_enable, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE,
                            "Toggle overriding other player's ghost alpha values to the one defined in "
                            "\"mom_ghost_online_color_alpha_override\".\n",
                            RefreshGhostData);

static MAKE_CONVAR_C(mom_ghost_online_alpha_override, "100", FCVAR_REPLICATED | FCVAR_ARCHIVE,
                     "Overrides ghosts alpha to be this value.\n", 0, 255, RefreshGhostData);

static MAKE_TOGGLE_CONVAR_C(mom_ghost_online_trail_enable, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE,
                            "Toggles drawing other ghost's trails. 0 = OFF, 1 = ON\n", RefreshGhostData);

static MAKE_TOGGLE_CONVAR_C(mom_ghost_online_flashlights_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggles drawing other ghosts' flashlights. 0 = OFF, 1 = ON\n", RefreshGhostData);

static MAKE_CONVAR(mom_ghost_online_sticky_alpha, "50", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Sets the ghost stickybomb alpha value. 10 = more transparent, 255 = opaque.", 10.0f, 255.0f);

CMomentumOnlineGhostEntity::CMomentumOnlineGhostEntity() : m_pCurrentFrame(nullptr), m_pNextFrame(nullptr), m_cvarPaintSound("mom_paint_apply_sound")
{
    ListenForGameEvent("mapfinished_panel_closed");
    m_nGhostButtons = 0;
    m_bSpectating = false;
    m_specTargetID = 0;
}

CMomentumOnlineGhostEntity::~CMomentumOnlineGhostEntity()
{
    m_vecPositionPackets.Purge();
    m_vecDecalPackets.Purge();
}

void CMomentumOnlineGhostEntity::AddPositionFrame(const PositionPacket &newFrame)
{
    m_vecPositionPackets.Insert(new ReceivedFrame_t<PositionPacket>(gpGlobals->curtime, newFrame));
}

void CMomentumOnlineGhostEntity::AddDecalFrame(const DecalPacket &decal)
{
    m_vecDecalPackets.Insert(new ReceivedFrame_t<DecalPacket>(gpGlobals->curtime, decal));
}

void CMomentumOnlineGhostEntity::FireDecal(const DecalPacket &decal)
{
    switch (decal.decal_type)
    {
    case DECAL_BULLET:
        if (decal.data.bullet.iAmmoType == AMMO_TYPE_GRENADE)
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
                decal.data.bullet.iAmmoType,
                decal.data.bullet.iMode,
                decal.data.bullet.iSeed,
                decal.data.bullet.fSpread);
        }
        break;
    case DECAL_PAINT:
        DoPaint(decal);
        break;
    case DECAL_KNIFE:
        DoKnifeSlash(decal);
        break;
    case DECAL_ROCKET:
        FireRocket(decal);
        break;
    case DECAL_STICKY_SHOOT:
        FireSticky(decal);
        break;
    case DECAL_STICKY_DETONATE:
        DetonateStickies();
        break;
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

void CMomentumOnlineGhostEntity::DoPaint(const DecalPacket& packet)
{
    Vector vecDirShooting;
    AngleVectors(packet.vAngle, &vecDirShooting);

    Vector vecEnd = packet.vOrigin + vecDirShooting * MAX_TRACE_LENGTH;

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
    data.m_nDamageType = packet.data.paint.color.GetRawColor(); // Color data
    data.m_flScale = packet.data.paint.fDecalRadius; // Scale of decal

    DispatchEffect("Painting", data);

    // Play the paint shot sound
    if (m_cvarPaintSound.GetBool())
    {
        CPASAttenuationFilter filter(packet.vOrigin, SND_PAINT_SHOT);
        if (!te->CanPredict())
            return;

        EmitSound(filter, entindex(), SND_PAINT_SHOT, &packet.vOrigin);
    }
}

void CMomentumOnlineGhostEntity::DoKnifeSlash(const DecalPacket &packet)
{
    trace_t tr;
    Vector vForward;
    // Trace data here, play miss sound and do damage if hit
    CKnife::KnifeTrace(packet.vOrigin, packet.vAngle, packet.data.knife.bStab, this, this, &tr, &vForward);
    // Play the smacking sounds and do the decal if it actually hit
    CKnife::KnifeSmack(tr, this, packet.vAngle, packet.data.knife.bStab);
}

void CMomentumOnlineGhostEntity::ThrowGrenade(const DecalPacket& packet)
{
    // Vector values stored in a QAngle, shhh~
    Vector vecThrow(packet.vAngle.x, packet.vAngle.y, packet.vAngle.z);
    CMomGrenadeProjectile::Create(packet.vOrigin, vec3_angle, vecThrow, AngularImpulse(600, packet.data.bullet.iMode, 0), this);
}

void CMomentumOnlineGhostEntity::FireRocket(const DecalPacket &packet)
{
    EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_ROCKETLAUNCHER, "single_shot"));

    CMomRocket::EmitRocket(packet.vOrigin, packet.vAngle, this);
}

void CMomentumOnlineGhostEntity::FireSticky(const DecalPacket &packet)
{
    EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_STICKYLAUNCHER, "single_shot"));

    const auto pSticky = CMomStickybomb::Create(packet.vOrigin, packet.vAngle, packet.data.stickyShoot.velocity, this);

    if (pSticky)
    {
        pSticky->SetRenderColorA(mom_ghost_online_sticky_alpha.GetInt());
    }

    // If we've gone over the max stickybomb count, fizzle the oldest
    if (m_vecExplosives.Count() > MOM_WEAPON_STICKYBOMB_COUNT)
    {
        const auto pTemp = m_vecExplosives[0];
        if (pTemp)
        {
            pTemp->Fizzle();
        }
    }
}

void CMomentumOnlineGhostEntity::DetonateStickies()
{
    FOR_EACH_VEC_BACK(m_vecExplosives, i)
    {
        CMomStickybomb *pTemp = dynamic_cast<CMomStickybomb *>(m_vecExplosives[i]);
        if (pTemp)
        {
            // This guy will die soon enough.
            if (pTemp->IsEffectActive(EF_NODRAW))
                continue;

            if (!pTemp->IsArmed())
            {
                continue;
            }

            pTemp->Detonate();
        }
    }
}

void CMomentumOnlineGhostEntity::SetGhostName(const char *pGhostName)
{
    Q_strncpy(m_szGhostName.GetForModify(), pGhostName, MAX_PLAYER_NAME_LENGTH);
}

void CMomentumOnlineGhostEntity::AppearanceFlashlightChanged(const AppearanceData_t &newApp)
{
    CMomRunEntity::AppearanceFlashlightChanged(newApp);

    SetGhostFlashlight(newApp.m_bFlashlightEnabled);
}

void CMomentumOnlineGhostEntity::AppearanceModelColorChanged(const AppearanceData_t &newApp)
{
    CMomRunEntity::AppearanceModelColorChanged(newApp);

    if (mom_ghost_online_alpha_override_enable.GetBool())
    {
        SetRenderColorA(mom_ghost_online_alpha_override.GetInt());
    }
}

void CMomentumOnlineGhostEntity::Spawn()
{
    BaseClass::Spawn();
    SetNextThink(gpGlobals->curtime + mom_ghost_online_lerp.GetFloat());
}

void CMomentumOnlineGhostEntity::CreateTrail()
{
    RemoveTrail();

    if (!mom_ghost_online_trail_enable.GetBool())
        return;

    BaseClass::CreateTrail();
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
            ReceivedFrame_t<DecalPacket> *fireMeImmedately = m_vecDecalPackets.RemoveAtHead();
            FireDecal(fireMeImmedately->frame);
            delete fireMeImmedately;
        }

        if (m_vecDecalPackets.Head()->recvTime < flCurtime)
        {
            ReceivedFrame_t<DecalPacket> *fireMe = m_vecDecalPackets.RemoveAtHead();
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
            ReceivedFrame_t<PositionPacket> *pTemp = m_vecPositionPackets.RemoveAtHead();
            delete pTemp;
        }

        if (!m_pCurrentFrame)
        {
            ReceivedFrame_t<PositionPacket> *pHead = m_vecPositionPackets.Head();
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
            HandleDucking();
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

bool CMomentumOnlineGhostEntity::GetCurrentPositionPacketData(PositionPacket *out) const
{
    if (out && !m_bSpectating.Get())
    {
        out->Position = GetAbsOrigin();
        out->Velocity = GetAbsVelocity();
        out->EyeAngle = m_vecLookAngles;
        out->Buttons = m_nGhostButtons;
        out->ViewOffset = GetViewOffset().z;
        return true;
    }
    return false;
}

void CMomentumOnlineGhostEntity::UpdatePlayerSpectate()
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        m_pCurrentSpecPlayer->TravelSpectateTargets(true);
    }
}

SpectateMessageType_t CMomentumOnlineGhostEntity::UpdateSpectateState(bool bIsSpec, uint64 specTargetID)
{
    SpectateMessageType_t type = SPEC_UPDATE_INVALID;
    const auto bTargetChanged = m_specTargetID != specTargetID;
    if (bTargetChanged)
    {
        m_specTargetID = specTargetID;
    }

    const auto bStateChanged = bIsSpec != m_bSpectating;
    if (bStateChanged)
    {
        if (!m_bSpectating && bIsSpec)
        {
            type = SPEC_UPDATE_STARTED;
        }
        else if (m_bSpectating && !bIsSpec)
        {
            type = SPEC_UPDATE_STOP;
        }

        SetIsSpectating(bIsSpec);
    }
    else if (bTargetChanged)
    {
        type = SPEC_UPDATE_CHANGETARGET;
    }

    return type;
}

void CMomentumOnlineGhostEntity::SetGhostFlashlight(bool bEnable)
{
    if (!mom_ghost_online_flashlights_enable.GetBool())
    {
        // In case they have it on still
        if (GetEffects() & EF_DIMLIGHT)
            RemoveEffects(EF_DIMLIGHT);
        return;
    }

    if (bEnable)
    {
        AddEffects(EF_DIMLIGHT);
        if (mom_ghost_online_sounds.GetBool())
            EmitSound(SND_FLASHLIGHT_ON);
    }
    else
    {
        RemoveEffects(EF_DIMLIGHT);
        if (mom_ghost_online_sounds.GetBool())
            EmitSound(SND_FLASHLIGHT_OFF);
    }
}

void CMomentumOnlineGhostEntity::SetIsSpectating(bool bState)
{
    m_bSpectating = bState;

    if (m_bSpectating)
    {
        DestroyExplosives();
        HideGhost();
    }
    else
    {
        UnHideGhost();
    }
}
