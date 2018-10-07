#include "cbase.h"
#include "mom_pointentity.h"

LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA_NO_BASE(CPointEntity)
	DEFINE_PRED_FIELD(m_vecNetworkOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iNameCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

#undef CPointEntity // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_PointEntity, DT_PointEntity, CPointEntity)
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
	RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
	RecvPropInt(RECVINFO(m_iNameCRC)),
END_RECV_TABLE();
#define CPointEntity C_PointEntity // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST_NOBASE(CPointEntity, DT_PointEntity)
	SendPropVector(SENDINFO(m_vecOrigin), 0, SPROP_NOSCALE | SPROP_COORD | SPROP_CHANGES_OFTEN),
	SendPropQAngles(SENDINFO(m_angRotation), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
	SendPropInt(SENDINFO(m_iNameCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CPointEntity)
END_DATADESC();
#endif

void CPointEntity::Spawn(void)
{
#ifdef GAME_DLL
	SetTransmitState(FL_EDICT_ALWAYS);
#endif
	SetSolid(SOLID_NONE);
	//	UTIL_SetSize(this, vec3_origin, vec3_origin);
}

#ifdef GAME_DLL

bool CPointEntity::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "mins") || FStrEq(szKeyName, "maxs"))
	{
		Warning("Warning! Can't specify mins/maxs for point entities! (%s)\n", GetClassname());
		return true;
	}

	return BaseClass::KeyValue(szKeyName, szValue);
}
#endif