#include "cbase.h"
#include "mom_basetoggle.h"

extern void RecvProxy_EffectFlags(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_MoveCollide(const CRecvProxyData *pData, void *pStruct, void *pOut);

LINK_ENTITY_TO_CLASS(toggle, CBaseToggle);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA_NO_BASE(CBaseToggle)
	DEFINE_PRED_TYPEDESCRIPTION( m_Collision, CCollisionProperty),
	DEFINE_PRED_FIELD(m_iSpawnFlags, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_vecNetworkOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_CollisionGroup, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_FIELD(m_iEFlags, FIELD_INTEGER),
END_PREDICTION_DATA()

#undef CBaseToggle // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_BaseToggle, DT_BaseToggle, CBaseToggle)
	RecvPropDataTable(RECVINFO_DT(m_Collision), 0, &REFERENCE_RECV_TABLE(DT_CollisionProperty)),
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
	RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
	RecvPropInt(RECVINFO(m_nModelIndex)),
	RecvPropInt(RECVINFO(m_CollisionGroup)),
	RecvPropInt(RECVINFO(m_fEffects), 0, RecvProxy_EffectFlags ),
	RecvPropInt(RECVINFO_NAME(m_iSpawnFlags, m_spawnflags)),
	RecvPropInt( "movecollide", 0, SIZEOF_IGNORE, 0, RecvProxy_MoveCollide ),
END_RECV_TABLE();
#define CBaseToggle C_BaseToggle // Redefine for rest of the code
#else // Server save data and send table
BEGIN_DATADESC( CBaseToggle )
	DEFINE_FIELD( m_toggle_state, FIELD_INTEGER ),
	DEFINE_FIELD( m_flMoveDistance, FIELD_FLOAT ),
	DEFINE_FIELD( m_flWait, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLip, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecPosition1, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecPosition2, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecMoveAng, FIELD_VECTOR ),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD( m_vecAngle1, FIELD_VECTOR ),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD( m_vecAngle2, FIELD_VECTOR ),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD( m_flHeight, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecFinalDest, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecFinalAngle, FIELD_VECTOR ),
	DEFINE_FIELD( m_sMaster, FIELD_STRING),
	DEFINE_FIELD( m_movementType, FIELD_INTEGER ),	// Linear or angular movement? (togglemovetypes_t)
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST_NOBASE(CBaseToggle, DT_BaseToggle)
	SendPropDataTable(SENDINFO_DT(m_Collision), &REFERENCE_SEND_TABLE(DT_CollisionProperty)),
	SendPropVector(SENDINFO(m_vecOrigin), 0,  SPROP_NOSCALE|SPROP_COORD|SPROP_CHANGES_OFTEN ),
	SendPropQAngles(SENDINFO(m_angRotation), 0, SPROP_NOSCALE|SPROP_CHANGES_OFTEN ),
	SendPropInt(SENDINFO(m_fEffects),EF_MAX_BITS, SPROP_UNSIGNED),
	SendPropModelIndex(SENDINFO(m_nModelIndex)),
	SendPropInt(SENDINFO(m_CollisionGroup), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_spawnflags)),
	SendPropInt(SENDINFO_NAME(m_MoveCollide, movecollide), MOVECOLLIDE_MAX_BITS, SPROP_UNSIGNED),
END_SEND_TABLE()
#endif
