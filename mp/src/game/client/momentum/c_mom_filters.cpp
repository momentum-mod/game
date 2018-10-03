#include "cbase.h"
#include "c_mom_filters.h"

LINK_ENTITY_TO_CLASS(filter, C_BaseFilter);

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_BaseFilter, DT_BaseFilter, CBaseFilter)
	RecvPropInt(RECVINFO(m_iNameCRC)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA_NO_BASE( C_BaseFilter )
	DEFINE_PRED_FIELD( m_iNameCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

bool C_BaseFilter::PassesFilter(CBaseEntity * pCaller, CBaseEntity * pEntity)
{
	bool baseResult = PassesFilterImpl( pCaller, pEntity );
	return (m_bNegated) ? !baseResult : baseResult;
}

bool C_BaseFilter::PassesDamageFilter(const CTakeDamageInfo & info)
{
	return false;
}

void C_BaseFilter::Spawn()
{
}

void C_BaseFilter::InputTestActivator(inputdata_t & inputdata)
{
}

bool C_BaseFilter::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	return true;
}

// ###################################################################
//	> FilterName
// ###################################################################
class C_FilterName : public C_BaseFilter
{
	DECLARE_CLASS( C_FilterName, C_BaseFilter );
	DECLARE_PREDICTABLE();
	DECLARE_CLIENTCLASS();

public:
	CNetworkVar(unsigned int, m_iFilterNameCRC);

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		CRC32_t crc;
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, "!player", Q_strlen("!player"));
		CRC32_Final(&crc);

		// special check for !player as GetEntityName for player won't return "!player" as a name
		if (crc == m_iFilterNameCRC)
		{
			return pEntity->IsPlayer();
		}
		else
		{
			return pEntity->GetNameCRC() == m_iFilterNameCRC;
		}
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_name, C_FilterName );

IMPLEMENT_CLIENTCLASS_DT(C_FilterName, DT_FilterName, CFilterName)
	RecvPropInt(RECVINFO(m_iFilterNameCRC)),
END_RECV_TABLE();

BEGIN_PREDICTION_DATA( C_FilterName )
	DEFINE_PRED_FIELD( m_iFilterNameCRC, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
