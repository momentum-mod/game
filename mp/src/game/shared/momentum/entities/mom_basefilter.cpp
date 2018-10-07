#include "cbase.h"
#include "mom_basefilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(filter, CBaseFilter);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA_NO_BASE(CBaseFilter)
	DEFINE_PRED_FIELD(m_iNameCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bNegated, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();

#undef CBaseFilter // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_BaseFilter, DT_BaseFilter, CBaseFilter)
	RecvPropInt(RECVINFO(m_iNameCRC)),
	RecvPropBool(RECVINFO(m_bNegated)),
END_RECV_TABLE();
#define CBaseFilter C_BaseFilter // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST_NOBASE(CBaseFilter, DT_BaseFilter)
	SendPropInt(SENDINFO(m_iNameCRC)),
	SendPropBool(RECVINFO(m_bNegated)),
END_SEND_TABLE();

BEGIN_DATADESC(CBaseFilter)
	DEFINE_KEYFIELD(m_bNegated, FIELD_BOOLEAN, "Negated"),

	// Inputs
	DEFINE_INPUTFUNC(FIELD_INPUT, "TestActivator", InputTestActivator),

	// Outputs
	DEFINE_OUTPUT(m_OnPass, "OnPass"),
	DEFINE_OUTPUT(m_OnPass, "OnPass"),
	DEFINE_OUTPUT(m_OnFail, "OnFail"),
END_DATADESC();
#endif

bool CBaseFilter::PassesFilter(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	bool baseResult = PassesFilterImpl(pCaller, pEntity);
	return (m_bNegated) ? !baseResult : baseResult;
}

bool CBaseFilter::PassesDamageFilter(const CTakeDamageInfo &info)
{
	bool baseResult = PassesDamageFilterImpl(info);
	return (m_bNegated) ? !baseResult : baseResult;
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for testing the activator. If the activator passes the
//			filter test, the OnPass output is fired. If not, the OnFail output is fired.
//-----------------------------------------------------------------------------
void CBaseFilter::InputTestActivator(inputdata_t &inputdata)
{
	if (PassesFilter(inputdata.pCaller, inputdata.pActivator))
	{
		m_OnPass.FireOutput(inputdata.pActivator, this);
	}
	else
	{
		m_OnFail.FireOutput(inputdata.pActivator, this);
	}
}

bool CBaseFilter::PassesFilterImpl(CBaseEntity * pCaller, CBaseEntity * pEntity)
{
	return true;
}

bool CBaseFilter::PassesDamageFilterImpl(const CTakeDamageInfo &info)
{
	return PassesFilterImpl(NULL, info.GetAttacker());
}

#ifdef GAME_DLL 
// Server stuff
int CBaseFilter::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}
#endif