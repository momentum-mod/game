//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "filters.h"
#include "entitylist.h"
#include "ai_squad.h"
#include "ai_basenpc.h"
#include "momentum/matchers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ###################################################################
//	> BaseFilter
// ###################################################################
LINK_ENTITY_TO_CLASS(filter_base, CBaseFilter);

BEGIN_DATADESC( CBaseFilter )

	DEFINE_KEYFIELD(m_bNegated, FIELD_BOOLEAN, "Negated"),
	DEFINE_KEYFIELD(m_bPassCallerWhenTested, FIELD_BOOLEAN, "PassCallerWhenTested"),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_INPUT, "TestActivator", InputTestActivator ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "TestEntity", InputTestEntity ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "SetField", InputSetField ),

	// Outputs
	DEFINE_OUTPUT( m_OnPass, "OnPass"),
	DEFINE_OUTPUT( m_OnFail, "OnFail"),

END_DATADESC()

//-----------------------------------------------------------------------------

bool CBaseFilter::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	return true;
}


bool CBaseFilter::PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	bool baseResult = PassesFilterImpl( pCaller, pEntity );
	return (m_bNegated) ? !baseResult : baseResult;
}


bool CBaseFilter::PassesDamageFilter(const CTakeDamageInfo &info)
{
	bool baseResult = PassesDamageFilterImpl(info);
	return (m_bNegated) ? !baseResult : baseResult;
}


bool CBaseFilter::PassesDamageFilterImpl( const CTakeDamageInfo &info )
{
	return PassesFilterImpl( NULL, info.GetAttacker() );
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for testing the activator. If the activator passes the
//			filter test, the OnPass output is fired. If not, the OnFail output is fired.
//-----------------------------------------------------------------------------
void CBaseFilter::InputTestActivator( inputdata_t &inputdata )
{
	if ( PassesFilter( inputdata.pCaller, inputdata.pActivator ) )
	{
		m_OnPass.FireOutput( inputdata.pActivator, m_bPassCallerWhenTested ? inputdata.pCaller : this );
	}
	else
	{
		m_OnFail.FireOutput( inputdata.pActivator, m_bPassCallerWhenTested ? inputdata.pCaller : this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for testing the activator. If the activator passes the
//			filter test, the OnPass output is fired. If not, the OnFail output is fired.
//-----------------------------------------------------------------------------
void CBaseFilter::InputTestEntity( inputdata_t &inputdata )
{
	if ( PassesFilter( inputdata.pCaller, inputdata.value.Entity() ) )
	{
		m_OnPass.FireOutput( inputdata.value.Entity(), m_bPassCallerWhenTested ? inputdata.pCaller : this );
	}
	else
	{
		m_OnFail.FireOutput( inputdata.value.Entity(), m_bPassCallerWhenTested ? inputdata.pCaller : this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tries to set the filter's target since most filters use "filtername" anyway
//-----------------------------------------------------------------------------
void CBaseFilter::InputSetField( inputdata_t& inputdata )
{
	KeyValue("filtername", inputdata.value.String());
	Activate();
}


// ###################################################################
//	> FilterMultiple
//
//   Allows one to filter through multiple filters
// ###################################################################
#define MAX_FILTERS 5
enum filter_t
{
	FILTER_AND,
	FILTER_OR,
};

class CFilterMultiple : public CBaseFilter
{
	DECLARE_CLASS( CFilterMultiple, CBaseFilter );
	DECLARE_DATADESC();

	filter_t	m_nFilterType;
	string_t	m_iFilterName[MAX_FILTERS];
	EHANDLE		m_hFilter[MAX_FILTERS];

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilterImpl(const CTakeDamageInfo &info);
	void Activate(void);
};

LINK_ENTITY_TO_CLASS(filter_multi, CFilterMultiple);

BEGIN_DATADESC( CFilterMultiple )


	// Keys
	DEFINE_KEYFIELD(m_nFilterType, FIELD_INTEGER, "FilterType"),

	// Silence, Classcheck!
//	DEFINE_ARRAY( m_iFilterName, FIELD_STRING, MAX_FILTERS ),

	DEFINE_KEYFIELD(m_iFilterName[0], FIELD_STRING, "Filter01"),
	DEFINE_KEYFIELD(m_iFilterName[1], FIELD_STRING, "Filter02"),
	DEFINE_KEYFIELD(m_iFilterName[2], FIELD_STRING, "Filter03"),
	DEFINE_KEYFIELD(m_iFilterName[3], FIELD_STRING, "Filter04"),
	DEFINE_KEYFIELD(m_iFilterName[4], FIELD_STRING, "Filter05"),
	DEFINE_ARRAY( m_hFilter, FIELD_EHANDLE, MAX_FILTERS ),

END_DATADESC()



//------------------------------------------------------------------------------
// Purpose : Called after all entities have been loaded
//------------------------------------------------------------------------------
void CFilterMultiple::Activate( void )
{
	BaseClass::Activate();
	
	// We may reject an entity specified in the array of names, but we want the array of valid filters to be contiguous!
	int nNextFilter = 0;

	// Get handles to my filter entities
	for ( int i = 0; i < MAX_FILTERS; i++ )
	{
		if ( m_iFilterName[i] != NULL_STRING )
		{
			CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, m_iFilterName[i] );
			CBaseFilter *pFilter = dynamic_cast<CBaseFilter *>(pEntity);
			if ( pFilter == NULL )
			{
				Warning("filter_multi: Tried to add entity (%s) which is not a filter entity!\n", STRING( m_iFilterName[i] ) );
				continue;
			}
			else if ( pFilter == this )
			{
				Warning("filter_multi: Tried to add itself!\n");
				continue;
			}

			// Take this entity and increment out array pointer
			m_hFilter[nNextFilter] = pFilter;
			nNextFilter++;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if the entity passes our filter, false if not.
// Input  : pEntity - Entity to test.
//-----------------------------------------------------------------------------
bool CFilterMultiple::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	// Test against each filter
	if (m_nFilterType == FILTER_AND)
	{
		for (int i=0;i<MAX_FILTERS;i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (!pFilter->PassesFilter( pCaller, pEntity ) )
				{
					return false;
				}
			}
		}
		return true;
	}
	else  // m_nFilterType == FILTER_OR
	{
		for (int i=0;i<MAX_FILTERS;i++)
		{
			if (m_hFilter[i] != NULL)
			{
				CBaseFilter* pFilter = (CBaseFilter *)(m_hFilter[i].Get());
				if (pFilter->PassesFilter( pCaller, pEntity ) )
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
		for (int i=0;i<MAX_FILTERS;i++)
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
		for (int i=0;i<MAX_FILTERS;i++)
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


// ###################################################################
//	> FilterName
// ###################################################################
class CFilterName : public CBaseFilter
{
	DECLARE_CLASS( CFilterName, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterName;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		// special check for !player as GetEntityName for player won't return "!player" as a name
		if (FStrEq(STRING(m_iFilterName), "!player"))
		{
			return pEntity->IsPlayer();
		}
		else
		{
			return pEntity->NameMatches( STRING(m_iFilterName) );
		}
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_STRING);
		m_iFilterName = inputdata.value.StringID();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_name, CFilterName );

BEGIN_DATADESC( CFilterName )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterName,	FIELD_STRING,	"filtername" ),

END_DATADESC()

// ###################################################################
//	> FilterModel
// ###################################################################
class CFilterModel : public CBaseFilter
{
    DECLARE_CLASS(CFilterModel, CBaseFilter);
    DECLARE_DATADESC();

public:
    string_t m_iFilterModel;
    string_t m_strFilterSkin;

    bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
    {
        if (FStrEq(STRING(m_strFilterSkin), "-1") /*m_strFilterSkin == NULL_STRING|| FStrEq(STRING(m_strFilterSkin), "")*/)
            return Matcher_NamesMatch(STRING(m_iFilterModel), STRING(pEntity->GetModelName()));
        else if (pEntity->GetBaseAnimating())
        {
            //DevMsg("Skin isn't null\n");
            return Matcher_NamesMatch(STRING(m_iFilterModel), STRING(pEntity->GetModelName())) && Matcher_Match(STRING(m_strFilterSkin), pEntity->GetBaseAnimating()->m_nSkin);
        }
        return false;
    }

    void InputSetField( inputdata_t& inputdata )
    {
        inputdata.value.Convert(FIELD_STRING);
        m_iFilterModel = inputdata.value.StringID();
    }
};

LINK_ENTITY_TO_CLASS(filter_activator_model, CFilterModel);

BEGIN_DATADESC(CFilterModel)

	// Keyfields
	DEFINE_KEYFIELD(m_iFilterModel,	FIELD_STRING, "filtermodel"),
	DEFINE_KEYFIELD(m_strFilterSkin, FIELD_STRING, "skin"),

END_DATADESC()

// ###################################################################
//	> FilterContext
// ###################################################################
class CFilterContext : public CBaseFilter
{
    DECLARE_CLASS(CFilterContext, CBaseFilter);
    DECLARE_DATADESC();

public:
	bool m_bAny;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		bool passes = false;
		ResponseContext_t curcontext;
		const char *contextvalue;
		for (int i = 0; i < GetContextCount(); i++)
		{
			curcontext = m_ResponseContexts[i];
			if (!pEntity->HasContext(STRING(curcontext.m_iszName), NULL))
			{
				if (m_bAny)
					continue;
				else
					return false;
			}

			contextvalue = pEntity->GetContextValue(STRING(curcontext.m_iszName));
			if (Matcher_NamesMatch(STRING(m_ResponseContexts[i].m_iszValue), contextvalue))
			{
				passes = true;
				if (m_bAny)
					break;
			}
			else if (!m_bAny)
			{
				return false;
			}
		}

		return passes;
	}

	void InputSetField( inputdata_t& inputdata )
	{
		m_ResponseContexts.RemoveAll();
		AddContext(inputdata.value.String());
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_context, CFilterContext );

BEGIN_DATADESC( CFilterContext )

	// Keyfields
	DEFINE_KEYFIELD( m_bAny,	FIELD_BOOLEAN,	"any" ),

END_DATADESC()

extern bool ReadUnregisteredKeyfields(CBaseEntity *pTarget, const char *szKeyName, variant_t *variant);

// ###################################################################
//	> CFilterKeyfield
// ###################################################################
class CFilterKeyfield : public CBaseFilter
{
	DECLARE_CLASS( CFilterKeyfield, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterKey;
	string_t m_iFilterValue;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		variant_t var;
		bool found = (pEntity->ReadKeyField(STRING(m_iFilterKey), &var) || ReadUnregisteredKeyfields(pEntity, STRING(m_iFilterKey), &var));
		return m_iFilterValue != NULL_STRING ? Matcher_Match(STRING(m_iFilterValue), var.String()) : found;
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_STRING);
		m_iFilterKey = inputdata.value.StringID();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_keyfield, CFilterKeyfield );

BEGIN_DATADESC( CFilterKeyfield )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterKey,	FIELD_STRING,	"keyname" ),
	DEFINE_KEYFIELD( m_iFilterValue,	FIELD_STRING,	"value" ),

END_DATADESC()

// ###################################################################
//	> FilterClass
// ###################################################################
class CFilterClass : public CBaseFilter
{
	DECLARE_CLASS( CFilterClass, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterClass;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		return pEntity->ClassMatches( STRING(m_iFilterClass) );
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_STRING);
		m_iFilterClass = inputdata.value.StringID();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_class, CFilterClass );

BEGIN_DATADESC( CFilterClass )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterClass,	FIELD_STRING,	"filterclass" ),

END_DATADESC()


// ###################################################################
//	> FilterTeam
// ###################################################################
class FilterTeam : public CBaseFilter
{
	DECLARE_CLASS( FilterTeam, CBaseFilter );
	DECLARE_DATADESC();

public:
	int		m_iFilterTeam;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
	 	return ( pEntity->GetTeamNumber() == m_iFilterTeam );
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_team, FilterTeam );

BEGIN_DATADESC( FilterTeam )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterTeam,	FIELD_INTEGER,	"filterteam" ),

END_DATADESC()


// ###################################################################
//	> FilterMassGreater
// ###################################################################
class CFilterMassGreater : public CBaseFilter
{
	DECLARE_CLASS( CFilterMassGreater, CBaseFilter );
	DECLARE_DATADESC();

public:
	float m_fFilterMass;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		if ( pEntity->VPhysicsGetObject() == NULL )
			return false;

		return ( pEntity->VPhysicsGetObject()->GetMass() > m_fFilterMass );
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_FLOAT);
		m_fFilterMass = inputdata.value.Float();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_mass_greater, CFilterMassGreater );

BEGIN_DATADESC( CFilterMassGreater )

// Keyfields
DEFINE_KEYFIELD( m_fFilterMass,	FIELD_FLOAT,	"filtermass" ),

END_DATADESC()


// ###################################################################
//	> FilterDamageType
// ###################################################################
class FilterDamageType : public CBaseFilter
{
	DECLARE_CLASS( FilterDamageType, CBaseFilter );
	DECLARE_DATADESC();

protected:

	bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		ASSERT( false );
	 	return true;
	}

	bool PassesDamageFilterImpl(const CTakeDamageInfo &info)
	{
	 	return info.GetDamageType() == m_iDamageType;
	}

	int m_iDamageType;
};

LINK_ENTITY_TO_CLASS( filter_damage_type, FilterDamageType );

BEGIN_DATADESC( FilterDamageType )

	// Keyfields
	DEFINE_KEYFIELD( m_iDamageType,	FIELD_INTEGER,	"damagetype" ),

END_DATADESC()

// ###################################################################
//	> CFilterEnemy
// ###################################################################

#define SF_FILTER_ENEMY_NO_LOSE_AQUIRED	(1<<0)

class CFilterEnemy : public CBaseFilter
{
	DECLARE_CLASS( CFilterEnemy, CBaseFilter );
		// NOT SAVED	
		// m_iszPlayerName
	DECLARE_DATADESC();

public:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl( const CTakeDamageInfo &info );

private:

	bool	PassesNameFilter( CBaseEntity *pCaller );
	bool	PassesProximityFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy );
	bool	PassesMobbedFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy );

	string_t	m_iszEnemyName;				// Name or classname
	float		m_flRadius;					// Radius (enemies are acquired at this range)
	float		m_flOuterRadius;			// Outer radius (enemies are LOST at this range)
	int		m_nMaxSquadmatesPerEnemy;	// Maximum number of squadmates who may share the same enemy
	string_t	m_iszPlayerName;			// "!player"
};

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
bool CFilterEnemy::PassesNameFilter( CBaseEntity *pEnemy )
{
	// If there is no name specified, we're not using it
	if ( m_iszEnemyName	== NULL_STRING )
		return true;

	// Cache off the special case player name
	if ( m_iszPlayerName == NULL_STRING )
	{
		m_iszPlayerName = FindPooledString( "!player" );
	}

	if ( m_iszEnemyName == m_iszPlayerName )
	{
		if ( pEnemy->IsPlayer() )
		{
			if ( m_bNegated )
				return false;

			return true;
		}
	}

	// May be either a targetname or classname
	bool bNameOrClassnameMatches = ( m_iszEnemyName == pEnemy->GetEntityName() || m_iszEnemyName == pEnemy->m_iClassname );

	// We only leave this code block in a state meaning we've "succeeded" in any context
	if ( m_bNegated )
	{
		// We wanted the names to not match, but they did
		if ( bNameOrClassnameMatches )
			return false;
	}
	else
	{
		// We wanted them to be the same, but they weren't
		if ( bNameOrClassnameMatches == false )
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
bool CFilterEnemy::PassesProximityFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy )
{
	// If there is no radius specified, we're not testing it
	if ( m_flRadius <= 0.0f )
		return true;

	// We test the proximity differently when we've already picked up this enemy before
	bool bAlreadyEnemy = ( pCaller->GetEnemy() == pEnemy );

	// Get our squared length to the enemy from the caller
	float flDistToEnemySqr = ( pCaller->GetAbsOrigin() - pEnemy->GetAbsOrigin() ).LengthSqr();

	// Two radii are used to control oscillation between true/false cases
	// The larger radius is either specified or defaulted to be double or half the size of the inner radius
	float flLargerRadius = m_flOuterRadius;
	if ( flLargerRadius == 0 )
	{
		flLargerRadius = ( m_bNegated ) ? (m_flRadius*0.5f) : (m_flRadius*2.0f);
	}

	float flSmallerRadius = m_flRadius;
	if ( flSmallerRadius > flLargerRadius )
	{
		::V_swap( flLargerRadius, flSmallerRadius );
	}

	float flDist;	
	if ( bAlreadyEnemy )
	{
		flDist = ( m_bNegated ) ? flSmallerRadius : flLargerRadius;
	}
	else
	{
		flDist = ( m_bNegated ) ? flLargerRadius : flSmallerRadius;
	}

	// Test for success
	if ( flDistToEnemySqr <= (flDist*flDist) )
	{
		// We wanted to fail but didn't
		if ( m_bNegated )
			return false;

		return true;
	}
	
	// We wanted to succeed but didn't
	if ( m_bNegated == false )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Attempt to govern how many squad members can target any given entity
// Input  : *pCaller - Entity assessing the target
//			*pEnemy - Entity being assessed
// Output : Returns true if potential enemy passes this filter stage
//-----------------------------------------------------------------------------
bool CFilterEnemy::PassesMobbedFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy )
{
	// Must be a valid candidate
	CAI_BaseNPC *pNPC = pCaller->MyNPCPointer();
	if ( pNPC == NULL || pNPC->GetSquad() == NULL )
		return true;

	// Make sure we're checking for this
	if ( m_nMaxSquadmatesPerEnemy <= 0 )
		return true;

	AISquadIter_t iter;
	int nNumMatchingSquadmates = 0;
	
	// Look through our squad members to see how many of them are already mobbing this entity
	for ( CAI_BaseNPC *pSquadMember = pNPC->GetSquad()->GetFirstMember( &iter ); pSquadMember != NULL; pSquadMember = pNPC->GetSquad()->GetNextMember( &iter ) )
	{
		// Disregard ourself
		if ( pSquadMember == pNPC )
			continue;

		// If the enemies match, count it
		if ( pSquadMember->GetEnemy() == pEnemy )
		{
			nNumMatchingSquadmates++;

			// If we're at or passed the max we stop
			if ( nNumMatchingSquadmates >= m_nMaxSquadmatesPerEnemy )
			{
				// We wanted to find more than allowed and we did
				if ( m_bNegated )
					return true;
				
				// We wanted to be less but we're not
				return false;
			}
		}
	}

	// We wanted to find more than the allowed amount but we didn't
	if ( m_bNegated )
		return false;

	return true;
}

LINK_ENTITY_TO_CLASS( filter_enemy, CFilterEnemy );

BEGIN_DATADESC( CFilterEnemy )
	
	DEFINE_KEYFIELD( m_iszEnemyName, FIELD_STRING, "filtername" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "filter_radius" ),
	DEFINE_KEYFIELD( m_flOuterRadius, FIELD_FLOAT, "filter_outer_radius" ),
	DEFINE_KEYFIELD( m_nMaxSquadmatesPerEnemy, FIELD_INTEGER, "filter_max_per_enemy" ),
	DEFINE_FIELD( m_iszPlayerName, FIELD_STRING ),

END_DATADESC()

extern bool TestEntityTriggerIntersection_Accurate( CBaseEntity *pTrigger, CBaseEntity *pEntity );

// ###################################################################
//	> CFilterInVolume
// Passes when the entity is within the specified volume.
// ###################################################################
class CFilterInVolume : public CBaseFilter
{
	DECLARE_CLASS( CFilterInVolume, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iszVolumeTester;

	void Spawn()
	{
		BaseClass::Spawn();

		// Assume no string = use activator
		if (m_iszVolumeTester == NULL_STRING)
			m_iszVolumeTester = AllocPooledString("!activator");
	}

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		CBaseEntity *pVolume = gEntList.FindEntityByNameNearest(STRING(m_target), pEntity->GetLocalOrigin(), 0, this, pEntity, pCaller);
		if (!pVolume)
		{
			Msg("%s cannot find volume %s\n", GetDebugName(), STRING(m_target));
			return false;
		}

		CBaseEntity *pTarget = gEntList.FindEntityByName(NULL, STRING(m_iszVolumeTester), this, pEntity, pCaller);
		if (pTarget)
			return TestEntityTriggerIntersection_Accurate(pVolume, pTarget);
		else
		{
			Msg("%s cannot find target entity %s, returning false\n", GetDebugName(), STRING(m_iszVolumeTester));
			return false;
		}
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_STRING);
		m_iszVolumeTester = inputdata.value.StringID();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_involume, CFilterInVolume );

BEGIN_DATADESC( CFilterInVolume )

	// Keyfields
	DEFINE_KEYFIELD( m_iszVolumeTester,	FIELD_STRING,	"tester" ),

END_DATADESC()

// ###################################################################
//	> CFilterSurfaceProp
// ###################################################################
class CFilterSurfaceData : public CBaseFilter
{
	DECLARE_CLASS( CFilterSurfaceData, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterSurface;
	int m_iSurfaceIndex;

	enum
	{
		SURFACETYPE_SURFACEPROP,
		SURFACETYPE_GAMEMATERIAL,
	};

	// Gets the surfaceprop's game material and filters by that.
	int m_iSurfaceType;

	void ParseSurfaceIndex()
	{
		m_iSurfaceIndex = physprops->GetSurfaceIndex(STRING(m_iFilterSurface));

		switch (m_iSurfaceType)
		{
			case SURFACETYPE_GAMEMATERIAL:
			{
				const surfacedata_t *pSurfaceData = physprops->GetSurfaceData(m_iSurfaceIndex);
				if (pSurfaceData)
					m_iSurfaceIndex = pSurfaceData->game.material;
				else
					Warning("Can't get surface data for %s\n", STRING(m_iFilterSurface));
			} break;
		}
	}

	void Activate()
	{
		ParseSurfaceIndex();
	}

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		if (pEntity->VPhysicsGetObject())
		{
			int iMatIndex = pEntity->VPhysicsGetObject()->GetMaterialIndex();
			switch (m_iSurfaceType)
			{
				case SURFACETYPE_GAMEMATERIAL:
				{
					const surfacedata_t *pSurfaceData = physprops->GetSurfaceData(iMatIndex);
					if (pSurfaceData)
						return m_iSurfaceIndex == pSurfaceData->game.material;
				}
				default:
					return iMatIndex == m_iSurfaceIndex;
			}
		}

		return false;
	}

	void InputSetField( inputdata_t& inputdata )
	{
		inputdata.value.Convert(FIELD_STRING);
		m_iFilterSurface = inputdata.value.StringID();
		ParseSurfaceIndex();
	}
};

LINK_ENTITY_TO_CLASS( filter_activator_surfacedata, CFilterSurfaceData );

BEGIN_DATADESC( CFilterSurfaceData )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterSurface,	FIELD_STRING,	"filterstring" ),
	DEFINE_KEYFIELD( m_iSurfaceType, FIELD_INTEGER, "SurfaceType" ),

END_DATADESC()
