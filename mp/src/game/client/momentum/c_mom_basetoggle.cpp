#include "cbase.h"
#include "c_mom_buttons.h"

LINK_ENTITY_TO_CLASS(toggle, C_BaseToggle);

extern void RecvProxy_EffectFlags(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_MoveCollide(const CRecvProxyData *pData, void *pStruct, void *pOut);

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

BEGIN_PREDICTION_DATA_NO_BASE( C_BaseToggle )
	DEFINE_PRED_TYPEDESCRIPTION( m_Collision, CCollisionProperty ),
	DEFINE_PRED_FIELD( m_iSpawnFlags, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecNetworkOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_CollisionGroup, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_FIELD( m_iEFlags, FIELD_INTEGER ),
END_PREDICTION_DATA()
