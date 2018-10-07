#include "cbase.h"
#include "mom_basetoggle.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(toggle, CBaseToggle);

#ifdef CLIENT_DLL // Client prediction and recv table
extern void RecvProxy_EffectFlags(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_MoveCollide(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_MoveType(const CRecvProxyData *pData, void *pStruct, void *pOut);

BEGIN_PREDICTION_DATA_NO_BASE(CBaseToggle)
	DEFINE_PRED_TYPEDESCRIPTION(m_Collision, CCollisionProperty),
	DEFINE_PRED_FIELD(m_iMasterCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iNameCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_spawnflags, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_vecNetworkOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_CollisionGroup, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_FIELD(m_iEFlags, FIELD_INTEGER),
END_PREDICTION_DATA();

#undef CBaseToggle // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_BaseToggle, DT_BaseToggle, CBaseToggle)
	RecvPropDataTable(RECVINFO_DT(m_Collision), 0, &REFERENCE_RECV_TABLE(DT_CollisionProperty)),
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
	RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
	RecvPropInt(RECVINFO(m_iMasterCRC)),
	RecvPropInt(RECVINFO(m_iNameCRC)),
	RecvPropInt(RECVINFO(m_nModelIndex)),
	RecvPropInt(RECVINFO(m_CollisionGroup)),
	RecvPropInt(RECVINFO(m_fEffects), 0, RecvProxy_EffectFlags ),
	RecvPropInt(RECVINFO(m_spawnflags)),
	RecvPropInt( "movecollide", 0, SIZEOF_IGNORE, 0, RecvProxy_MoveCollide ),
	RecvPropInt( "movetype", 0, SIZEOF_IGNORE, 0, RecvProxy_MoveType ),
END_RECV_TABLE();
#define CBaseToggle C_BaseToggle // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST_NOBASE(CBaseToggle, DT_BaseToggle)
	SendPropDataTable(SENDINFO_DT(m_Collision), &REFERENCE_SEND_TABLE(DT_CollisionProperty)),
	SendPropVector(SENDINFO(m_vecOrigin), 0,  SPROP_NOSCALE|SPROP_COORD|SPROP_CHANGES_OFTEN ),
	SendPropQAngles(SENDINFO(m_angRotation), 0, SPROP_NOSCALE|SPROP_CHANGES_OFTEN ),
	SendPropInt(SENDINFO(m_fEffects),EF_MAX_BITS, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_iMasterCRC)),
	SendPropInt(SENDINFO(m_iNameCRC)),
	SendPropModelIndex(SENDINFO(m_nModelIndex)),
	SendPropInt(SENDINFO(m_CollisionGroup), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_spawnflags)),
	SendPropInt(SENDINFO_NAME(m_MoveCollide, movecollide), MOVECOLLIDE_MAX_BITS, SPROP_UNSIGNED),
END_SEND_TABLE();

BEGIN_DATADESC(CBaseToggle)
	DEFINE_FIELD(m_toggle_state, FIELD_INTEGER),
	DEFINE_FIELD(m_flMoveDistance, FIELD_FLOAT),
	DEFINE_FIELD(m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(m_flLip, FIELD_FLOAT),
	DEFINE_FIELD(m_vecPosition1, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(m_vecPosition2, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(m_vecMoveAng, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(m_vecAngle1, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(m_vecAngle2, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(m_flHeight, FIELD_FLOAT),
	DEFINE_FIELD(m_hActivator, FIELD_EHANDLE),
	DEFINE_FIELD(m_vecFinalDest, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(m_vecFinalAngle, FIELD_VECTOR),
	DEFINE_FIELD(m_sMaster, FIELD_STRING),
	DEFINE_FIELD(m_movementType, FIELD_INTEGER),	// Linear or angular movement? (togglemovetypes_t)
END_DATADESC();
#endif

CBaseToggle::CBaseToggle()
{
#ifdef _DEBUG
	// necessary since in debug, we initialize vectors to NAN for debugging
	m_vecPosition1.Init();
	m_vecPosition2.Init();
	m_vecAngle1.Init();
	m_vecAngle2.Init();
	m_vecFinalDest.Init();
	m_vecFinalAngle.Init();
#endif
}

bool CBaseToggle::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "lip"))
	{
		m_flLip = atof(szValue);
	}
	else if (FStrEq(szKeyName, "wait"))
	{
		m_flWait = atof(szValue);
	}
	else if (FStrEq(szKeyName, "master"))
	{
		CRC32_t crc; 
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, szKeyName, Q_strlen(szKeyName));
		CRC32_Final(&crc);

		m_iMasterCRC = crc;
		m_sMaster = AllocPooledString(szValue);
	}
	else if (FStrEq(szKeyName, "distance"))
	{
		m_flMoveDistance = atof(szValue);
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate m_vecVelocity and m_flNextThink to reach vecDest from
//			GetOrigin() traveling at flSpeed.
// Input  : Vector	vecDest - 
//			flSpeed - 
//-----------------------------------------------------------------------------
void CBaseToggle::LinearMove(const Vector &vecDest, float flSpeed)
{
#ifdef GAME_DLL
	ASSERTSZ(flSpeed != 0.0f, "LinearMove:  no speed is defined!");
#endif

	m_vecFinalDest = vecDest;

	m_movementType = MOVE_TOGGLE_LINEAR;
	// Already there?
	if (vecDest == GetLocalOrigin())
	{
		MoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - GetLocalOrigin();

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set m_flNextThink to trigger a call to LinearMoveDone when dest is reached
	SetMoveDoneTime(flTravelTime);

	// scale the destdelta vector by the time spent traveling to get velocity
	SetLocalVelocity(vecDestDelta / flTravelTime);
}

//-----------------------------------------------------------------------------
// Purpose: After moving, set origin to exact final destination, call "move done" function.
//-----------------------------------------------------------------------------
void CBaseToggle::LinearMoveDone(void)
{
	UTIL_SetOrigin(this, m_vecFinalDest);
	SetAbsVelocity(vec3_origin);
	SetMoveDoneTime(-1.f);
}

//-----------------------------------------------------------------------------
// Purpose: Calculate m_vecVelocity and m_flNextThink to reach vecDest from
//			GetLocalOrigin() traveling at flSpeed. Just like LinearMove, but rotational.
// Input  : vecDestAngle - 
//			flSpeed - 
//-----------------------------------------------------------------------------
void CBaseToggle::AngularMove(const QAngle &vecDestAngle, float flSpeed)
{
#ifdef GAME_DLL
	ASSERTSZ(flSpeed != 0.0f, "AngularMove:  no speed is defined!");
#endif

	m_vecFinalAngle = vecDestAngle;

	m_movementType = MOVE_TOGGLE_ANGULAR;
	// Already there?
	if (vecDestAngle == GetLocalAngles())
	{
		MoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	QAngle vecDestDelta = vecDestAngle - GetLocalAngles();

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	const float MinTravelTime = 0.01f;
	if (flTravelTime < MinTravelTime)
	{
		// If we only travel for a short time, we can fail WillSimulateGamePhysics()
		flTravelTime = MinTravelTime;
		flSpeed = vecDestDelta.Length() / flTravelTime;
	}

	// set m_flNextThink to trigger a call to AngularMoveDone when dest is reached
	SetMoveDoneTime(flTravelTime);

	// scale the destdelta vector by the time spent traveling to get velocity
	SetLocalAngularVelocity(vecDestDelta * (1.0 / flTravelTime));
}

//-----------------------------------------------------------------------------
// Purpose: After rotating, set angle to exact final angle, call "move done" function.
//-----------------------------------------------------------------------------
void CBaseToggle::AngularMoveDone(void)
{
	SetLocalAngles(m_vecFinalAngle);
	SetLocalAngularVelocity(vec3_angle);
	SetMoveDoneTime(-1.f);
}

// DVS TODO: obselete, remove?
bool CBaseToggle::IsLockedByMaster(void)
{
	if (m_sMaster != NULL_STRING && !UTIL_IsMasterCRCTriggered(m_iMasterCRC, m_hActivator))
		return true;
	else
		return false;
}

void CBaseToggle::MoveDone(void)
{
	switch (m_movementType)
	{
	case MOVE_TOGGLE_LINEAR:
		LinearMoveDone();
		break;
	case MOVE_TOGGLE_ANGULAR:
		AngularMoveDone();
		break;
	}
	m_movementType = MOVE_TOGGLE_NONE;
	BaseClass::MoveDone();
}

void CBaseToggle::AxisDir(void)
{
	if (m_spawnflags & SF_DOOR_ROTATE_ROLL)
		m_vecMoveAng = QAngle(0.f, 0.f, 1.f);		// angles are roll
	else if (m_spawnflags & SF_DOOR_ROTATE_PITCH)
		m_vecMoveAng = QAngle(1.f, 0.f, 0.f);		// angles are pitch
	else
		m_vecMoveAng = QAngle(0.f, 1.f, 0.f);		// angles are yaw
}

float CBaseToggle::AxisValue(int flags, const QAngle &angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_ROLL))
		return angles.z;
	if (FBitSet(flags, SF_DOOR_ROTATE_PITCH))
		return angles.x;

	return angles.y;
}

float CBaseToggle::AxisDelta(int flags, const QAngle &angle1, const QAngle &angle2)
{
	// UNDONE: Use AngleDistance() here?
	if (FBitSet(flags, SF_DOOR_ROTATE_ROLL))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_PITCH))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}