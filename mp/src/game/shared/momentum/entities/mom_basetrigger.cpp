#include "cbase.h"
#include "mom_basetrigger.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar showtriggers( "showtriggers", "0", FCVAR_CHEAT, "Shows trigger brushes" );

LINK_ENTITY_TO_CLASS( trigger, CBaseTrigger );

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CBaseTrigger) // MOM_TODO: Add _NO_BASE stuff to predict here
	DEFINE_PRED_FIELD(m_bDisabled, FIELD_BOOL, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iTargetCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iFilterCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

#undef CBaseTrigger // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_BaseTrigger, DT_BaseTrigger, CBaseTrigger) // MOM_TODO: Add _NO_BASE stuff to predict here
	RecvPropBool(RECVINFO(m_bDisabled)),
	RecvPropInt(RECVINFO(m_iTargetCRC)),
	RecvPropInt(RECVINFO(m_iFilterCRC)),
END_RECV_TABLE();
#define CBaseTrigger C_BaseTrigger // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CBaseTrigger, DT_BaseTrigger) // MOM_TODO: Add _NO_BASE stuff to predict here
	SendPropBool(SENDINFO(m_bDisabled)),
	SendPropInt(SENDINFO(m_iTargetCRC)),
	SendPropInt(SENDINFO(m_iFilterCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CBaseTrigger)
	// Keyfields
	DEFINE_KEYFIELD(m_iFilterName, FIELD_STRING, "filtername"),
	DEFINE_FIELD(m_hFilter, FIELD_EHANDLE),
	DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),
	DEFINE_UTLVECTOR(m_hTouchingEntities, FIELD_EHANDLE),

	// Inputs	
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
	DEFINE_INPUTFUNC(FIELD_VOID, "TouchTest", InputTouchTest),

	DEFINE_INPUTFUNC(FIELD_VOID, "StartTouch", InputStartTouch),
	DEFINE_INPUTFUNC(FIELD_VOID, "EndTouch", InputEndTouch),

	// Outputs
	DEFINE_OUTPUT(m_OnStartTouch, "OnStartTouch"),
	DEFINE_OUTPUT(m_OnStartTouchAll, "OnStartTouchAll"),
	DEFINE_OUTPUT(m_OnEndTouch, "OnEndTouch"),
	DEFINE_OUTPUT(m_OnEndTouchAll, "OnEndTouchAll"),
	DEFINE_OUTPUT(m_OnTouching, "OnTouching"),
	DEFINE_OUTPUT(m_OnNotTouching, "OnNotTouching"),
END_DATADESC();
#endif
CBaseTrigger::CBaseTrigger()
{
	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CBaseTrigger::Spawn()
{
	CRC32_t crc;
	if (Q_strlen(STRING(m_target)))
	{
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, STRING(m_target), Q_strlen(STRING(m_target)));
		CRC32_Final(&crc);

		m_iTargetCRC = crc;
	}

	if (Q_strlen(STRING(m_iFilterName)))
	{
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, STRING(m_iFilterName), Q_strlen(STRING(m_iFilterName)));
		CRC32_Final(&crc);

		m_iFilterCRC = crc;
	}

	if (HasSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS) || HasSpawnFlags(SF_TRIGGER_ONLY_NPCS_IN_VEHICLES))
	{
		// Automatically set this trigger to work with NPC's.
		AddSpawnFlags(SF_TRIGGER_ALLOW_NPCS);
	}

	if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES))
	{
		AddSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS);
	}

	if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES))
	{
		AddSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS);
	}
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CBaseTrigger::Activate(void)
{
	// Get a handle to my filter entity if there is one
	if (m_iFilterName != NULL_STRING)
	{
		m_hFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName(NULL, m_iFilterName));
	}

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Called after player becomes active in the game
//-----------------------------------------------------------------------------
void CBaseTrigger::PostClientActive(void)
{
	BaseClass::PostClientActive();

	if (!m_bDisabled)
	{
		PhysicsTouchTriggers();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTrigger::InitTrigger()
{
	SetSolid(GetParent() ? SOLID_VPHYSICS : SOLID_BSP);
	AddSolidFlags(FSOLID_NOT_SOLID);
	if (m_bDisabled)
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}

	SetMoveType(MOVETYPE_NONE);
	SetModel(STRING(GetModelName()));    // set size and link into world
	if (showtriggers.GetInt() == 0)
	{
		AddEffects(EF_NODRAW);
	}

	m_hTouchingEntities.Purge();

	if (HasSpawnFlags(SF_TRIG_TOUCH_DEBRIS))
	{
		CollisionProp()->AddSolidFlags(FSOLID_TRIGGER_TOUCH_DEBRIS);
	}
}

//------------------------------------------------------------------------------
// Purpose: Turns on this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::Enable(void)
{
	m_bDisabled = false;

	if (VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions(true);
	}

	if (!IsSolidFlagSet(FSOLID_TRIGGER))
	{
		AddSolidFlags(FSOLID_TRIGGER);
		PhysicsTouchTriggers();
	}
}

//------------------------------------------------------------------------------
// Purpose: Turns off this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::Disable(void)
{
	m_bDisabled = true;

	if (VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions(false);
	}

	if (IsSolidFlagSet(FSOLID_TRIGGER))
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
		PhysicsTouchTriggers();
	}
}

//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------
void CBaseTrigger::UpdateOnRemove(void)
{
	if (VPhysicsGetObject())
	{
		VPhysicsGetObject()->RemoveTrigger();
	}

	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
// Purpose: Tests to see if anything is touching this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::TouchTest(void)
{
	// If the trigger is disabled don't test to see if anything is touching it.
	if (!m_bDisabled)
	{
		if (m_hTouchingEntities.Count() != 0)
		{

			m_OnTouching.FireOutput(this, this);
		}
		else
		{
			m_OnNotTouching.FireOutput(this, this);
		}
	}
}

//------------------------------------------------------------------------------
// Purpose: Input handler to turn on this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::InputEnable(inputdata_t &inputdata)
{
	Enable();
}

//------------------------------------------------------------------------------
// Purpose: Input handler to turn off this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::InputDisable(inputdata_t &inputdata)
{
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: Toggles this trigger between enabled and disabled.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputToggle(inputdata_t &inputdata)
{
	if (IsSolidFlagSet(FSOLID_TRIGGER))
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}

	PhysicsTouchTriggers();
}

void CBaseTrigger::InputTouchTest(inputdata_t &inputdata)
{
	TouchTest();
}

//-----------------------------------------------------------------------------
// Purpose: Called to simulate what happens when an entity touches the trigger.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputStartTouch(inputdata_t &inputdata)
{
	//Pretend we just touched the trigger.
	StartTouch(inputdata.pCaller);
}

//-----------------------------------------------------------------------------
// Purpose: Called to simulate what happens when an entity leaves the trigger.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputEndTouch(inputdata_t &inputdata)
{
	//And... pretend we left the trigger.
	EndTouch(inputdata.pCaller);
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this entity passes the filter criteria, false if not.
// Input  : pOther - The entity to be filtered.
//-----------------------------------------------------------------------------
bool CBaseTrigger::PassesTriggerFilters(CBaseEntity *pOther)
{
	// First test spawn flag filters
	if (HasSpawnFlags(SF_TRIGGER_ALLOW_ALL) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS) && (pOther->GetFlags() & FL_CLIENT)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_NPCS) && (pOther->GetFlags() & FL_NPC)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PUSHABLES) && FClassnameIs(pOther, "func_pushable")) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PHYSICS) && pOther->GetMoveType() == MOVETYPE_VPHYSICS)
#if defined( HL2_EPISODIC ) || defined( TF_DLL )		
		||
		(HasSpawnFlags(SF_TRIG_TOUCH_DEBRIS) &&
		(pOther->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			pOther->GetCollisionGroup() == COLLISION_GROUP_DEBRIS_TRIGGER ||
			pOther->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
			)
#endif
		)
	{
		if (pOther->GetFlags() & FL_NPC)
		{
			CAI_BaseNPC *pNPC = pOther->MyNPCPointer();

			if (HasSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS))
			{
				if (!pNPC || !pNPC->IsPlayerAlly())
				{
					return false;
				}
			}

			if (HasSpawnFlags(SF_TRIGGER_ONLY_NPCS_IN_VEHICLES))
			{
				if (!pNPC || !pNPC->IsInAVehicle())
					return false;
			}
		}

		bool bOtherIsPlayer = pOther->IsPlayer();

		if (bOtherIsPlayer)
		{
			CBasePlayer *pPlayer = (CBasePlayer*)pOther;
			if (!pPlayer->IsAlive())
				return false;

			if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES))
			{
				if (!pPlayer->IsInAVehicle())
					return false;

				// Make sure we're also not exiting the vehicle at the moment
				IServerVehicle *pVehicleServer = pPlayer->GetVehicle();
				if (pVehicleServer == NULL)
					return false;

				if (pVehicleServer->IsPassengerExiting())
					return false;
			}

			if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES))
			{
				if (pPlayer->IsInAVehicle())
					return false;
			}

			if (HasSpawnFlags(SF_TRIGGER_DISALLOW_BOTS))
			{
				if (pPlayer->IsFakeClient())
					return false;
			}
		}

		CBaseFilter *pFilter = m_hFilter.Get();
		return (!pFilter) ? true : pFilter->PassesFilter(this, pOther);
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::StartTouch(CBaseEntity *pOther)
{
	if (PassesTriggerFilters(pOther))
	{
		EHANDLE hOther;
		hOther = pOther;

		bool bAdded = false;
		if (m_hTouchingEntities.Find(hOther) == m_hTouchingEntities.InvalidIndex())
		{
			m_hTouchingEntities.AddToTail(hOther);
			bAdded = true;
		}

		m_OnStartTouch.FireOutput(pOther, this);
		OnStartTouch(pOther);

		if (bAdded && (m_hTouchingEntities.Count() == 1))
		{
			// First entity to touch us that passes our filters
			m_OnStartTouchAll.FireOutput(pOther, this);
			StartTouchAll();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::EndTouch(CBaseEntity *pOther)
{
	if (IsTouching(pOther))
	{
		EHANDLE hOther;
		hOther = pOther;
		m_hTouchingEntities.FindAndRemove(hOther);

		//FIXME: Without this, triggers fire their EndTouch outputs when they are disabled!
		//if ( !m_bDisabled )
		//{
		m_OnEndTouch.FireOutput(pOther, this);
		OnEndTouch(pOther);
		//}

		// If there are no more entities touching this trigger, fire the lost all touches
		// Loop through the touching entities backwards. Clean out old ones, and look for existing
		bool bFoundOtherTouchee = false;
		int iSize = m_hTouchingEntities.Count();
		for (int i = iSize - 1; i >= 0; i--)
		{
			EHANDLE hOther;
			hOther = m_hTouchingEntities[i];

			if (!hOther)
			{
				m_hTouchingEntities.Remove(i);
			}
			else if (hOther->IsPlayer() && !hOther->IsAlive())
			{
#ifdef STAGING_ONLY
				if (!HushAsserts())
				{
					AssertMsg(false, "Dead player [%s] is still touching this trigger at [%f %f %f]", hOther->GetEntityName().ToCStr(), XYZ(hOther->GetAbsOrigin()));
				}
				Warning("Dead player [%s] is still touching this trigger at [%f %f %f]", hOther->GetEntityName().ToCStr(), XYZ(hOther->GetAbsOrigin()));
#endif
				m_hTouchingEntities.Remove(i);
			}
			else
			{
				bFoundOtherTouchee = true;
			}
		}

		//FIXME: Without this, triggers fire their EndTouch outputs when they are disabled!
		// Didn't find one?
		if (!bFoundOtherTouchee /*&& !m_bDisabled*/)
		{
			m_OnEndTouchAll.FireOutput(pOther, this);
			EndTouchAll();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified entity is touching us
//-----------------------------------------------------------------------------
bool CBaseTrigger::IsTouching(CBaseEntity *pOther)
{
	EHANDLE hOther;
	hOther = pOther;
	return (m_hTouchingEntities.Find(hOther) != m_hTouchingEntities.InvalidIndex());
}

//-----------------------------------------------------------------------------
// Purpose: Return a pointer to the first entity of the specified type being touched by this trigger
//-----------------------------------------------------------------------------
CBaseEntity *CBaseTrigger::GetTouchedEntityOfType(const char *sClassName)
{
	int iCount = m_hTouchingEntities.Count();
	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity *pEntity = m_hTouchingEntities[i];
		if (FClassnameIs(pEntity, sClassName))
			return pEntity;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified point is within this zone
//-----------------------------------------------------------------------------
bool CBaseTrigger::PointIsWithin(const Vector &vecPoint)
{
	Ray_t ray;
	trace_t tr;
	ICollideable *pCollide = CollisionProp();
	ray.Init(vecPoint, vecPoint);
	enginetrace->ClipRayToCollideable(ray, MASK_ALL, pCollide, &tr);
	return (tr.startsolid);
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CBaseTrigger::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		// --------------
		// Print Target
		// --------------
		char tempstr[255];
		if (IsSolidFlagSet(FSOLID_TRIGGER))
		{
			Q_strncpy(tempstr, "State: Enabled", sizeof(tempstr));
		}
		else
		{
			Q_strncpy(tempstr, "State: Disabled", sizeof(tempstr));
		}
		EntityText(text_offset, tempstr, 0);
		text_offset++;
	}
	return text_offset;
}
#endif