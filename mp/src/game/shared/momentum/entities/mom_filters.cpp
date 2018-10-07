#include "cbase.h"
#include "mom_filters.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(filter_multi, CFilterMultiple);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterMultiple)
END_PREDICTION_DATA();

#undef CFilterMultiple // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterMultiple, DT_FilterMultiple, CFilterMultiple)
	RecvPropArray3(RECVINFO_ARRAY(m_iFilterNameArrayCRC), RecvPropInt(RECVINFO_ARRAY(m_iFilterNameArrayCRC))),
END_RECV_TABLE();
#define CFilterMultiple C_FilterMultiple // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterMultiple, DT_FilterMultiple)
	SendPropArray3(SENDINFO_ARRAY3(m_iFilterNameArrayCRC), SendPropInt(SENDINFO_ARRAY(m_iFilterNameArrayCRC))),
END_SEND_TABLE();

BEGIN_DATADESC(CFilterMultiple)
	// Keys
	DEFINE_KEYFIELD(m_nFilterType, FIELD_INTEGER, "FilterType"),

	// Silence, Classcheck!
	//	DEFINE_ARRAY( m_iFilterName, FIELD_STRING, MAX_FILTERS ),

	DEFINE_KEYFIELD(m_iFilterName[0], FIELD_STRING, "Filter01"),
	DEFINE_KEYFIELD(m_iFilterName[1], FIELD_STRING, "Filter02"),
	DEFINE_KEYFIELD(m_iFilterName[2], FIELD_STRING, "Filter03"),
	DEFINE_KEYFIELD(m_iFilterName[3], FIELD_STRING, "Filter04"),
	DEFINE_KEYFIELD(m_iFilterName[4], FIELD_STRING, "Filter05"),
	DEFINE_ARRAY(m_hFilter, FIELD_EHANDLE, MAX_FILTERS),
END_DATADESC();
#endif

void CFilterMultiple::Spawn(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the entity passes our filter, false if not.
// Input  : pEntity - Entity to test.
//-----------------------------------------------------------------------------
bool CFilterMultiple::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	// Test against each filter
	if (m_nFilterType == FILTER_AND)
	{
		for (int i = 0; i < MAX_FILTERS; i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (!pFilter->PassesFilter(pCaller, pEntity))
				{
					return false;
				}
			}
		}
		return true;
	}
	else  // m_nFilterType == FILTER_OR
	{
		for (int i = 0; i < MAX_FILTERS; i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (pFilter->PassesFilter(pCaller, pEntity))
				{
					return true;
				}
			}
		}
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the entity passes our filter, false if not.
// Input  : pEntity - Entity to test.
//-----------------------------------------------------------------------------
bool CFilterMultiple::PassesDamageFilterImpl(const CTakeDamageInfo &info)
{
	// Test against each filter
	if (m_nFilterType == FILTER_AND)
	{
		for (int i = 0; i < MAX_FILTERS; i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (!pFilter->PassesDamageFilter(info))
				{
					return false;
				}
			}
		}
		return true;
	}
	else  // m_nFilterType == FILTER_OR
	{
		for (int i = 0; i < MAX_FILTERS; i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (pFilter->PassesDamageFilter(info))
				{
					return true;
				}
			}
		}
		return false;
	}
}

//------------------------------------------------------------------------------
// Purpose : Called after all entities have been loaded
//------------------------------------------------------------------------------
void CFilterMultiple::Activate(void)
{
	BaseClass::Activate();

	// We may reject an entity specified in the array of names, but we want the array of valid filters to be contiguous!
	int nNextFilter = 0;

	// Get handles to my filter entities
	for (int i = 0; i < MAX_FILTERS; i++)
	{
		if (m_iFilterName[i] != NULL_STRING)
		{
			CBaseEntity *pEntity = FindEntityByNameCRC(NULL, m_iFilterNameArrayCRC[i]);
			CBaseFilter *pFilter = dynamic_cast<CBaseFilter *>(pEntity);
			if (pFilter == NULL)
			{
				Warning("filter_multi: Tried to add entity (%s) which is not a filter entity!\n", STRING(m_iFilterName[i]));
				continue;
			}

			// Take this entity and increment out array pointer
			m_hFilter[nNextFilter] = pFilter;
			nNextFilter++;
		}
	}
}

LINK_ENTITY_TO_CLASS(filter_activator_name, CFilterName);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterName)
END_PREDICTION_DATA();

#undef CFilterName // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterName, DT_FilterName, CFilterName)
	RecvPropInt(RECVINFO(m_iFilterNameCRC)),
END_RECV_TABLE();
#define CFilterName C_FilterName // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterName, DT_FilterName)
	SendPropInt(SENDINFO(m_iFilterNameCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CFilterName)
	DEFINE_KEYFIELD(m_iFilterName, FIELD_STRING, "filtername"),
END_DATADESC();
#endif

void CFilterName::Spawn(void)
{
	BaseClass::Spawn();

	CRC32_t crc;
	if (Q_strlen(STRING(m_iFilterName)))
	{
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, STRING(m_iFilterName), Q_strlen(STRING(m_iFilterName)));
		CRC32_Final(&crc);

		m_iFilterNameCRC = crc;
	}
}

bool CFilterName::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	static CRC32_t crc = 0;

	if (crc == 0)
	{
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, "!player", Q_strlen("!player"));
		CRC32_Final(&crc);
	}

	// special check for !player as GetEntityName for player won't return "!player" as a name
	if (m_iFilterNameCRC == crc)
	{
		return (pEntity->IsPlayer());
	}
	else
	{
		return (pEntity->GetNameCRC() == m_iFilterNameCRC);
	}
}

LINK_ENTITY_TO_CLASS(filter_activator_team, CFilterTeam);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterTeam)
END_PREDICTION_DATA();

#undef CFilterTeam // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterTeam, DT_FilterTeam, CFilterTeam)
END_RECV_TABLE();
#define CFilterTeam C_FilterTeam // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterTeam, DT_FilterTeam)
END_SEND_TABLE();

BEGIN_DATADESC(CFilterTeam)
	DEFINE_KEYFIELD(m_iFilterTeam, FIELD_INTEGER, "filterteam"),
END_DATADESC();
#endif

bool CFilterTeam::PassesFilterImpl(CBaseEntity * pCaller, CBaseEntity * pEntity)
{
	return ( pEntity->GetTeamNumber() == m_iFilterTeam );
}

LINK_ENTITY_TO_CLASS(filter_activator_class, CFilterClass);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterClass)
END_PREDICTION_DATA();

#undef CFilterClass // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterClass, DT_FilterClass, CFilterClass)
	RecvPropInt(RECVINFO(m_iClassnameCRC)),
	RecvPropInt(RECVINFO(m_iFilterClassCRC)),
END_RECV_TABLE();
#define CFilterClass C_FilterClass // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterClass, DT_FilterClass)
	SendPropInt(SENDINFO(m_iClassnameCRC)),
	SendPropInt(SENDINFO(m_iFilterClassCRC)),
END_SEND_TABLE();

BEGIN_DATADESC(CFilterClass)
	DEFINE_KEYFIELD(m_iFilterClass, FIELD_STRING, "filterclass"),
END_DATADESC();
#endif

void CFilterClass::Spawn(void)
{
	BaseClass::Spawn();

	CRC32_t crc;
	if (Q_strlen(STRING(m_iFilterClass)))
	{
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, STRING(m_iFilterClass), Q_strlen(STRING(m_iFilterClass)));
		CRC32_Final(&crc);

		m_iFilterClassCRC = crc;
	}
}

bool CFilterClass::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	return pEntity->GetClassnameCRC() == m_iFilterClassCRC;
}

LINK_ENTITY_TO_CLASS( filter_activator_mass_greater, CFilterMassGreater );

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterMassGreater)
END_PREDICTION_DATA();

#undef CFilterMassGreater // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterMassGreater, DT_FilterMassGreater, CFilterMassGreater)
	RecvPropFloat(RECVINFO(m_fFilterMass)),
END_RECV_TABLE();
#define CFilterMassGreater C_FilterMassGreater // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterMassGreater, DT_FilterMassGreater)
	SendPropFloat(SENDINFO(m_fFilterMass)),
END_SEND_TABLE();

BEGIN_DATADESC(CFilterMassGreater)
	DEFINE_KEYFIELD(m_fFilterMass, FIELD_FLOAT, "filtermass"),
END_DATADESC();
#endif

bool CFilterMassGreater::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	if ( pEntity->VPhysicsGetObject() == NULL )
		return false;

	return ( pEntity->VPhysicsGetObject()->GetMass() > m_fFilterMass );
}

LINK_ENTITY_TO_CLASS(filter_damage_type, CFilterDamageType);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterDamageType)
END_PREDICTION_DATA();

#undef CFilterMassGreater // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterDamageType, DT_FilterDamageType, CFilterDamageType)
	RecvPropInt(RECVINFO(m_iDamageType)),
END_RECV_TABLE();
#define CFilterDamageType C_FilterDamageType // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterDamageType, DT_FilterDamageType)
	SendPropInt(SENDINFO(m_iDamageType)),
END_SEND_TABLE();

BEGIN_DATADESC(CFilterDamageType)
	DEFINE_KEYFIELD(m_iDamageType, FIELD_INTEGER, "damagetype"),
END_DATADESC();
#endif

bool CFilterDamageType::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity )
{
#ifdef GAME_DLL
	ASSERT( false );
#endif
	return true;
}

bool CFilterDamageType::PassesDamageFilterImpl(const CTakeDamageInfo &info)
{
	return info.GetDamageType() == m_iDamageType;
}

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS(filter_enemy, CFilterEnemy);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CFilterEnemy)
END_PREDICTION_DATA();

#undef CFilterEnemy // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_FilterEnemy, DT_FilterEnemy, CFilterEnemy)
END_RECV_TABLE();
#define CFilterEnemy C_FilterEnemy // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CFilterEnemy, DT_FilterEnemy)
END_SEND_TABLE();

BEGIN_DATADESC(CFilterEnemy)
	DEFINE_KEYFIELD(m_iszEnemyName, FIELD_STRING, "filtername"),
	DEFINE_KEYFIELD(m_flRadius, FIELD_FLOAT, "filter_radius"),
	DEFINE_KEYFIELD(m_flOuterRadius, FIELD_FLOAT, "filter_outer_radius"),
	DEFINE_KEYFIELD(m_nMaxSquadmatesPerEnemy, FIELD_INTEGER, "filter_max_per_enemy"),
	DEFINE_FIELD(m_iszPlayerName, FIELD_STRING),
END_DATADESC();
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	if ( pCaller == NULL || pEntity == NULL )
		return false;

	// If asked to, we'll never fail to pass an already acquired enemy
	//	This allows us to use test criteria to initially pick an enemy, then disregard the test until a new enemy comes along
	if ( HasSpawnFlags( SF_FILTER_ENEMY_NO_LOSE_AQUIRED ) && ( pEntity == pCaller->GetEnemy() ) )
		return true;

	// This is a little weird, but it's saying that if we're not the entity we're excluding the filter to, then just pass it throughZ
	if ( PassesNameFilter( pEntity ) == false )
		return true;

	if ( PassesProximityFilter( pCaller, pEntity ) == false )
		return false;

	// NOTE: This can result in some weird NPC behavior if used improperly
	if ( PassesMobbedFilter( pCaller, pEntity ) == false )
		return false;

	// The filter has been passed, meaning:
	//	- If we wanted all criteria to fail, they have
	//  - If we wanted all criteria to succeed, they have

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesDamageFilterImpl( const CTakeDamageInfo &info )
{
	// NOTE: This function has no meaning to this implementation of the filter class!
	Assert( 0 );
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Tests the enemy's name or classname
// Input  : *pEnemy - Entity being assessed
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesNameFilter(CBaseEntity *pEnemy)
{
	// If there is no name specified, we're not using it
	if (m_iszEnemyName == NULL_STRING)
		return true;

	// Cache off the special case player name
	if (m_iszPlayerName == NULL_STRING)
	{
		m_iszPlayerName = FindPooledString("!player");
	}

	if (m_iszEnemyName == m_iszPlayerName)
	{
		if (pEnemy->IsPlayer())
		{
			if (m_bNegated)
				return false;

			return true;
		}
	}

	// May be either a targetname or classname
	bool bNameOrClassnameMatches = (m_iszEnemyName == pEnemy->GetEntityName() || m_iszEnemyName == pEnemy->m_iClassname);

	// We only leave this code block in a state meaning we've "succeeded" in any context
	if (m_bNegated)
	{
		// We wanted the names to not match, but they did
		if (bNameOrClassnameMatches)
			return false;
	}
	else
	{
		// We wanted them to be the same, but they weren't
		if (bNameOrClassnameMatches == false)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Tests the enemy's proximity to the caller's position
// Input  : *pCaller - Entity assessing the target
//			*pEnemy - Entity being assessed
// Output : Returns true if potential enemy passes this filter stage
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesProximityFilter(CBaseEntity *pCaller, CBaseEntity *pEnemy)
{
	// If there is no radius specified, we're not testing it
	if (m_flRadius <= 0.0f)
		return true;

	// We test the proximity differently when we've already picked up this enemy before
	bool bAlreadyEnemy = (pCaller->GetEnemy() == pEnemy);

	// Get our squared length to the enemy from the caller
	float flDistToEnemySqr = (pCaller->GetAbsOrigin() - pEnemy->GetAbsOrigin()).LengthSqr();

	// Two radii are used to control oscillation between true/false cases
	// The larger radius is either specified or defaulted to be double or half the size of the inner radius
	float flLargerRadius = m_flOuterRadius;
	if (flLargerRadius == 0)
	{
		flLargerRadius = (m_bNegated) ? (m_flRadius*0.5f) : (m_flRadius*2.0f);
	}

	float flSmallerRadius = m_flRadius;
	if (flSmallerRadius > flLargerRadius)
	{
		::V_swap(flLargerRadius, flSmallerRadius);
	}

	float flDist;
	if (bAlreadyEnemy)
	{
		flDist = (m_bNegated) ? flSmallerRadius : flLargerRadius;
	}
	else
	{
		flDist = (m_bNegated) ? flLargerRadius : flSmallerRadius;
	}

	// Test for success
	if (flDistToEnemySqr <= (flDist*flDist))
	{
		// We wanted to fail but didn't
		if (m_bNegated)
			return false;

		return true;
	}

	// We wanted to succeed but didn't
	if (m_bNegated == false)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Attempt to govern how many squad members can target any given entity
// Input  : *pCaller - Entity assessing the target
//			*pEnemy - Entity being assessed
// Output : Returns true if potential enemy passes this filter stage
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesMobbedFilter(CBaseEntity *pCaller, CBaseEntity *pEnemy)
{
	// Must be a valid candidate
	CAI_BaseNPC *pNPC = pCaller->MyNPCPointer();
	if (pNPC == NULL || pNPC->GetSquad() == NULL)
		return true;

	// Make sure we're checking for this
	if (m_nMaxSquadmatesPerEnemy <= 0)
		return true;

	AISquadIter_t iter;
	int nNumMatchingSquadmates = 0;

	// Look through our squad members to see how many of them are already mobbing this entity
	for (CAI_BaseNPC *pSquadMember = pNPC->GetSquad()->GetFirstMember(&iter); pSquadMember != NULL; pSquadMember = pNPC->GetSquad()->GetNextMember(&iter))
	{
		// Disregard ourself
		if (pSquadMember == pNPC)
			continue;

		// If the enemies match, count it
		if (pSquadMember->GetEnemy() == pEnemy)
		{
			nNumMatchingSquadmates++;

			// If we're at or passed the max we stop
			if (nNumMatchingSquadmates >= m_nMaxSquadmatesPerEnemy)
			{
				// We wanted to find more than allowed and we did
				if (m_bNegated)
					return true;

				// We wanted to be less but we're not
				return false;
			}
		}
	}

	// We wanted to find more than the allowed amount but we didn't
	if (m_bNegated)
		return false;

	return true;
}
#endif