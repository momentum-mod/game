#include "cbase.h"
#include "mom_triggers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ##################################################################################
//	>> TriggerMultiple
// ##################################################################################
LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CTriggerMultiple)
END_PREDICTION_DATA();

#undef CTriggerMultiple // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_TriggerMultiple, DT_TriggerMultiple, CTriggerMultiple)
END_RECV_TABLE();
#define CTriggerMultiple C_TriggerMultiple // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CTriggerMultiple, DT_TriggerMultiple)
END_SEND_TABLE();

BEGIN_DATADESC(CTriggerMultiple)
	// Function Pointers
	DEFINE_FUNCTION(MultiTouch),
	DEFINE_FUNCTION(MultiWaitOver),

	// Outputs
	DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
END_DATADESC();
#endif

// Global list of triggers that care about weapon fire
// Doesn't need saving, the triggers re-add themselves on restore.
CUtlVector< CHandle<CTriggerMultiple> >	g_hWeaponFireTriggers;

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerMultiple::Spawn(void)
{
	BaseClass::Spawn();

	InitTrigger();

	if (m_flWait == 0)
	{
		m_flWait = 0.2;
	}

#ifdef GAME_DLL
	ASSERTSZ(m_iHealth == 0, "trigger_multiple with health");
#endif
	SetTouch(&CTriggerMultiple::MultiTouch);
}

//-----------------------------------------------------------------------------
// Purpose: Touch function. Activates the trigger.
// Input  : pOther - The thing that touched us.
//-----------------------------------------------------------------------------
void CTriggerMultiple::MultiTouch(CBaseEntity *pOther)
{
	if (PassesTriggerFilters(pOther))
	{
		ActivateMultiTrigger(pOther);
	}
}

//-----------------------------------------------------------------------------
// Purpose: The wait time has passed, so set back up for another activation
//-----------------------------------------------------------------------------
void CTriggerMultiple::MultiWaitOver(void)
{
	SetThink(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pActivator - 
//-----------------------------------------------------------------------------
void CTriggerMultiple::ActivateMultiTrigger(CBaseEntity *pActivator)
{
	if (GetNextThink() > gpGlobals->curtime)
		return;         // still waiting for reset time

	m_hActivator = pActivator;

	m_OnTrigger.FireOutput(m_hActivator, this);

	if (m_flWait > 0)
	{
		SetThink(&CTriggerMultiple::MultiWaitOver);
		SetNextThink(gpGlobals->curtime + m_flWait);
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.1f);
		SetThink(&CTriggerMultiple::SUB_Remove);
	}
}

// ##################################################################################
//	>> TriggerOnce
// ##################################################################################
LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CTriggerOnce)
END_PREDICTION_DATA();

#undef CTriggerOnce // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_TriggerOnce, DT_TriggerOnce, CTriggerOnce)
END_RECV_TABLE();
#define CTriggerOnce C_TriggerOnce // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CTriggerOnce, DT_TriggerOnce)
END_SEND_TABLE();

BEGIN_DATADESC(CTriggerOnce)
END_DATADESC();
#endif

void CTriggerOnce::Spawn(void)
{
	BaseClass::Spawn();

	m_flWait = -1;
}

// ##################################################################################
//	>> TriggerLook
//
//  Triggers once when player is looking at m_target
//
// ##################################################################################
LINK_ENTITY_TO_CLASS(trigger_look, CTriggerLook);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CTriggerLook)
END_PREDICTION_DATA();

#undef CTriggerLook // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_TriggerLook, DT_TriggerLook, CTriggerLook)
END_RECV_TABLE();
#define CTriggerLook C_TriggerLook // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CTriggerLook, DT_TriggerLook)
END_SEND_TABLE();

BEGIN_DATADESC(CTriggerLook)
	DEFINE_FIELD(m_hLookTarget, FIELD_EHANDLE),
	DEFINE_FIELD(m_flLookTimeTotal, FIELD_FLOAT),
	DEFINE_FIELD(m_flLookTimeLast, FIELD_TIME),
	DEFINE_KEYFIELD(m_flTimeoutDuration, FIELD_FLOAT, "timeout"),
	DEFINE_FIELD(m_bTimeoutFired, FIELD_BOOLEAN),
	DEFINE_FIELD(m_hActivator, FIELD_EHANDLE),

	DEFINE_OUTPUT(m_OnTimeout, "OnTimeout"),

	DEFINE_FUNCTION(TimeoutThink),

	// Inputs
	DEFINE_INPUT(m_flFieldOfView, FIELD_FLOAT, "FieldOfView"),
	DEFINE_INPUT(m_flLookTime, FIELD_FLOAT, "LookTime"),
END_DATADESC();
#endif

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerLook::Spawn(void)
{
	m_hLookTarget = NULL;
	m_flLookTimeTotal = -1;
	m_bTimeoutFired = false;

	BaseClass::Spawn();
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerLook::Touch(CBaseEntity *pOther)
{
	// Don't fire the OnTrigger if we've already fired the OnTimeout. This will be
	// reset in OnEndTouch.
	if (m_bTimeoutFired)
		return;

	// --------------------------------
	// Make sure we have a look target
	// --------------------------------
	if (m_hLookTarget == NULL)
	{
		m_hLookTarget = FindEntityByNameCRC(nullptr, m_iTargetCRC);
		if (m_hLookTarget == NULL)
		{
			return;
		}
	}

	// This is designed for single player only
	// so we'll always have the same player
	if (pOther->IsPlayer())
	{
		// ----------------------------------------
		// Check that toucher is facing the target
		// ----------------------------------------
#ifdef CLIENT_DLL
		QAngle vLookDir;
#else
		Vector vLookDir;
#endif
		if (HasSpawnFlags(SF_TRIGGERLOOK_USEVELOCITY))
		{
#ifdef GAME_DLL
			vLookDir = pOther->GetAbsVelocity();
			if (vLookDir == vec3_origin)
			{
				// See if they're in a vehicle
				CBasePlayer *pPlayer = (CBasePlayer *)pOther;
				if (pPlayer->IsInAVehicle())
				{
					vLookDir = pPlayer->GetVehicle()->GetVehicleEnt()->GetSmoothedVelocity();
				}
			}
			VectorNormalize(vLookDir);
#endif
		}
		else
		{
#ifdef CLIENT_DLL
			vLookDir = ((CBaseCombatCharacter*)pOther)->GetAbsAngles();
#else
			vLookDir = ((CBaseCombatCharacter*)pOther)->EyeDirection3D();
#endif
		}

		Vector vTargetDir = m_hLookTarget->GetAbsOrigin() - pOther->EyePosition();
		VectorNormalize(vTargetDir);

		float fDotPr = DotProduct(*reinterpret_cast<Vector*>(&vLookDir), vTargetDir);
		if (fDotPr > m_flFieldOfView)
		{
			// Is it the first time I'm looking?
			if (m_flLookTimeTotal == -1)
			{
				m_flLookTimeLast = gpGlobals->curtime;
				m_flLookTimeTotal = 0;
			}
			else
			{
				m_flLookTimeTotal += gpGlobals->curtime - m_flLookTimeLast;
				m_flLookTimeLast = gpGlobals->curtime;
			}

			if (m_flLookTimeTotal >= m_flLookTime)
			{
				Trigger(pOther, false);
			}
		}
		else
		{
			m_flLookTimeTotal = -1;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pOther - 
//-----------------------------------------------------------------------------
void CTriggerLook::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);

	if (pOther->IsPlayer() && m_flTimeoutDuration)
	{
		m_bTimeoutFired = false;
		m_hActivator = pOther;
		SetThink(&CTriggerLook::TimeoutThink);
		SetNextThink(gpGlobals->curtime + m_flTimeoutDuration);
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerLook::EndTouch(CBaseEntity *pOther)
{
	BaseClass::EndTouch(pOther);

	if (pOther->IsPlayer())
	{
		SetThink(NULL);
		SetNextThink(TICK_NEVER_THINK);

		m_flLookTimeTotal = -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the trigger is fired by look logic or timeout.
//-----------------------------------------------------------------------------
void CTriggerLook::Trigger(CBaseEntity *pActivator, bool bTimeout)
{
	if (bTimeout)
	{
		// Fired due to timeout (player never looked at the target).
		m_OnTimeout.FireOutput(pActivator, this);

		// Don't fire the OnTrigger for this toucher.
		m_bTimeoutFired = true;
	}
	else
	{
		// Fire because the player looked at the target.
		m_OnTrigger.FireOutput(pActivator, this);
		m_flLookTimeTotal = -1;

		// Cancel the timeout think.
		SetThink(NULL);
		SetNextThink(TICK_NEVER_THINK);
	}

	if (HasSpawnFlags(SF_TRIGGERLOOK_FIREONCE))
	{
		SetThink(&CTriggerLook::SUB_Remove);
		SetNextThink(gpGlobals->curtime);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerLook::TimeoutThink(void)
{
	Trigger(m_hActivator, true);
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CTriggerLook::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		// ----------------
		// Print Look time
		// ----------------
		char tempstr[255];
		Q_snprintf(tempstr, sizeof(tempstr), "Time:   %3.2f", m_flLookTime - MAX(0, m_flLookTimeTotal));
		EntityText(text_offset, tempstr, 0);
		text_offset++;
	}
	return text_offset;
}
#endif

// ##################################################################################
//	>> TriggerPush
//
//  Purpose: A trigger that pushes the player, NPCs, or objects.
//
// ##################################################################################
LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CTriggerPush)
END_PREDICTION_DATA();

void TriggerPushProxy_PushDir(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CTriggerPush *entity = (CTriggerPush *)pStruct;

	*(float*)&entity->m_vecPushDir.Get().x = pData->m_Value.m_Vector[0];
	*(float*)&entity->m_vecPushDir.Get().y = pData->m_Value.m_Vector[1];
	*(float*)&entity->m_vecPushDir.Get().z = pData->m_Value.m_Vector[2];

	// Convert pushdir from angles to a vector
	Vector vecAbsDir;
	QAngle angPushDir = QAngle(entity->m_vecPushDir.Get().x, entity->m_vecPushDir.Get().y, entity->m_vecPushDir.Get().z);
	AngleVectors(angPushDir, &vecAbsDir);

	// Transform the vector into entity space
	VectorIRotate( vecAbsDir, entity->EntityToWorldTransform(), (Vector)entity->m_vecPushDir.Get() );

	if (entity->m_flSpeed == 0)
	{
		entity->m_flSpeed = 100;
	}
}

#undef CTriggerPush // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_TriggerPush, DT_TriggerPush, CTriggerPush)
	RecvPropFloat(RECVINFO(m_flAlternateTicksFix)),
	RecvPropFloat(RECVINFO(m_flPushSpeed)),
	RecvPropVector(RECVINFO(m_vecPushDir), 0, TriggerPushProxy_PushDir),
END_RECV_TABLE();
#define CTriggerPush C_TriggerPush // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CTriggerPush, DT_TriggerPush)
	SendPropFloat(SENDINFO(m_flAlternateTicksFix)),
	SendPropFloat(SENDINFO(m_flPushSpeed)),
	SendPropVector(SENDINFO(m_vecPushDir)),
END_SEND_TABLE();

BEGIN_DATADESC(CTriggerPush)
	DEFINE_KEYFIELD(m_vecPushDir, FIELD_VECTOR, "pushdir"),
	DEFINE_KEYFIELD(m_flAlternateTicksFix, FIELD_FLOAT, "alternateticksfix"),
	//DEFINE_FIELD( m_flPushSpeed, FIELD_FLOAT ),
END_DATADESC();
#endif

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerPush::Spawn()
{
#ifdef CLIENT_DLL
	m_iUserID = INVALID_USER_ID;
	m_nTickBasePush = -1;
#endif
	// Convert pushdir from angles to a vector
	Vector vecAbsDir;
	QAngle angPushDir = QAngle(m_vecPushDir.Get().x, m_vecPushDir.Get().y, m_vecPushDir.Get().z);
	AngleVectors(angPushDir, &vecAbsDir);

	// Transform the vector into entity space
	VectorIRotate(vecAbsDir, EntityToWorldTransform(), m_vecPushDir.GetForModify());

	BaseClass::Spawn();

	InitTrigger();

	if (m_flSpeed == 0)
	{
		m_flSpeed = 100;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTriggerPush::Activate()
{
	// Fix problems with triggers pushing too hard under sv_alternateticks.
	// This is somewhat hacky, but it's simple and we're really close to shipping.
	ConVarRef sv_alternateticks("sv_alternateticks");
	if ((m_flAlternateTicksFix != 0) && sv_alternateticks.GetBool())
	{
		m_flPushSpeed = m_flSpeed * m_flAlternateTicksFix;
	}
	else
	{
		m_flPushSpeed = m_flSpeed;
	}

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CTriggerPush::Touch(CBaseEntity *pOther)
{
	if (!pOther->IsSolid() || (pOther->GetMoveType() == MOVETYPE_PUSH || pOther->GetMoveType() == MOVETYPE_NONE))
		return;

	if (!PassesTriggerFilters(pOther))
		return;

	// FIXME: If something is hierarchically attached, should we try to push the parent?
	if (pOther->GetMoveParent())
		return;

#ifdef CLIENT_DLL
	if (!pOther->IsPlayer())
		return;

	CBasePlayer* player = (CBasePlayer*)pOther;
#endif

	// Transform the push dir into global space
	Vector vecAbsDir;
	VectorRotate(m_vecPushDir.Get(), EntityToWorldTransform(), vecAbsDir);

	// Instant trigger, just transfer velocity and remove
	if (HasSpawnFlags(SF_TRIG_PUSH_ONCE)
#ifdef CLIENT_DLL
		&&
		((player->GetTickBase() == m_nTickBasePush && player->GetUserID() == m_iUserID) ||
		(m_nTickBasePush == -1 && m_iUserID == INVALID_USER_ID))
#endif
		)
	{
		pOther->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);

		if (vecAbsDir.z > 0)
		{
			pOther->SetGroundEntity(NULL);
		}
#ifdef CLIENT_DLL
		m_iUserID = player->GetUserID();
		m_nTickBasePush = player->GetTickBase();
#else // We dont need this on client as it gets deleted via networking the deletion
		UTIL_Remove(this);
#endif
		return;
	}

	switch (pOther->GetMoveType())
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
		break;

	case MOVETYPE_VPHYSICS:
	{
		IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
		if (pPhys)
		{
			// UNDONE: Assume the velocity is for a 100kg object, scale with mass
			pPhys->ApplyForceCenter(m_flPushSpeed * vecAbsDir * 100.0f * gpGlobals->frametime);
			return;
		}
	}
	break;

	default:
	{
		Vector vecPush = (m_flPushSpeed * vecAbsDir);
		if ((pOther->GetFlags() & FL_BASEVELOCITY)/* && !lagcompensation->IsCurrentlyDoingLagCompensation()*/) // lol
		{
			vecPush = vecPush + pOther->GetBaseVelocity();
		}
		if (vecPush.z > 0 && (pOther->GetFlags() & FL_ONGROUND))
		{
			pOther->SetGroundEntity(NULL);
			Vector origin = pOther->GetAbsOrigin();
			origin.z += 1.0f;
			pOther->SetAbsOrigin(origin);
		}		

		pOther->SetBaseVelocity(vecPush);
		pOther->AddFlag(FL_BASEVELOCITY);
	}
	break;
	}
}

// ##################################################################################
//	>> TriggerTeleport
//
//  Purpose: A trigger that teleports things
//
// ##################################################################################
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CTriggerTeleport)
END_PREDICTION_DATA();

#undef CTriggerTeleport // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTeleport, DT_TriggerTeleport, CTriggerTeleport)
	RecvPropInt(RECVINFO(m_iLandmarkCRC)),
END_RECV_TABLE();
#define CTriggerTeleport C_TriggerTeleport // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CTriggerTeleport, DT_TriggerTeleport)
	SendPropInt(SENDINFO(m_iLandmarkCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CTriggerTeleport)
	DEFINE_KEYFIELD(m_iLandmark, FIELD_STRING, "landmark"),
END_DATADESC();
#endif

const int SF_TELEPORT_PRESERVE_ANGLES = 0x20; // Preserve angles even when a local landmark is not specified

//-----------------------------------------------------------------------------
// Purpose: Teleports the entity that touched us to the location of our target,
//			setting the toucher's angles to our target's angles if they are a
//			player.
//
//			If a landmark was specified, the toucher is offset from the target
//			by their initial offset from the landmark and their angles are
//			left alone.
//
// Input  : pOther - The entity that touched us.
//-----------------------------------------------------------------------------
void CTriggerTeleport::Touch(CBaseEntity *pOther)
{
	CBaseEntity *pentTarget = nullptr;
	CBaseEntity *pentLandmark = nullptr;

	if (!PassesTriggerFilters(pOther))
	{
		return;
	}

	// The activator and caller are the same
	pentTarget = FindEntityByNameCRC(pentTarget, m_iTargetCRC);

	if (!pentTarget)
	{
		return;
	}

	Vector tmp = pentTarget->GetAbsOrigin();
	QAngle tmp_angle = pentTarget->GetAbsAngles();

	if (m_iLandmarkCRC != 0)
	{
		// The activator and caller are the same
		pentLandmark = FindEntityByNameCRC(pentLandmark, m_iLandmarkCRC);

		if (pentLandmark)
		{
			tmp += pOther->GetAbsOrigin() - pentLandmark->GetAbsOrigin();
		}
	}

	if (!pentLandmark && pOther->IsPlayer())
	{
		// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
		tmp.z -= pOther->WorldAlignMins().z;
	}

	pOther->SetGroundEntity(NULL);

	//
	// Only modify the toucher's angles and zero their velocity if no landmark was specified.
	//
	const QAngle *pAngles = NULL;
	Vector *pVelocity = NULL;

	if (!pentLandmark && !HasSpawnFlags(SF_TELEPORT_PRESERVE_ANGLES))
	{
#ifdef CLIENT_DLL
		pOther->m_angNetworkAngles = tmp_angle;
		pOther->SetLocalAngles(tmp_angle);
#endif

		pVelocity = NULL; // BUGBUG - This does not set the player's velocity to zero!!!
	}

#ifdef CLIENT_DLL
	pOther->m_vecNetworkOrigin = tmp;
	pOther->SetLocalOrigin(tmp);

	C_BasePlayer* player = C_BasePlayer::GetLocalPlayer();
	if (player == pOther)
	{
		if (!pentLandmark && !HasSpawnFlags(SF_TELEPORT_PRESERVE_ANGLES))
		{
			// We need to do it this way to set viewangles at the same frame as the new orign is viewed to the screen
			if (prediction->GetIsFirstTimePredicted())
			{
				player->m_bFixViewAngle = true;
				player->m_vecFixedViewAngles = tmp_angle;
			}

			prediction->SetLocalViewAngles(tmp_angle);
		}
	}
#else
	pOther->Teleport(&tmp, pAngles, pVelocity);
#endif
}

#ifdef GAME_DLL
void CTriggerTeleport::Spawn(void)
{
	if (Q_strlen(STRING(m_iLandmark)))
	{
		CRC32_t crc;

		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, STRING(m_iLandmark), Q_strlen(STRING(m_iLandmark)));
		CRC32_Final(&crc);

		m_iLandmarkCRC = crc;
	}

	BaseClass::Spawn();

	InitTrigger();
}
#endif