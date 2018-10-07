#include "cbase.h"
#include "mom_basedoor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(func_door, CBaseDoor);

#ifdef GAME_DLL
// func_water is implemented as a linear door so we can raise/lower the water level.
LINK_ENTITY_TO_CLASS(func_water, CBaseDoor);
#endif

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CBaseDoor) // MOM_TODO: Add _NO_BASE stuff to predict here
	DEFINE_PRED_FIELD(m_flWaveHeight, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iChainTargetCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

#undef CBaseDoor // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_BaseDoor, DT_BaseDoor, CBaseDoor) // MOM_TODO: Add _NO_BASE stuff to predict here
	RecvPropFloat(RECVINFO(m_flWaveHeight)),
	RecvPropInt(RECVINFO(m_iChainTargetCRC)),
END_RECV_TABLE();
#define CBaseDoor C_BaseDoor // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CBaseDoor, DT_BaseDoor) // MOM_TODO: Add _NO_BASE stuff to predict here
	SendPropFloat(SENDINFO(m_flWaveHeight), 8, SPROP_ROUNDUP, 0.0f, 8.0f),
	SendPropInt(SENDINFO(m_iChainTargetCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CBaseDoor)
	DEFINE_KEYFIELD(m_vecMoveDir, FIELD_VECTOR, "movedir"),

	DEFINE_FIELD(m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(m_bUnlockedSentence, FIELD_CHARACTER),
	DEFINE_KEYFIELD(m_NoiseMoving, FIELD_SOUNDNAME, "noise1"),
	DEFINE_KEYFIELD(m_NoiseArrived, FIELD_SOUNDNAME, "noise2"),
	DEFINE_KEYFIELD(m_NoiseMovingClosed, FIELD_SOUNDNAME, "startclosesound"),
	DEFINE_KEYFIELD(m_NoiseArrivedClosed, FIELD_SOUNDNAME, "closesound"),
	DEFINE_KEYFIELD(m_ChainTarget, FIELD_STRING, "chainstodoor"),
	// DEFINE_FIELD( m_isChaining, FIELD_BOOLEAN ),
	// DEFINE_FIELD( m_ls, locksound_t ),
	//	DEFINE_FIELD( m_isChaining, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD(m_ls.sLockedSound, FIELD_SOUNDNAME, "locked_sound"),
	DEFINE_KEYFIELD(m_ls.sUnlockedSound, FIELD_SOUNDNAME, "unlocked_sound"),
	DEFINE_FIELD(m_bLocked, FIELD_BOOLEAN),
	DEFINE_KEYFIELD(m_flWaveHeight, FIELD_FLOAT, "WaveHeight"),
	DEFINE_KEYFIELD(m_flBlockDamage, FIELD_FLOAT, "dmg"),
	DEFINE_KEYFIELD(m_eSpawnPosition, FIELD_INTEGER, "spawnpos"),

	DEFINE_KEYFIELD(m_bForceClosed, FIELD_BOOLEAN, "forceclosed"),
	DEFINE_FIELD(m_bDoorGroup, FIELD_BOOLEAN),

	#ifdef HL1_DLL
	DEFINE_KEYFIELD(m_iBlockFilterName, FIELD_STRING, "filtername"),
	DEFINE_FIELD(m_hBlockFilter, FIELD_EHANDLE),
	#endif

	DEFINE_KEYFIELD(m_bLoopMoveSound, FIELD_BOOLEAN, "loopmovesound"),
	DEFINE_KEYFIELD(m_bIgnoreDebris, FIELD_BOOLEAN, "ignoredebris"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Open", InputOpen),
	DEFINE_INPUTFUNC(FIELD_VOID, "Close", InputClose),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
	DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock),
	DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSpeed", InputSetSpeed),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetToggleState", InputSetToggleState),

	DEFINE_OUTPUT(m_OnBlockedOpening, "OnBlockedOpening"),
	DEFINE_OUTPUT(m_OnBlockedClosing, "OnBlockedClosing"),
	DEFINE_OUTPUT(m_OnUnblockedOpening, "OnUnblockedOpening"),
	DEFINE_OUTPUT(m_OnUnblockedClosing, "OnUnblockedClosing"),
	DEFINE_OUTPUT(m_OnFullyClosed, "OnFullyClosed"),
	DEFINE_OUTPUT(m_OnFullyOpen, "OnFullyOpen"),
	DEFINE_OUTPUT(m_OnClose, "OnClose"),
	DEFINE_OUTPUT(m_OnOpen, "OnOpen"),
	DEFINE_OUTPUT(m_OnLockedUse, "OnLockedUse"),

	// Function Pointers
	DEFINE_FUNCTION(DoorTouch),
	DEFINE_FUNCTION(DoorGoUp),
	DEFINE_FUNCTION(DoorGoDown),
	DEFINE_FUNCTION(DoorHitTop),
	DEFINE_FUNCTION(DoorHitBottom),
	DEFINE_THINKFUNC(MovingSoundThink),
	DEFINE_THINKFUNC(CloseAreaPortalsThink),
END_DATADESC();
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseDoor::Spawn(void)
{
	Precache();

#ifdef HL1_DLL
	SetSolid(SOLID_BSP);
#else
	if (GetMoveParent() && GetRootMoveParent()->GetSolid() == SOLID_BSP)
	{
		SetSolid(SOLID_BSP);
	}
	else
	{
		SetSolid(SOLID_VPHYSICS);
	}
#endif

	// Convert movedir from angles to a vector
	QAngle angMoveDir = QAngle(m_vecMoveDir.x, m_vecMoveDir.y, m_vecMoveDir.z);
	AngleVectors(angMoveDir, &m_vecMoveDir);

	SetModel(STRING(GetModelName()));
	m_vecPosition1 = GetLocalOrigin();

	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	Vector vecOBB = CollisionProp()->OBBSize();
	vecOBB -= Vector(2, 2, 2);
	m_vecPosition2 = m_vecPosition1 + (m_vecMoveDir * (DotProductAbs(m_vecMoveDir, vecOBB) - m_flLip));

	if (!IsRotatingDoor())
	{
		if ((m_eSpawnPosition == FUNC_DOOR_SPAWN_OPEN) || HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
		{	// swap pos1 and pos2, put door at pos2
			UTIL_SetOrigin(this, m_vecPosition2);
			m_toggle_state = TS_AT_TOP;
		}
		else
		{
			m_toggle_state = TS_AT_BOTTOM;
		}
	}

	if (HasSpawnFlags(SF_DOOR_LOCKED))
	{
		m_bLocked = true;
	}

	SetMoveType(MOVETYPE_PUSH);

	if (m_flSpeed == 0)
	{
		m_flSpeed = 100;
	}

	SetTouch(&CBaseDoor::DoorTouch);

	if (!FClassnameIs(this, "func_water"))
	{
		if (HasSpawnFlags(SF_DOOR_PASSABLE))
		{
			//normal door
			AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);
			AddSolidFlags(FSOLID_NOT_SOLID);
		}

		if (HasSpawnFlags(SF_DOOR_NONSOLID_TO_PLAYER))
		{
			SetCollisionGroup(COLLISION_GROUP_PASSABLE_DOOR);
			// HACKHACK: Set this hoping that any children of the door that get blocked by the player
			// will get fixed up by vphysics
			// NOTE: We could decouple this as a separate behavior, but managing player collisions is already complex enough.
			// NOTE: This is necessary to prevent the player from blocking the wrecked train car in ep2_outland_01
			AddFlag(FL_UNBLOCKABLE_BY_PLAYER);
		}
		if (m_bIgnoreDebris)
		{
			// both of these flags want to set the collision group and 
			// there isn't a combo group
			Assert(!HasSpawnFlags(SF_DOOR_NONSOLID_TO_PLAYER));
			if (HasSpawnFlags(SF_DOOR_NONSOLID_TO_PLAYER))
			{
				Warning("Door %s with conflicting collision settings, removing ignoredebris\n", GetDebugName());
			}
			else
			{
				SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);
			}
		}
	}

	if ((m_eSpawnPosition == FUNC_DOOR_SPAWN_OPEN) && HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
	{
		Warning("Door %s using obsolete 'Start Open' spawnflag with 'Spawn Position' set to 'Open'. Reverting to old behavior.\n", GetDebugName());
	}

	CreateVPhysics();

	m_bIgnoreNonPlayerEntsOnBlock = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseDoor::Activate(void)
{
	BaseClass::Activate();

	CBaseDoor *pDoorList[64];
	m_bDoorGroup = true;

	// force movement groups to sync!!!
	int doorCount = GetDoorMovementGroup(pDoorList, ARRAYSIZE(pDoorList));
	for (int i = 0; i < doorCount; i++)
	{
		if (pDoorList[i]->m_vecMoveDir == m_vecMoveDir)
		{
			bool error = false;
			if (pDoorList[i]->IsRotatingDoor())
			{
				error = (pDoorList[i]->GetLocalAngles() != GetLocalAngles()) ? true : false;
			}
			else
			{
				error = (pDoorList[i]->GetLocalOrigin() != GetLocalOrigin()) ? true : false;
			}
			if (error)
			{
				// don't do group blocking
				m_bDoorGroup = false;
#ifdef HL1_DLL
				// UNDONE: This should probably fixup m_vecPosition1 & m_vecPosition2
				Warning("Door group %s has misaligned origin!\n", STRING(GetEntityName()));
#endif
			}
		}
	}

	switch (m_toggle_state)
	{
	case TS_AT_TOP:
		UpdateAreaPortals(true);
		break;
	case TS_AT_BOTTOM:
		UpdateAreaPortals(false);
		break;
	}

#ifdef HL1_DLL
	// Get a handle to my filter entity if there is one
	if (m_iBlockFilterName != NULL_STRING)
	{
		m_hBlockFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName(NULL, m_iBlockFilterName, NULL));
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called when the player uses the door.
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CBaseDoor::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_hActivator = pActivator;

	if (m_ChainTarget != NULL_STRING)
		ChainUse();

	// We can't +use this if it can't be +used
	if (m_hActivator != NULL && m_hActivator->IsPlayer() && HasSpawnFlags(SF_DOOR_PUSE) == false)
	{
#ifdef GAME_DLL
		PlayLockSounds(this, &m_ls, TRUE, FALSE);
#endif
		return;
	}

	bool bAllowUse = false;

	// if not ready to be used, ignore "use" command.
	if (HasSpawnFlags(SF_DOOR_NEW_USE_RULES))
	{
		//New behavior:
		// If not ready to be used, ignore "use" command.
		// Allow use in these cases:
		//		- when the door is closed/closing
		//		- when the door is open/opening and can be manually closed
		if ((m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN) || (HasSpawnFlags(SF_DOOR_NO_AUTO_RETURN) && (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP)))
			bAllowUse = true;
	}
	else
	{
		// Legacy behavior:
		if (m_toggle_state == TS_AT_BOTTOM || (HasSpawnFlags(SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP))
			bAllowUse = true;
	}

	if (bAllowUse)
	{
		if (m_bLocked)
		{
			m_OnLockedUse.FireOutput(pActivator, pCaller);
#ifdef GAME_DLL 
			PlayLockSounds(this, &m_ls, TRUE, FALSE);
#endif
		}
		else
		{
			DoorActivate();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called the first frame that the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
void CBaseDoor::StartBlocked(CBaseEntity *pOther)
{
	//
	// Fire whatever events we need to due to our blocked state.
	//
	if (m_toggle_state == TS_GOING_DOWN)
	{
		m_OnBlockedClosing.FireOutput(pOther, this);
	}
	else
	{
		m_OnBlockedOpening.FireOutput(pOther, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame when the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
void CBaseDoor::Blocked(CBaseEntity *pOther)
{
	// Damage is not a thing in momentum
	/* Hurt the blocker a little.
	if (m_flBlockDamage)
	{
		// if the door is marked "force closed" or it has a negative wait, then there's nothing to do but 
		// push/damage the object.
		// If block damage is set, but this object is a physics prop that can't be damaged, just
		// give up and disable collisions
		if ((m_bForceClosed || m_flWait < 0) && pOther->GetMoveType() == MOVETYPE_VPHYSICS &&
			(pOther->m_takedamage == DAMAGE_NO || pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
		{
			EntityPhysics_CreateSolver(this, pOther, true, 4.0f);
		}
		else
		{
			pOther->TakeDamage(CTakeDamageInfo(this, this, m_flBlockDamage, DMG_CRUSH));
		}
	}
	// If set, ignore non-player ents that block us.  Mainly of use in multiplayer to prevent exploits.
	else if (pOther && !pOther->IsPlayer() && m_bIgnoreNonPlayerEntsOnBlock)
	{
		return;
	}*/

	// If we're set to force ourselves closed, keep going
	if (m_bForceClosed)
		return;

	// if a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast
	if (m_flWait >= 0)
	{
		if (m_toggle_state == TS_GOING_DOWN)
		{
			DoorGoUp();
		}
		else
		{
			DoorGoDown();
		}
	}

	// Block all door pieces with the same targetname here.
	if (GetDebugName() != nullptr)
	{
		CBaseDoor *pDoorList[64];
		int doorCount = GetDoorMovementGroup(pDoorList, ARRAYSIZE(pDoorList));

		for (int i = 0; i < doorCount; i++)
		{
			CBaseDoor *pDoor = pDoorList[i];

			if (pDoor->m_flWait >= 0)
			{
				if (m_bDoorGroup && pDoor->m_vecMoveDir == m_vecMoveDir && pDoor->GetAbsVelocity() == GetAbsVelocity() && pDoor->GetLocalAngularVelocity() == GetLocalAngularVelocity())
				{
					pDoor->m_nSimulationTick = m_nSimulationTick;	// don't run simulation this frame if you haven't run yet

																	// this is the most hacked, evil, bastardized thing I've ever seen. kjb
					if (!pDoor->IsRotatingDoor())
					{// set origin to realign normal doors
						pDoor->SetLocalOrigin(GetLocalOrigin());
						pDoor->SetAbsVelocity(vec3_origin);// stop!

					}
					else
					{// set angles to realign rotating doors
						pDoor->SetLocalAngles(GetLocalAngles());
						pDoor->SetLocalAngularVelocity(vec3_angle);
					}
				}

				if (pDoor->m_toggle_state == TS_GOING_DOWN)
					pDoor->DoorGoUp();
				else
					pDoor->DoorGoDown();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called the first frame that the door is unblocked while opening or closing.
//-----------------------------------------------------------------------------
void CBaseDoor::EndBlocked(void)
{
	//
	// Fire whatever events we need to due to our unblocked state.
	//
	if (m_toggle_state == TS_GOING_DOWN)
	{
		m_OnUnblockedClosing.FireOutput(this, this);
	}
	else
	{
		m_OnUnblockedOpening.FireOutput(this, this);
	}
}

//-----------------------------------------------------------------------------
bool CBaseDoor::CreateVPhysics(void)
{
	if (!FClassnameIs(this, "func_water"))
	{
		//normal door
		// NOTE: Create this even when the door is not solid to support constraints.
		VPhysicsInitShadow(false, false);
	}
	else
	{
		// special contents
		AddSolidFlags(FSOLID_VOLUME_CONTENTS);
		SETBITS(m_spawnflags, SF_DOOR_SILENT);	// water is silent for now

		IPhysicsObject *pPhysics = VPhysicsInitShadow(false, false);
		fluidparams_t fluid;

		Assert(CollisionProp()->GetCollisionAngles() == vec3_angle);
		fluid.damping = 0.01f;
		fluid.surfacePlane[0] = 0;
		fluid.surfacePlane[1] = 0;
		fluid.surfacePlane[2] = 1;
		fluid.surfacePlane[3] = CollisionProp()->GetCollisionOrigin().z + CollisionProp()->OBBMaxs().z - 1;
		fluid.currentVelocity.Init(0, 0, 0);
		fluid.torqueFactor = 0.1f;
		fluid.viscosityFactor = 0.01f;
		fluid.pGameData = static_cast<void *>(this);

		//FIXME: Currently there's no way to specify that you want slime
		fluid.contents = CONTENTS_WATER;

		physenv->CreateFluidController(pPhysics, &fluid);
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
// This is ONLY used by the node graph to test movement through a door
void CBaseDoor::InputSetToggleState(inputdata_t &inputdata)
{
	SetToggleState(inputdata.value.Int());
}

void CBaseDoor::SetToggleState(int state)
{
	if (state == TS_AT_TOP)
		UTIL_SetOrigin(this, m_vecPosition2);
	else
		UTIL_SetOrigin(this, m_vecPosition1);
}

bool CBaseDoor::ShouldSavePhysics()
{
	// don't save physics if you're func_water
	return !FClassnameIs(this, "func_water");
}

//-----------------------------------------------------------------------------
// Purpose: Doors not tied to anything (e.g. button, another door) can be touched,
//			to make them activate.
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBaseDoor::DoorTouch(CBaseEntity *pOther)
{
	if (m_ChainTarget != NULL_STRING)
		ChainTouch(pOther);

	// Ignore touches by anything but players.
	if (!pOther->IsPlayer())
	{
		return;
	}

	// If door is not opened by touch, do nothing.
	if (!HasSpawnFlags(SF_DOOR_PTOUCH))
	{
		return;
	}

#ifdef GAME_DLL
	// If door has master, and it's not ready to trigger, 
	// play 'locked' sound.
	if (m_sMaster != NULL_STRING && !UTIL_IsMasterTriggered(m_sMaster, pOther))
	{
		PlayLockSounds(this, &m_ls, TRUE, FALSE);
	}
#endif
	if (m_bLocked)
	{
		m_OnLockedUse.FireOutput(pOther, pOther);
#ifdef GAME_DLL
		PlayLockSounds(this, &m_ls, TRUE, FALSE);
#endif
		return;
	}

	// Remember who activated the door.
	m_hActivator = pOther;

	if (DoorActivate())
	{
		// Temporarily disable the touch function, until movement is finished.
		SetTouch(NULL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Causes the door to "do its thing", i.e. start moving, and cascade activation.
// Output : int
//-----------------------------------------------------------------------------
int CBaseDoor::DoorActivate(void)
{
	if (!UTIL_IsMasterCRCTriggered(m_iMasterCRC, m_hActivator))
		return 0;

	if (HasSpawnFlags(SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP)
	{
		// door should close
		DoorGoDown();
	}
	else
	{
		// door should open
#ifdef GAME_DLL
		// play door unlock sounds
		PlayLockSounds(this, &m_ls, FALSE, FALSE);
#endif

		if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_GOING_UP)
		{
			DoorGoUp();
		}
	}

	return 1;
}


//-----------------------------------------------------------------------------
// Purpose: Starts the door going to its "up" position (simply ToggleData->vecPosition2).
//-----------------------------------------------------------------------------
void CBaseDoor::DoorGoUp(void)
{
	UpdateAreaPortals(true);
#ifdef GAME_DLL
	// It could be going-down, if blocked.
	ASSERT(m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN);
#endif
	// emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		// If we're not moving already, start the moving noise
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
		{
			StartMovingSound();
		}
	}

	m_toggle_state = TS_GOING_UP;

	SetMoveDone(&CBaseDoor::DoorHitTop);
	if (IsRotatingDoor())		// !!! BUGBUG Triggered doors don't work with this yet
	{
		float	sign = 1.0;

		if (m_hActivator != NULL)
		{
			if (!HasSpawnFlags(SF_DOOR_ONEWAY) && m_vecMoveAng.y) 		// Y axis rotation, move away from the player
			{
				// Positive is CCW, negative is CW, so make 'sign' 1 or -1 based on which way we want to open.
				// Important note:  All doors face East at all times, and twist their local angle to open.
				//					So you can't look at the door's facing to determine which way to open.

				Vector nearestPoint;
				CollisionProp()->CalcNearestPoint(m_hActivator->GetAbsOrigin(), &nearestPoint);
				Vector activatorToNearestPoint = nearestPoint - m_hActivator->GetAbsOrigin();
				activatorToNearestPoint.z = 0;

				Vector activatorToOrigin = GetAbsOrigin() - m_hActivator->GetAbsOrigin();
				activatorToOrigin.z = 0;

				// Point right hand at door hinge, curl hand towards closest spot on door, if thumb
				// is up, open door CW.  -- Department of Basic Cross Product Understanding for Noobs
				Vector cross = activatorToOrigin.Cross(activatorToNearestPoint);

				if (cross.z > 0.0f)
				{
					sign = -1.0f;
				}
			}
		}
		AngularMove(m_vecAngle2*sign, m_flSpeed);
	}
	else
	{
		LinearMove(m_vecPosition2, m_flSpeed);
	}

	//Fire our open ouput
	m_OnOpen.FireOutput(this, this);
}

//-----------------------------------------------------------------------------
// Purpose: Starts the door going to its "down" position (simply ToggleData->vecPosition1).
//-----------------------------------------------------------------------------
void CBaseDoor::DoorGoDown(void)
{
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		// If we're not moving already, start the moving noise
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
		{
			StartMovingSound();
		}
	}

#ifdef DOOR_ASSERT
	ASSERT(m_toggle_state == TS_AT_TOP);
#endif // DOOR_ASSERT
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseDoor::DoorHitBottom);
	if (IsRotatingDoor())//rotating door
		AngularMove(m_vecAngle1, m_flSpeed);
	else
		LinearMove(m_vecPosition1, m_flSpeed);

	//Fire our closed output
	m_OnClose.FireOutput(this, this);
}


//-----------------------------------------------------------------------------
// Purpose: The door has reached the "up" position.  Either go back down, or
//			wait for another activation.
//-----------------------------------------------------------------------------
void CBaseDoor::DoorHitTop(void)
{
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		CPASAttenuationFilter filter(this);
		filter.MakeReliable();
		StopMovingSound();

		EmitSound_t ep;
		ep.m_nChannel = CHAN_STATIC;
		ep.m_pSoundName = (char*)STRING(m_NoiseArrived);
		ep.m_flVolume = 1;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}

#ifdef GAME_DLL
	ASSERT(m_toggle_state == TS_GOING_UP);
#endif
	m_toggle_state = TS_AT_TOP;

	// toggle-doors don't come down automatically, they wait for refire.
	if (HasSpawnFlags(SF_DOOR_NO_AUTO_RETURN))
	{
		// Re-instate touch method, movement is complete
		SetTouch(&CBaseDoor::DoorTouch);
	}
	else
	{
		// In flWait seconds, DoorGoDown will fire, unless wait is -1, then door stays open
		SetMoveDoneTime(m_flWait);
		SetMoveDone(&CBaseDoor::DoorGoDown);

		if (m_flWait == -1)
		{
			SetNextThink(TICK_NEVER_THINK);
		}
	}

	if (HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
	{
		m_OnFullyClosed.FireOutput(this, this);
	}
	else
	{
		m_OnFullyOpen.FireOutput(this, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: The door has reached the "down" position.  Back to quiescence.
//-----------------------------------------------------------------------------
void CBaseDoor::DoorHitBottom(void)
{
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		CPASAttenuationFilter filter(this);
		filter.MakeReliable();

		StopMovingSound();

		EmitSound_t ep;
		ep.m_nChannel = CHAN_STATIC;
		if (m_NoiseArrivedClosed == NULL_STRING)
			ep.m_pSoundName = (char*)STRING(m_NoiseArrived);
		else
			ep.m_pSoundName = (char*)STRING(m_NoiseArrivedClosed);
		ep.m_flVolume = 1;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}

#ifdef GAME_DLL
	ASSERT(m_toggle_state == TS_GOING_DOWN);
#endif
	m_toggle_state = TS_AT_BOTTOM;

	// Re-instate touch method, cycle is complete
	SetTouch(&CBaseDoor::DoorTouch);

	if (HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
	{
		m_OnFullyOpen.FireOutput(m_hActivator, this);
	}
	else
	{
		m_OnFullyClosed.FireOutput(m_hActivator, this);
	}

	// Close the area portals just after the door closes, to prevent visual artifacts in multiplayer games
	SetContextThink(&CBaseDoor::CloseAreaPortalsThink, gpGlobals->curtime + 0.5f, CLOSE_AREAPORTAL_THINK_CONTEXT);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : isOpen - 
//-----------------------------------------------------------------------------
void CBaseDoor::UpdateAreaPortals(bool isOpen)
{
	// cancel pending close
	SetContextThink(NULL, gpGlobals->curtime, CLOSE_AREAPORTAL_THINK_CONTEXT);

	if (IsRotatingDoor() && HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE)) // logic inverted when using rot doors that start open
		isOpen = !isOpen;

	const char* name = GetDebugName();
	if (!name)
		return;

#ifdef GAME_DLL // MOM_TODO: Does the client want this stuff?
	CBaseEntity *pPortal = NULL;
	while ((pPortal = gEntList.FindEntityByClassname(pPortal, "func_areaportal")) != NULL)
	{
		if (pPortal->HasTarget(MAKE_STRING(name)))
		{
			// USE_ON means open the portal, off means close it
			pPortal->Use(this, this, isOpen ? USE_ON : USE_OFF, 0);
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Unlocks the door so that it can be opened.
//-----------------------------------------------------------------------------
void CBaseDoor::Unlock(void)
{
	m_bLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: Locks the door so that it cannot be opened.
//-----------------------------------------------------------------------------
void CBaseDoor::Lock(void)
{
	m_bLocked = true;
}

// Lists all doors in the same movement group as this one
int CBaseDoor::GetDoorMovementGroup(CBaseDoor *pDoorList[], int listMax)
{
	int count = 0;
	CBaseEntity	*pTarget = NULL;

	// Block all door pieces with the same targetname here.
	if (GetDebugName() != nullptr)
	{
		for (;;)
		{
			pTarget = FindEntityByNameCRC(pTarget, GetNameCRC());

			if (pTarget != this)
			{
				if (!pTarget)
					break;

				CBaseDoor *pDoor = dynamic_cast<CBaseDoor *>(pTarget);

				if (pDoor && count < listMax)
				{
					pDoorList[count] = pDoor;
					count++;
				}
			}
		}
	}
	return count;
}

//-----------------------------------------------------------------------------
// Purpose: Closes the door if it is not already closed.
//-----------------------------------------------------------------------------
void CBaseDoor::InputClose(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_BOTTOM)
	{
		DoorGoDown();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler that locks the door.
//-----------------------------------------------------------------------------
void CBaseDoor::InputLock(inputdata_t &inputdata)
{
	Lock();
}

//-----------------------------------------------------------------------------
// Purpose: Opens the door if it is not already open.
//-----------------------------------------------------------------------------
void CBaseDoor::InputOpen(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_GOING_UP)
	{
		// I'm locked, can't open
		if (m_bLocked)
			return;

#ifdef GAME_DLL
		// Play door unlock sounds.
		PlayLockSounds(this, &m_ls, false, false);
#endif
		DoorGoUp();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens the door if it is not already open.
//-----------------------------------------------------------------------------
void CBaseDoor::InputToggle(inputdata_t &inputdata)
{
	// I'm locked, can't open
	if (m_bLocked)
		return;

	if (m_toggle_state == TS_AT_BOTTOM)
	{
		DoorGoUp();
	}
	else if (m_toggle_state == TS_AT_TOP)
	{
		DoorGoDown();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler that unlocks the door.
//-----------------------------------------------------------------------------
void CBaseDoor::InputUnlock(inputdata_t &inputdata)
{
	Unlock();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseDoor::InputSetSpeed(inputdata_t &inputdata)
{
	m_flSpeed = inputdata.value.Float();
}

void CBaseDoor::StartMovingSound(void)
{
	MovingSoundThink();

#ifdef CSTRIKE_DLL // this event is only used by CS:S bots

	CBasePlayer *player = ToBasePlayer(m_hActivator);
	IGameEvent * event = gameeventmanager->CreateEvent("door_moving");
	if (event)
	{
		event->SetInt("entindex", entindex());
		event->SetInt("userid", (player) ? player->GetUserID() : 0);
		gameeventmanager->FireEvent(event);
	}

#endif
}

void CBaseDoor::StopMovingSound(void)
{
	SetContextThink(NULL, gpGlobals->curtime, "MovingSound");
	char *pSoundName;
	if (m_NoiseMovingClosed == NULL_STRING || m_toggle_state == TS_GOING_UP || m_toggle_state == TS_AT_TOP)
	{
		pSoundName = (char*)STRING(m_NoiseMoving);
	}
	else
	{
		pSoundName = (char*)STRING(m_NoiseMovingClosed);
	}
	StopSound(entindex(), CHAN_STATIC, pSoundName);
}

void CBaseDoor::MovingSoundThink(void)
{
	CPASAttenuationFilter filter(this);
	filter.MakeReliable();

	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	if (m_NoiseMovingClosed == NULL_STRING || m_toggle_state == TS_GOING_DOWN || m_toggle_state == TS_AT_BOTTOM)
	{
		ep.m_pSoundName = (char*)STRING(m_NoiseMoving);
	}
	else
	{
		ep.m_pSoundName = (char*)STRING(m_NoiseMovingClosed);
	}
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;

	EmitSound(filter, entindex(), ep);

	//Only loop sounds in HL1 to maintain HL2 behavior
	if (ShouldLoopMoveSound())
	{
		float duration = enginesound->GetSoundDuration(ep.m_pSoundName);
		SetContextThink(&CBaseDoor::MovingSoundThink, gpGlobals->curtime + duration, "MovingSound");
	}
}

#ifdef HL1_DLL
bool CBaseDoor::PassesBlockTouchFilter(CBaseEntity *pOther)
{
	CBaseFilter* pFilter = (CBaseFilter*)(m_hBlockFilter.Get());
	return (pFilter && pFilter->PassesFilter(this, pOther));
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Passes Use along to certain named doors.
//-----------------------------------------------------------------------------
void CBaseDoor::ChainUse(void)
{
	if (m_isChaining)
		return;

	CBaseEntity *ent = NULL;
	while ((ent = FindEntityByNameCRC(ent, m_iChainTargetCRC)) != NULL)
	{
		if (ent == this)
			continue;

		CBaseDoor *door = dynamic_cast<CBaseDoor *>(ent);
		if (door)
		{
			door->SetChaining(true);
			door->Use(m_hActivator, NULL, USE_TOGGLE, 0.0f); // only the first param is used
			door->SetChaining(false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Passes Touch along to certain named doors.
//-----------------------------------------------------------------------------
void CBaseDoor::ChainTouch(CBaseEntity *pOther)
{
	if (m_isChaining)
		return;

	CBaseEntity *ent = NULL;
	while ((ent = FindEntityByNameCRC(ent, m_iChainTargetCRC)) != NULL)
	{
		if (ent == this)
			continue;

		CBaseDoor *door = dynamic_cast<CBaseDoor *>(ent);
		if (door)
		{
			door->SetChaining(true);
			door->Touch(pOther);
			door->SetChaining(false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Delays turning off area portals when closing doors to prevent visual artifacts
//-----------------------------------------------------------------------------
void CBaseDoor::CloseAreaPortalsThink(void)
{
	UpdateAreaPortals(false);
	SetContextThink(NULL, gpGlobals->curtime, CLOSE_AREAPORTAL_THINK_CONTEXT);
}

#ifdef GAME_DLL
int CBaseDoor::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseDoor::Precache(void)
{
	//Fill in a default value if necessary
	if (IsRotatingDoor())
	{
		UTIL_ValidateSoundName(m_NoiseMoving, "RotDoorSound.DefaultMove");
		UTIL_ValidateSoundName(m_NoiseArrived, "RotDoorSound.DefaultArrive");
		UTIL_ValidateSoundName(m_ls.sLockedSound, "RotDoorSound.DefaultLocked");
		UTIL_ValidateSoundName(m_ls.sUnlockedSound, "DoorSound.Null");
	}
	else
	{
		UTIL_ValidateSoundName(m_NoiseMoving, "DoorSound.DefaultMove");
		UTIL_ValidateSoundName(m_NoiseArrived, "DoorSound.DefaultArrive");
#ifndef HL1_DLL		
		UTIL_ValidateSoundName(m_ls.sLockedSound, "DoorSound.DefaultLocked");
#endif
		UTIL_ValidateSoundName(m_ls.sUnlockedSound, "DoorSound.Null");
	}

#ifdef HL1_DLL
	if (m_ls.sLockedSound != NULL_STRING && strlen((char*)STRING(m_ls.sLockedSound)) < 4)
	{
		// Too short to be ANYTHING ".wav", so it must be an old index into a long-lost
		// array of sound choices. slam it to a known "deny" sound. We lose the designer's
		// original selection, but we don't get unresponsive doors.
		m_ls.sLockedSound = AllocPooledString("buttons/button2.wav");
	}
#endif//HL1_DLL

	//Precache them all
	PrecacheScriptSound((char *)STRING(m_NoiseMoving));
	PrecacheScriptSound((char *)STRING(m_NoiseArrived));
	PrecacheScriptSound((char *)STRING(m_NoiseMovingClosed));
	PrecacheScriptSound((char *)STRING(m_NoiseArrivedClosed));
	PrecacheScriptSound((char *)STRING(m_ls.sLockedSound));
	PrecacheScriptSound((char *)STRING(m_ls.sUnlockedSound));

	//Get sentence group names, for doors which are directly 'touched' to open
	switch (m_bLockedSentence)
	{
	case 1: m_ls.sLockedSentence = AllocPooledString("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = AllocPooledString("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = AllocPooledString("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = AllocPooledString("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = AllocPooledString("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = AllocPooledString("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = AllocPooledString("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = AllocPooledString("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = AllocPooledString("NG"); break; // broken door

	default: m_ls.sLockedSentence = NULL_STRING; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = AllocPooledString("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = AllocPooledString("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = AllocPooledString("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = AllocPooledString("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = AllocPooledString("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = AllocPooledString("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = AllocPooledString("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = AllocPooledString("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = NULL_STRING; break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cache user-entity-field values until spawn is called.
// Input  : szKeyName - 
//			szValue - 
// Output : Returns true.
//-----------------------------------------------------------------------------
bool CBaseDoor::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "locked_sentence"))
	{
		m_bLockedSentence = atof(szValue);
	}
	else if (FStrEq(szKeyName, "unlocked_sentence"))
	{
		m_bUnlockedSentence = atof(szValue);
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}
#endif