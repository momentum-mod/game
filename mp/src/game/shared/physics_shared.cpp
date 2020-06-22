//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game & Client shared functions moved from physics.cpp
//
//=============================================================================//
#include "cbase.h"
#include "vcollide_parse.h"
#include "filesystem.h"
#include "movevars_shared.h"
#include "engine/ivmodelinfo.h"
#include "physics_shared.h"
#include "solidsetdefaults.h"
#include "model_types.h"
#include "bone_setup.h"
#include "vphysics/object_hash.h"
#include "vphysics/friction.h"
#include "coordsize.h"
#include <KeyValues.h>
#include "decals.h"
#include "IEffects.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "physics_saverestore.h"

#include "utlbuffer.h"
#include "momentum/util/mom_util.h"
#include "icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
IPhysics			*physics = NULL;
IPhysicsObject		*g_PhysWorldObject = NULL;
IPhysicsCollision	*physcollision = NULL;
IPhysicsEnvironment	*physenv = NULL;
#ifdef PORTAL
IPhysicsEnvironment	*physenv_main = NULL;
#endif
IPhysicsSurfaceProps *physprops = NULL;
// UNDONE: This hash holds both entity & IPhysicsObject pointer pairs
// UNDONE: Split into separate hashes?
IPhysicsObjectPairHash *g_EntityCollisionHash = NULL;

#define SURFACEPROP_MANIFEST_FILE "scripts/surfaceproperties_manifest.txt"

const objectparams_t g_PhysDefaultObjectParams =
{
	NULL,
	1.0, //mass
	1.0, // inertia
	0.1f, // damping
	0.1f, // rotdamping
	0.05f, // rotIntertiaLimit
	"DEFAULT",
	NULL,// game data
	0.f, // volume (leave 0 if you don't have one or call physcollision->CollideVolume() to compute it)
	1.0f, // drag coefficient
	true,// enable collisions?
};


void CSolidSetDefaults::ParseKeyValue( void *pData, const char *pKey, const char *pValue )
{
	if ( !Q_stricmp( pKey, "contents" ) )
	{
		m_contentsMask = atoi( pValue );
	}
}

void CSolidSetDefaults::SetDefaults( void *pData )
{
	solid_t *pSolid = (solid_t *)pData;
	pSolid->params = g_PhysDefaultObjectParams;
	m_contentsMask = CONTENTS_SOLID;
}

CSolidSetDefaults g_SolidSetup;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &mins - 
//			&maxs - 
// Output : CPhysCollide
//-----------------------------------------------------------------------------
CPhysCollide *PhysCreateBbox( const Vector &minsIn, const Vector &maxsIn )
{
	// UNDONE: Track down why this causes errors for the player controller and adjust/enable
	//float radius = 0.5 - DIST_EPSILON;
	Vector mins = minsIn;// + Vector(radius, radius, radius);
	Vector maxs = maxsIn;// - Vector(radius, radius, radius);

	// VPHYSICS caches/cleans up these
	CPhysCollide *pResult = physcollision->BBoxToCollide( mins, maxs );

	g_pPhysSaveRestoreManager->NoteBBox( mins, maxs, pResult );
	
	return pResult;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//			&mins - 
//			&maxs - 
//			&origin - 
//			isStatic - 
// Output : static IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysModelCreateBox( CBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, bool isStatic )
{
	int modelIndex = pEntity->GetModelIndex();
	const char *pSurfaceProps = "flesh";
	solid_t solid;
	PhysGetDefaultAABBSolid( solid );
	Vector dims = maxs - mins;
	solid.params.volume = dims.x * dims.y * dims.z;

	if ( modelIndex )
	{
		const model_t *model = modelinfo->GetModel( modelIndex );
		if ( model )
		{
			CStudioHdr studioHdr( modelinfo->GetStudiomodel( model ), mdlcache );
			if ( studioHdr.IsValid() )
			{
				pSurfaceProps = Studio_GetDefaultSurfaceProps( &studioHdr );
			}
		}
	}
	Q_strncpy( solid.surfaceprop, pSurfaceProps, sizeof( solid.surfaceprop ) );

	CPhysCollide *pCollide = PhysCreateBbox( mins, maxs );
	if ( !pCollide )
		return NULL;
	
	return PhysModelCreateCustom( pEntity, pCollide, origin, vec3_angle, STRING(pEntity->GetModelName()), isStatic, &solid );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//			&mins - 
//			&maxs - 
//			&origin - 
//			isStatic - 
// Output : static IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysModelCreateOBB( CBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, const QAngle &angle, bool isStatic )
{
	int modelIndex = pEntity->GetModelIndex();
	const char *pSurfaceProps = "flesh";
	solid_t solid;
	PhysGetDefaultAABBSolid( solid );
	Vector dims = maxs - mins;
	solid.params.volume = dims.x * dims.y * dims.z;

	if ( modelIndex )
	{
		const model_t *model = modelinfo->GetModel( modelIndex );
		if ( model )
		{
			CStudioHdr studioHdr( modelinfo->GetStudiomodel( model ), mdlcache );
			if (studioHdr.IsValid()) 
			{
				pSurfaceProps = Studio_GetDefaultSurfaceProps( &studioHdr );
			}
		}
	}
	Q_strncpy( solid.surfaceprop, pSurfaceProps, sizeof( solid.surfaceprop ) );

	CPhysCollide *pCollide = PhysCreateBbox( mins, maxs );
	if ( !pCollide )
		return NULL;
	
	return PhysModelCreateCustom( pEntity, pCollide, origin, angle, STRING(pEntity->GetModelName()), isStatic, &solid );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &solid - 
//			*pEntity - 
//			modelIndex - 
//			solidIndex - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PhysModelParseSolidByIndex( solid_t &solid, CBaseEntity *pEntity, int modelIndex, int solidIndex )
{
	vcollide_t *pCollide = modelinfo->GetVCollide( modelIndex );
	if ( !pCollide )
		return false;

	bool parsed = false;

	memset( &solid, 0, sizeof(solid) );
	solid.params = g_PhysDefaultObjectParams;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( pCollide->pKeyValues );
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "solid" ) )
		{
			solid_t tmpSolid;
			memset( &tmpSolid, 0, sizeof(tmpSolid) );
			tmpSolid.params = g_PhysDefaultObjectParams;

			pParse->ParseSolid( &tmpSolid, &g_SolidSetup );

			if ( solidIndex < 0 || tmpSolid.index == solidIndex )
			{
				parsed = true;
				solid = tmpSolid;
				// just to be sure we aren't ever getting a non-zero solid by accident
				Assert( solidIndex >= 0 || solid.index == 0 );
				break;
			}
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );

	// collisions are off by default
	solid.params.enableCollisions = true;

	solid.params.pGameData = static_cast<void *>(pEntity);
	solid.params.pName = STRING(pEntity->GetModelName());
	return parsed;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &solid - 
//			*pEntity - 
//			modelIndex - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PhysModelParseSolid( solid_t &solid, CBaseEntity *pEntity, int modelIndex )
{
	return PhysModelParseSolidByIndex( solid, pEntity, modelIndex, -1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &solid - 
//			*pEntity - 
//			*pCollide - 
//			solidIndex - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PhysModelParseSolidByIndex( solid_t &solid, CBaseEntity *pEntity, vcollide_t *pCollide, int solidIndex )
{
	bool parsed = false;

	memset( &solid, 0, sizeof(solid) );
	solid.params = g_PhysDefaultObjectParams;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( pCollide->pKeyValues );
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "solid" ) )
		{
			solid_t tmpSolid;
			memset( &tmpSolid, 0, sizeof(tmpSolid) );
			tmpSolid.params = g_PhysDefaultObjectParams;

			pParse->ParseSolid( &tmpSolid, &g_SolidSetup );

			if ( solidIndex < 0 || tmpSolid.index == solidIndex )
			{
				parsed = true;
				solid = tmpSolid;
				// just to be sure we aren't ever getting a non-zero solid by accident
				Assert( solidIndex >= 0 || solid.index == 0 );
				break;
			}
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );

	// collisions are off by default
	solid.params.enableCollisions = true;

	solid.params.pGameData = static_cast<void *>(pEntity);
	solid.params.pName = STRING(pEntity->GetModelName());
	return parsed;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//			modelIndex - 
//			&origin - 
//			&angles - 
//			*pSolid - 
// Output : IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysModelCreate( CBaseEntity *pEntity, int modelIndex, const Vector &origin, const QAngle &angles, solid_t *pSolid )
{
	if ( !physenv )
		return NULL;

	vcollide_t *pCollide = modelinfo->GetVCollide( modelIndex );
	if ( !pCollide || !pCollide->solidCount )
		return NULL;
	
	solid_t tmpSolid;
	if ( !pSolid )
	{
		pSolid = &tmpSolid;
		if ( !PhysModelParseSolidByIndex( tmpSolid, pEntity, pCollide, -1 ) )
			return NULL;
	}

	int surfaceProp = -1;
	if ( pSolid->surfaceprop[0] )
	{
		surfaceProp = physprops->GetSurfaceIndex( pSolid->surfaceprop );
	}
	IPhysicsObject *pObject = physenv->CreatePolyObject( pCollide->solids[pSolid->index], surfaceProp, origin, angles, &pSolid->params );
	//PhysCheckAdd( pObject, STRING(pEntity->m_iClassname) );

	if ( pObject )
	{
		if ( modelinfo->GetModelType(modelinfo->GetModel(modelIndex)) == mod_brush )
		{
			unsigned int contents = modelinfo->GetModelContents( modelIndex );
			Assert(contents!=0);
			// HACKHACK: contents is used to filter collisions
			// HACKHACK: So keep solid on for water brushes since they should pass collision rules (as triggers)
			if ( contents & MASK_WATER )
			{
				contents |= CONTENTS_SOLID;
			}
			if ( contents != pObject->GetContents() && contents != 0 )
			{
				pObject->SetContents( contents );
				pObject->RecheckCollisionFilter();
			}
		}

		g_pPhysSaveRestoreManager->AssociateModel( pObject, modelIndex);
	}

	return pObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//			modelIndex - 
//			&origin - 
//			&angles - 
// Output : IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysModelCreateUnmoveable( CBaseEntity *pEntity, int modelIndex, const Vector &origin, const QAngle &angles )
{
	if ( !physenv )
		return NULL;

	vcollide_t *pCollide = modelinfo->GetVCollide( modelIndex );
	if ( !pCollide || !pCollide->solidCount )
		return NULL;

	solid_t solid;

	if ( !PhysModelParseSolidByIndex( solid, pEntity, pCollide, -1 ) )
		return NULL;

	// collisions are off by default
	solid.params.enableCollisions = true;
	//solid.params.mass = 1.0;
	int surfaceProp = -1;
	if ( solid.surfaceprop[0] )
	{
		surfaceProp = physprops->GetSurfaceIndex( solid.surfaceprop );
	}
	solid.params.pGameData = static_cast<void *>(pEntity);
	solid.params.pName = STRING(pEntity->GetModelName());
	IPhysicsObject *pObject = physenv->CreatePolyObjectStatic( pCollide->solids[0], surfaceProp, origin, angles, &solid.params );

	//PhysCheckAdd( pObject, STRING(pEntity->m_iClassname) );
	if ( pObject )
	{
		if ( modelinfo->GetModelType(modelinfo->GetModel(modelIndex)) == mod_brush )
		{
			unsigned int contents = modelinfo->GetModelContents( modelIndex );
			Assert(contents!=0);
			if ( contents != pObject->GetContents() && contents != 0 )
			{
				pObject->SetContents( contents );
				pObject->RecheckCollisionFilter();
			}
		}
		g_pPhysSaveRestoreManager->AssociateModel( pObject, modelIndex);
	}

	return pObject;
}


//-----------------------------------------------------------------------------
// Purpose: Create a vphysics object based on an existing collision model
// Input  : *pEntity - 
//			*pModel - 
//			&origin - 
//			&angles - 
//			*pName - 
//			isStatic - 
//			*pSolid - 
// Output : IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysModelCreateCustom( CBaseEntity *pEntity, const CPhysCollide *pModel, const Vector &origin, const QAngle &angles, const char *pName, bool isStatic, solid_t *pSolid )
{
	if ( !physenv )
		return NULL;

	solid_t tmpSolid;
	if ( !pSolid )
	{
		PhysGetDefaultAABBSolid( tmpSolid );
		pSolid = &tmpSolid;
	}
	int surfaceProp = physprops->GetSurfaceIndex( pSolid->surfaceprop );
	pSolid->params.pGameData = static_cast<void *>(pEntity);
	pSolid->params.pName = pName;
	IPhysicsObject *pObject = NULL;
	if ( isStatic )
	{
		pObject = physenv->CreatePolyObjectStatic( pModel, surfaceProp, origin, angles, &pSolid->params );
	}
	else
	{
		pObject = physenv->CreatePolyObject( pModel, surfaceProp, origin, angles, &pSolid->params );
	}

	if ( pObject )
		g_pPhysSaveRestoreManager->AssociateModel( pObject, pModel);

	return pObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//			radius - 
//			&origin - 
//			&solid - 
// Output : IPhysicsObject
//-----------------------------------------------------------------------------
IPhysicsObject *PhysSphereCreate( CBaseEntity *pEntity, float radius, const Vector &origin, solid_t &solid )
{
	if ( !physenv )
		return NULL;

	int surfaceProp = -1;
	if ( solid.surfaceprop[0] )
	{
		surfaceProp = physprops->GetSurfaceIndex( solid.surfaceprop );
	}

	solid.params.pGameData = static_cast<void *>(pEntity);
	IPhysicsObject *pObject = physenv->CreateSphereObject( radius, surfaceProp, origin, vec3_angle, &solid.params, false );

	return pObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PhysGetDefaultAABBSolid( solid_t &solid )
{
	solid.params = g_PhysDefaultObjectParams;
	solid.params.mass = 85.0f;
	solid.params.inertia = 1e24f;
	Q_strncpy( solid.surfaceprop, "default", sizeof( solid.surfaceprop ) );
}

//-----------------------------------------------------------------------------
// Purpose: Destroy a physics object
// Input  : *pObject - 
//-----------------------------------------------------------------------------
void PhysDestroyObject( IPhysicsObject *pObject, CBaseEntity *pEntity )
{
	g_pPhysSaveRestoreManager->ForgetModel( pObject );

	
	if ( pObject )
		pObject->SetGameData( NULL );

	g_EntityCollisionHash->RemoveAllPairsForObject( pObject );
	if ( pEntity && pEntity->IsMarkedForDeletion() )
	{
		g_EntityCollisionHash->RemoveAllPairsForObject( pEntity );
	}

	if ( physenv )
	{
		physenv->DestroyObject( pObject );
	}
}

#define SP(propName) pKey = propsKV->CreateNewKey(); pKey->SetName(propName);
#define SP_KV(key, value) pKey->SetString(key, value);

void GetSurfaceProperties(CUtlBuffer &out)
{
	KeyValuesAD propsKV("surfaceprops");

	KeyValues *pKey = nullptr;

	SP("default");
	{
		SP_KV("density", "2000");
		SP_KV("elasticity", "0.25");
		SP_KV("friction", "0.8");
		SP_KV("dampening", "0.0");

		SP_KV("stepleft", "Default.StepLeft");
		SP_KV("stepright", "Default.StepRight");
		SP_KV("bulletimpact", "Default.BulletImpact");
		SP_KV("scraperough", "Default.ScrapeRough");
		SP_KV("scrapesmooth", "Default.ScrapeSmooth");
		SP_KV("impacthard", "Default.ImpactHard");
		SP_KV("impactsoft", "Default.ImpactSoft");

		SP_KV("audioreflectivity", "0.66");
		SP_KV("audiohardnessfactor", "1.0");
		SP_KV("audioroughnessfactor", "1.0");

		SP_KV("scrapeRoughThreshold", "0.5");
		SP_KV("impactHardThreshold", "0.5");

		SP_KV("gamematerial", "C");
		SP_KV("jumpfactor", "1.0");
		SP_KV("maxspeedfactor", "1.0");
		SP_KV("climbable", "0");
	}
	SP("solidmetal");
	{
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.1");
		SP_KV("audioreflectivity", "0.83");
		SP_KV("friction", "0.8");
		SP_KV("stepleft", "SolidMetal.StepLeft");
		SP_KV("stepright", "SolidMetal.StepRight");
		SP_KV("impacthard", "SolidMetal.ImpactHard");
		SP_KV("impactsoft", "SolidMetal.ImpactSoft");
		SP_KV("scraperough", "SolidMetal.ScrapeRough");
		SP_KV("scrapesmooth", "SolidMetal.ScrapeSmooth");
		SP_KV("bulletimpact", "SolidMetal.BulletImpact");

		SP_KV("gamematerial", "M");
	}
	SP("Metal_Box");
	{
		SP_KV("base", "solidmetal");
		SP_KV("thickness", "0.1");

		SP_KV("stepleft", "Metal_Box.StepLeft");
		SP_KV("stepright", "Metal_Box.StepRight");
		SP_KV("bulletimpact", "Metal_Box.BulletImpact");
		SP_KV("scraperough", "Metal_Box.ScrapeRough");
		SP_KV("scrapesmooth", "Metal_Box.ScrapeSmooth");
		SP_KV("impacthard", "Metal_Box.ImpactHard");
		SP_KV("impactsoft", "Metal_Box.ImpactSoft");

		SP_KV("break", "Metal_Box.Break");
	}
	SP("metal");
	{
		SP_KV("base", "solidmetal");
		SP_KV("elasticity", "0.25");
		SP_KV("thickness", "0.1");
	}

	SP("metal_bouncy");
	{
		SP_KV("base", "solidmetal");
		SP_KV("elasticity", "1000");
		SP_KV("friction", "0");
		SP_KV("density", "10000");
	}
	SP("slipperymetal");
	{
		SP_KV("base", "metal");
		SP_KV("friction", "0.1");
		SP_KV("elasticity", "0.15");

		SP_KV("audioreflectivity", "0.83");
		SP_KV("audioroughnessfactor", "0.1");
	}
	SP("metalgrate");
	{
		SP_KV("thickness", "0.5");
		SP_KV("density", "1600");
		SP_KV("elasticity", "0.25");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "MetalGrate.StepLeft");
		SP_KV("stepright", "MetalGrate.StepRight");
		SP_KV("impacthard", "MetalGrate.ImpactHard");
		SP_KV("impactsoft", "MetalGrate.ImpactSoft");
		SP_KV("scraperough", "MetalGrate.ScrapeRough");
		SP_KV("scrapeSmooth", "MetalGrate.ScrapeSmooth");
		SP_KV("bulletimpact", "MetalGrate.BulletImpact");

		SP_KV("audioreflectivity", "0.83");

		SP_KV("gamematerial", "G");
	}
	SP("metalvent");
	{
		SP_KV("base", "metal_box");
		SP_KV("thickness", "0.04");
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.1");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "MetalVent.StepLeft");
		SP_KV("stepright", "MetalVent.StepRight");
		SP_KV("impacthard", "MetalVent.ImpactHard");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("gamematerial", "V");
	}
	SP("metalpanel");
	{
		SP_KV("base", "metal");
		SP_KV("thickness", "0.1");
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.2");
		SP_KV("friction", "0.8");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("gamematerial", "M");
	}

	SP("dirt");
	{
		SP_KV("density", "1600");
		SP_KV("elasticity", "0.01");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Dirt.StepLeft");
		SP_KV("stepright", "Dirt.StepRight");
		SP_KV("impacthard", "Dirt.Impact");
		SP_KV("scraperough", "Dirt.Scrape");
		SP_KV("bulletimpact", "Dirt.BulletImpact");

		SP_KV("audioreflectivity", "0.03");
		SP_KV("audiohardnessfactor", "0.25");

		SP_KV("gamematerial", "D");
	}

	SP("mud");
	{
		SP_KV("base", "dirt");
		SP_KV("friction", "0.6");
		SP_KV("dampening", "6.0");

		SP_KV("stepleft", "Mud.StepLeft");
		SP_KV("stepright", "Mud.StepRight");

		SP_KV("audiohardnessfactor", "0.0");
		SP_KV("audioroughnessfactor", "0.1");
	}

	SP("slipperyslime");
	{
		SP_KV("base", "dirt");
		SP_KV("friction", "0.1");
		SP_KV("jumpfactor", "0.7");

		SP_KV("stepleft", "SlipperySlime.StepLeft");
		SP_KV("stepright", "SlipperySlime.StepRight");

		SP_KV("audiohardnessfactor", "0.0");
		SP_KV("audioroughnessfactor", "0.1");
	}

	SP("grass");
	{
		SP_KV("base", "dirt");
		SP_KV("stepleft", "Grass.StepLeft");
		SP_KV("stepright", "Grass.StepRight");

		SP_KV("gamematerial", "U");
	}

	SP("tile");
	{
		SP_KV("thickness", "0.5");
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.3");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Tile.StepLeft");
		SP_KV("stepright", "Tile.StepRight");

		SP_KV("audioreflectivity", "0.99");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("bulletimpact", "Tile.BulletImpact");
		SP_KV("gamematerial", "T");
	}
	SP("Wood");
	{
		SP_KV("density", "700");
		SP_KV("elasticity", "0.1");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Wood.StepLeft");
		SP_KV("stepright", "Wood.StepRight");
		SP_KV("bulletimpact", "Wood.BulletImpact");
		SP_KV("scraperough", "Wood.ScrapeRough");
		SP_KV("scrapesmooth", "Wood.ScrapeSmooth");
		SP_KV("impacthard", "Wood.ImpactHard");
		SP_KV("impactsoft", "Wood.ImpactSoft");
		SP_KV("break", "Wood.Break");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audiohardnessfactor", "0.25");

		SP_KV("gamematerial", "W");
	}

	SP("Wood_lowdensity");
	{
		SP_KV("base", "Wood");
		SP_KV("density", "300");
	}
	SP("Wood_Box");
	{
		SP_KV("base", "Wood");

		SP_KV("stepleft", "Wood_Box.StepLeft");
		SP_KV("stepright", "Wood_Box.StepRight");
		SP_KV("bulletimpact", "Wood_Box.BulletImpact");
		SP_KV("scraperough", "Wood_Box.ScrapeRough");
		SP_KV("scrapesmooth", "Wood_Box.ScrapeSmooth");
		SP_KV("impacthard", "Wood_Box.ImpactHard");
		SP_KV("impactsoft", "Wood_Box.ImpactSoft");
		SP_KV("break", "Wood_Box.Break");

	}
	SP("Wood_Crate");
	{
		SP_KV("base", "Wood");

		SP_KV("stepleft", "Wood_Crate.StepLeft");
		SP_KV("stepright", "Wood_Crate.StepRight");
		SP_KV("scraperough", "Wood_Crate.ScrapeRough");
		SP_KV("scrapesmooth", "Wood_Crate.ScrapeSmooth");
		SP_KV("impacthard", "Wood_Crate.ImpactHard");
		SP_KV("impactsoft", "Wood_Crate.ImpactSoft");
		SP_KV("break", "Wood_Crate.Break");

	}
	SP("Wood_Plank");
	{
		SP_KV("base", "Wood_Box");

		SP_KV("bulletimpact", "Wood_Plank.BulletImpact");
		SP_KV("scraperough", "Wood_Plank.ScrapeRough");
		SP_KV("scrapesmooth", "Wood_Plank.ScrapeSmooth");
		SP_KV("impacthard", "Wood_Plank.ImpactHard");
		SP_KV("impactsoft", "Wood_Plank.ImpactSoft");
		SP_KV("break", "Wood_Plank.Break");
	}
	SP("Wood_Solid");
	{
		SP_KV("base", "Wood");

		SP_KV("bulletimpact", "Wood_Solid.BulletImpact");
		SP_KV("scraperough", "Wood_Solid.ScrapeRough");
		SP_KV("scrapesmooth", "Wood_Solid.ScrapeSmooth");
		SP_KV("impacthard", "Wood_Solid.ImpactHard");
		SP_KV("impactsoft", "Wood_Solid.ImpactSoft");
		SP_KV("break", "Wood_Solid.Break");
	}
	SP("Wood_Furniture");
	{
		SP_KV("base", "Wood_Box");
		SP_KV("impactsoft", "Wood_Furniture.ImpactSoft");
		SP_KV("break", "Wood_Furniture.Break");
	}
	SP("Wood_Panel");
	{
		SP_KV("base", "Wood_Crate");
		SP_KV("thickness", "1.0");

		SP_KV("stepleft", "Wood_Panel.StepLeft");
		SP_KV("stepright", "Wood_Panel.StepRight");
		SP_KV("bulletimpact", "Wood_Panel.BulletImpact");
		SP_KV("scraperough", "Wood_Panel.ScrapeRough");
		SP_KV("scrapesmooth", "Wood_Panel.ScrapeSmooth");
		SP_KV("impacthard", "Wood_Panel.ImpactHard");
		SP_KV("impactsoft", "Wood_Panel.ImpactSoft");
		SP_KV("break", "Wood_Panel.Break");
	}
	SP("water");
	{
		SP_KV("density", "1000");
		SP_KV("elasticity", "0.1");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Water.StepLeft");
		SP_KV("stepright", "Water.StepRight");
		SP_KV("bulletimpact", "Water.BulletImpact");

		SP_KV("impacthard", "Water.ImpactHard");
		SP_KV("impactsoft", "Water.ImpactSoft");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audioroughnessfactor", "0.1");
		SP_KV("audiohardnessfactor", "0.0");

		SP_KV("gamematerial", "S");
	}
	SP("slime");
	{
		SP_KV("density", "2000");
		SP_KV("elasticity", "0.1");
		SP_KV("friction", "0.9");
		SP_KV("dampening", "200.0");

		SP_KV("stepleft", "Mud.StepLeft");
		SP_KV("stepright", "Mud.StepRight");
		SP_KV("bulletimpact", "Water.BulletImpact");

		SP_KV("gamematerial", "S");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audiohardnessfactor", "0.0");
		SP_KV("audioroughnessfactor", "0.1");
	}
	SP("quicksand");
	{
		SP_KV("density", "600");
		SP_KV("elasticity", "2.0");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audiohardnessfactor", "0.0");
		SP_KV("audioroughnessfactor", "1.0");

		SP_KV("gamematerial", "N");
	}
	SP("wade");
	{
		SP_KV("base", "water");
		SP_KV("stepleft", "Wade.StepLeft");
		SP_KV("stepright", "Wade.StepRight");

		SP_KV("audioreflectivity", "0.33");

		SP_KV("gamematerial", "X");
	}
	SP("ladder");
	{
		SP_KV("base", "metal");
		SP_KV("climbable", "1.0");
		SP_KV("stepleft", "Ladder.StepLeft");
		SP_KV("stepright", "Ladder.StepRight");

		SP_KV("audioreflectivity", "0.33");

		SP_KV("gamematerial", "X");
	}
	SP("woodladder");
	{
		SP_KV("base", "wood");
		SP_KV("climbable", "1.0");
		SP_KV("stepleft", "Ladder.WoodStepLeft");
		SP_KV("stepright", "Ladder.WoodStepRight");

		SP_KV("audioreflectivity", "0.33");

		SP_KV("gamematerial", "X");
	}
	SP("glass");
	{
		SP_KV("thickness", "0.5");
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.2");
		SP_KV("friction", "0.5");

		SP_KV("stepleft", "Glass.StepLeft");
		SP_KV("stepright", "Glass.StepRight");
		SP_KV("scraperough", "Glass.ScrapeRough");
		SP_KV("scrapesmooth", "Glass.ScrapeSmooth");
		SP_KV("impacthard", "Glass.ImpactHard");
		SP_KV("impactsoft", "Glass.ImpactSoft");

		SP_KV("bulletimpact", "Glass.BulletImpact");
		SP_KV("break", "Glass.Break");

		SP_KV("audioreflectivity", "0.66");
		SP_KV("audiohardnessfactor", "1.0");

		SP_KV("audioroughnessfactor", "0.0");
		SP_KV("gamematerial", "Y");
	}
	SP("computer");
	{
		SP_KV("base", "metal_box");

		SP_KV("bulletimpact", "Computer.BulletImpact");
		SP_KV("impacthard", "Computer.ImpactHard");
		SP_KV("impactsoft", "Computer.ImpactSoft");

		SP_KV("gamematerial", "P");
	}
	SP("concrete");
	{
		SP_KV("density", "2400");
		SP_KV("elasticity", "0.2");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Concrete.StepLeft");
		SP_KV("stepright", "Concrete.StepRight");
		SP_KV("scraperough", "Concrete.ScrapeRough");
		SP_KV("scrapesmooth", "Concrete.ScrapeSmooth");
		SP_KV("impacthard", "Concrete.ImpactHard");
		SP_KV("impactsoft", "Concrete.ImpactSoft");
		SP_KV("bulletimpact", "Concrete.BulletImpact");

		SP_KV("audioreflectivity", "0.66");

		SP_KV("gamematerial", "C");
	}
	SP("rock");
	{
		SP_KV("base", "concrete");
		SP_KV("impacthard", "Rock.ImpactHard");
		SP_KV("impactsoft", "Rock.ImpactSoft");
		SP_KV("scraperough", "Rock.ImpactHard");
		SP_KV("scrapesmooth", "Rock.ImpactSoft");

	}
	SP("porcelain");
	{
		SP_KV("base", "rock");
	}
	SP("boulder");
	{
		SP_KV("base", "rock");
		SP_KV("scraperough", "Boulder.ScrapeRough");
		SP_KV("scrapesmooth", "Boulder.ScrapeSmooth");
		SP_KV("impacthard", "Boulder.ImpactHard");
		SP_KV("impactsoft", "Boulder.ImpactSoft");
	}
	SP("gravel");
	{
		SP_KV("base", "rock");
		SP_KV("friction", "0.8");
		SP_KV("stepleft", "Gravel.StepLeft");
		SP_KV("stepright", "Gravel.StepRight");
	}

	SP("brick");
	{
		SP_KV("base", "rock");
	}
	SP("concrete_block");
	{

		SP_KV("base", "concrete");
		SP_KV("impacthard", "Concrete_Block.ImpactHard");
	}
	SP("chainlink");
	{
		SP_KV("thickness", "0.5");
		SP_KV("density", "1600");
		SP_KV("elasticity", "0.25");
		SP_KV("friction", "0.8");
		SP_KV("stepleft", "ChainLink.StepLeft");
		SP_KV("stepright", "ChainLink.StepRight");
		SP_KV("impacthard", "ChainLink.ImpactHard");
		SP_KV("impactsoft", "ChainLink.ImpactSoft");
		SP_KV("scraperough", "ChainLink.ScrapeRough");
		SP_KV("scrapesmooth", "ChainLink.ScrapeSmooth");
		SP_KV("bulletimpact", "ChainLink.BulletImpact");
		SP_KV("gamematerial", "G");
	}
	SP("chain");
	{
		SP_KV("base", "chainlink");
		SP_KV("impacthard", "ChainLink.ImpactHard");
		SP_KV("impactsoft", "ChainLink.ImpactSoft");
		SP_KV("scraperough", "ChainLink.ScrapeRough");
		SP_KV("scrapesmooth", "ChainLink.ScrapeSmooth");
		SP_KV("bulletimpact", "ChainLink.BulletImpact");
		SP_KV("gamematerial", "G");
	}
	SP("flesh");
	{
		SP_KV("density", "900");

		SP_KV("stepleft", "Flesh.StepLeft");
		SP_KV("stepright", "Flesh.StepRight");
		SP_KV("bulletimpact", "Flesh.BulletImpact");
		SP_KV("impacthard", "Flesh.ImpactHard");
		SP_KV("impactsoft", "Flesh.ImpactSoft");
		SP_KV("scraperough", "Flesh.ScrapeRough");
		SP_KV("scrapesmooth", "Flesh.ScrapeSmooth");
		SP_KV("break", "Flesh.Break");

		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioHardMinVelocity", "500");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("gamematerial", "F");
	}

	SP("bloodyflesh");
	{
		SP_KV("base", "flesh");

		SP_KV("impacthard", "Flesh_Bloody.ImpactHard");

		SP_KV("gamematerial", "B");
	}

	SP("alienflesh");
	{
		SP_KV("base", "flesh");

		SP_KV("gamematerial", "H");
	}

	SP("armorflesh");
	{
		SP_KV("base", "flesh");
		SP_KV("bulletimpact", "ArmorFlesh.BulletImpact");

		SP_KV("audiohardnessfactor", "1.0");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("gamematerial", "M");
	}

	SP("watermelon");
	{
		SP_KV("density", "900");
		SP_KV("bulletimpact", "Watermelon.BulletImpact");
		SP_KV("impacthard", "Watermelon.Impact");
		SP_KV("scraperough", "Watermelon.Scrape");

		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("gamematerial", "W");
	}

	SP("snow");
	{
		SP_KV("base", "dirt");
		SP_KV("density", "800");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Snow.StepLeft");
		SP_KV("stepright", "Snow.StepRight");

		SP_KV("audiohardnessfactor", "0.25");

		SP_KV("gamematerial", "J");
	}

	SP("ice");
	{
		SP_KV("density", "917");
		SP_KV("friction", "0.1");
		SP_KV("elasticity", "0.1");

		SP_KV("audioroughnessfactor", "0.1");
	}
	SP("carpet");
	{
		SP_KV("base", "dirt");
		SP_KV("density", "500");
		SP_KV("thickness", "0.1");
		SP_KV("elasticity", "0.01");
		SP_KV("friction", "0.8");

		SP_KV("impacthard", "Carpet.Impact");
		SP_KV("bulletimpact", "Carpet.BulletImpact");
		SP_KV("scraperough", "Carpet.Scrape");

		SP_KV("audioreflectivity", "0.03");
		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioroughnessfactor", "0.1");
	}
	SP("plaster");
	{
		SP_KV("base", "dirt");
		SP_KV("audiohardnessfactor", "0.5");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("stepleft", "drywall.StepLeft");
		SP_KV("stepright", "drywall.StepRight");
		SP_KV("bulletimpact", "drywall.ImpactHard");
		SP_KV("scraperough", "ceiling_tile.ScrapeRough");
		SP_KV("scrapesmooth", "ceiling_tile.ScrapeSmooth");
		SP_KV("impacthard", "drywall.ImpactHard");
		SP_KV("impactsoft", "drywall.ImpactSoft");
		SP_KV("break", "Cardboard.Break");

	}
	SP("cardboard");
	{
		SP_KV("base", "dirt");
		SP_KV("density", "500");
		SP_KV("thickness", "0.25");

		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioroughnessfactor", "0.25");

		SP_KV("stepleft", "Cardboard.StepLeft");
		SP_KV("stepright", "Cardboard.StepRight");
		SP_KV("bulletimpact", "Cardboard.BulletImpact");
		SP_KV("scraperough", "Cardboard.ScrapeRough");
		SP_KV("scrapesmooth", "Cardboard.ScrapeSmooth");
		SP_KV("impacthard", "Cardboard.ImpactHard");
		SP_KV("impactsoft", "Cardboard.ImpactSoft");
		SP_KV("break", "Cardboard.Break");
	}
	SP("plastic_barrel");
	{
		SP_KV("density", "500");
		SP_KV("thickness", "0.25");
		SP_KV("elasticity", "0.01");
		SP_KV("friction", "0.8");

		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioroughnessfactor", "0.25");

		SP_KV("stepleft", "Plastic_Barrel.StepLeft");
		SP_KV("stepright", "Plastic_Barrel.StepRight");
		SP_KV("bulletimpact", "Plastic_Barrel.BulletImpact");
		SP_KV("scraperough", "Plastic_Barrel.ScrapeRough");
		SP_KV("scrapesmooth", "Plastic_Barrel.ScrapeSmooth");
		SP_KV("impacthard", "Plastic_Barrel.ImpactHard");
		SP_KV("impactsoft", "Plastic_Barrel.ImpactSoft");
		SP_KV("break", "Plastic_Barrel.Break");

		SP_KV("gamematerial", "L");
	}
	SP("Plastic_Box");
	{
		SP_KV("density", "500");
		SP_KV("elasticity", "0.01");
		SP_KV("friction", "0.8");
		SP_KV("thickness", "0.25");

		SP_KV("audiohardnessfactor", "0.25");
		SP_KV("audioroughnessfactor", "0.25");

		SP_KV("stepleft", "Plastic_Box.StepLeft");
		SP_KV("stepright", "Plastic_Box.StepRight");
		SP_KV("bulletimpact", "Plastic_Box.BulletImpact");
		SP_KV("scraperough", "Plastic_Box.ScrapeRough");
		SP_KV("scrapesmooth", "Plastic_Box.ScrapeSmooth");
		SP_KV("impacthard", "Plastic_Box.ImpactHard");
		SP_KV("impactsoft", "Plastic_Box.ImpactSoft");
		SP_KV("break", "Plastic_Box.Break");

		SP_KV("gamematerial", "L");
	}
	SP("plastic");
	{
		SP_KV("base", "Plastic_Box");
		SP_KV("audioroughnessfactor", "0.1");

		SP_KV("bulletimpact", "Plastic_Box.ImpactHard");
	}
	SP("item");
	{
		SP_KV("base", "Plastic_Box");
		SP_KV("density", "600");

		SP_KV("bulletimpact", "Plastic_Box.ImpactHard");
	}
	SP("floatingstandable");
	{
		SP_KV("base", "dirt");
		SP_KV("density", "800");
	}
	SP("sand");
	{
		SP_KV("base", "dirt");
		SP_KV("stepleft", "Sand.StepLeft");
		SP_KV("stepright", "Sand.StepRight");
		SP_KV("bulletimpact", "Sand.BulletImpact");

		SP_KV("audioreflectivity", "0.03");

		SP_KV("gamematerial", "N");
	}
	SP("rubber");
	{
		SP_KV("base", "dirt");
		SP_KV("elasticity", "0.2");
		SP_KV("friction", "0.8");

		SP_KV("stepleft", "Rubber.StepLeft");
		SP_KV("stepright", "Rubber.StepRight");
		SP_KV("impacthard", "Rubber.ImpactHard");
		SP_KV("impactsoft", "Rubber.ImpactSoft");
		SP_KV("bulletimpact", "Rubber.BulletImpact");

		SP_KV("audioroughnessfactor", "0.1");
		SP_KV("audiohardnessfactor", "0.2");

	}
	SP("rubbertire");
	{
		SP_KV("base", "rubber");

		SP_KV("bulletimpact", "Rubber_Tire.BulletImpact");
		SP_KV("impacthard", "Rubber_Tire.ImpactHard");
		SP_KV("impactsoft", "Rubber_Tire.ImpactSoft");
		SP_KV("friction", "1.0");
	}
	SP("jeeptire");
	{
		SP_KV("base", "rubber");

		SP_KV("bulletimpact", "Rubber_Tire.BulletImpact");
		SP_KV("impacthard", "Rubber_Tire.ImpactHard");
		SP_KV("impactsoft", "Rubber_Tire.ImpactSoft");
		SP_KV("friction", "1.337");
	}

	SP("slidingrubbertire");
	{
		SP_KV("base", "rubber");
		SP_KV("friction", "0.2");
	}

	SP("brakingrubbertire");
	{
		SP_KV("base", "rubber");
		SP_KV("friction", "0.6");
	}

	SP("slidingrubbertire_front");
	{
		SP_KV("base", "rubber");
		SP_KV("friction", "0.2");
	}

	SP("slidingrubbertire_rear");
	{
		SP_KV("base", "rubber");
		SP_KV("friction", "0.2");
	}
	SP("glassbottle");
	{
		SP_KV("base", "glass");
		SP_KV("friction", "0.4");
		SP_KV("elasticity", "0.3");

		SP_KV("stepleft", "GlassBottle.StepLeft");
		SP_KV("stepright", "GlassBottle.StepRight");
		SP_KV("impacthard", "GlassBottle.ImpactHard");
		SP_KV("impactsoft", "GlassBottle.ImpactSoft");
		SP_KV("scraperough", "GlassBottle.ScrapeRough");
		SP_KV("scrapesmooth", "GlassBottle.ScrapeSmooth");
		SP_KV("bulletimpact", "GlassBottle.BulletImpact");

		SP_KV("break", "GlassBottle.Break");
	}
	SP("pottery");
	{
		SP_KV("base", "glassbottle");
		SP_KV("friction", "0.4");
		SP_KV("elasticity", "0.3");

		SP_KV("impacthard", "Pottery.ImpactHard");
		SP_KV("impactsoft", "Pottery.ImpactSoft");
		SP_KV("bulletimpact", "Pottery.BulletImpact");

		SP_KV("break", "Pottery.Break");
	}
	SP("grenade");
	{
		SP_KV("base", "metalpanel");
		SP_KV("friction", "0.9");
		SP_KV("elasticity", "0.01");


		SP_KV("audiohardnessfactor", "1.0");
		SP_KV("audioroughnessfactor", "0.4");

		SP_KV("stepleft", "Grenade.StepLeft");
		SP_KV("stepright", "Grenade.StepRight");
		SP_KV("bulletimpact", "Grenade.ImpactHard");
		SP_KV("scraperough", "Grenade.ScrapeRough");
		SP_KV("scrapesmooth", "Grenade.ScrapeSmooth");
		SP_KV("impacthard", "Grenade.ImpactHard");
		SP_KV("impactsoft", "Grenade.ImpactSoft");
	}
	SP("canister");
	{
		SP_KV("base", "metalpanel");
		SP_KV("impacthard", "Canister.ImpactHard");
		SP_KV("impactsoft", "Canister.ImpactSoft");
		SP_KV("scraperough", "Canister.ScrapeRough");
		SP_KV("scrapesmooth", "Canister.ScrapeSmooth");
	}
	SP("metal_barrel");
	{
		SP_KV("base", "metal_box");
		SP_KV("impacthard", "Metal_Barrel.ImpactHard");
		SP_KV("impactsoft", "Metal_Barrel.ImpactSoft");
		SP_KV("bulletimpact", "Metal_Barrel.BulletImpact");
	}
	SP("floating_metal_barrel");
	{
		SP_KV("base", "metal_barrel");
		SP_KV("density", "500");
	}
	SP("plastic_barrel_buoyant");
	{
		SP_KV("base", "plastic_barrel");
		SP_KV("density", "150");
	}
	SP("roller");
	{
		SP_KV("base", "metalpanel");
		SP_KV("friction", "0.7");
		SP_KV("elasticity", "0.3");
		SP_KV("impacthard", "Roller.Impact");
	}
	SP("popcan");
	{
		SP_KV("base", "metal_box");
		SP_KV("friction", "0.3");
		SP_KV("elasticity", "0.99");
		SP_KV("impacthard", "Popcan.ImpactHard");
		SP_KV("impactsoft", "Popcan.ImpactSoft");
		SP_KV("scraperough", "Popcan.ScrapeRough");
		SP_KV("scrapesmooth", "Popcan.ScrapeSmooth");
		SP_KV("bulletimpact", "Popcan.BulletImpact");
	}
	SP("paintcan");
	{
		SP_KV("base", "popcan");
		SP_KV("friction", "0.3");
		SP_KV("elasticity", "0.99");
		SP_KV("impacthard", "Paintcan.ImpactHard");
		SP_KV("impactsoft", "Paintcan.ImpactSoft");
	}
	SP("paper");
	{
		SP_KV("base", "cardboard");
	}
	SP("papercup");
	{
		SP_KV("base", "paper");
		SP_KV("friction", "0.8");
		SP_KV("elasticity", "0.1");
		SP_KV("impacthard", "Papercup.Impact");
		SP_KV("scraperough", "Popcan.ScrapeRough");
	}
	SP("ceiling_tile");
	{
		SP_KV("base", "cardboard");

		SP_KV("stepleft", "ceiling_tile.StepLeft");
		SP_KV("stepright", "ceiling_tile.StepRight");
		SP_KV("bulletimpact", "ceiling_tile.BulletImpact");
		SP_KV("scraperough", "ceiling_tile.ScrapeRough");
		SP_KV("scrapesmooth", "ceiling_tile.ScrapeSmooth");
		SP_KV("impacthard", "ceiling_tile.ImpactHard");
		SP_KV("impactsoft", "ceiling_tile.ImpactSoft");

		SP_KV("break", "ceiling_tile.Break");
	}
	SP("weapon");
	{
		SP_KV("base", "metal");
		SP_KV("stepleft", "weapon.StepLeft");
		SP_KV("stepright", "weapon.StepRight");
		SP_KV("bulletimpact", "weapon.BulletImpact");
		SP_KV("scraperough", "weapon.ScrapeRough");
		SP_KV("scrapesmooth", "weapon.ScrapeSmooth");
		SP_KV("impacthard", "weapon.ImpactHard");
		SP_KV("impactsoft", "weapon.ImpactSoft");
	}
	SP("default_silent");
	{
		SP_KV("gamematerial", "X");
	}
	SP("player");
	{
		SP_KV("density", "1000");
		SP_KV("friction", "0.5");
		SP_KV("elasticity", "0.001");
		SP_KV("audiohardnessfactor", "0.0");
		SP_KV("audioroughnessfactor", "0.0");
	}
	SP("player_control_clip");
	{
		SP_KV("gamematerial", "I");
	}
	SP("no_decal");
	{
		SP_KV("density", "900");
		SP_KV("gamematerial", "-");
	}
	SP("foliage");
	{
		SP_KV("base", "Wood_Solid");

		SP_KV("density", "700");
		SP_KV("elasticity", "0.1");
		SP_KV("friction", "0.8");

		SP_KV("gamematerial", "O");
	}
	SP("metalvehicle");
	{
		SP_KV("base", "metal");
		SP_KV("thickness", "0.1");
		SP_KV("density", "2700");
		SP_KV("elasticity", "0.2");
		SP_KV("friction", "0.8");

		SP_KV("audioreflectivity", "0.33");
		SP_KV("audioroughnessfactor", "0.1");
		SP_KV("audioHardMinVelocity", "500");

		SP_KV("impactHardThreshold", "0.5");


		SP_KV("impacthard", "MetalVehicle.ImpactHard");
		SP_KV("impactsoft", "MetalVehicle.ImpactSoft");
		SP_KV("scraperough", "MetalVehicle.ScrapeRough");
		SP_KV("scrapesmooth", "MetalVehicle.ScrapeSmooth");

		SP_KV("gamematerial", "M");
	}
	SP("crowbar");
	{
		SP_KV("base", "metal");
		SP_KV("impactsoft", "Weapon_Crowbar.Melee_HitWorld");
		SP_KV("impacthard", "Weapon_Crowbar.Melee_HitWorld");
	}
	SP("antlionsand");
	{
		SP_KV("base", "sand");
	}

	SP("metal_seafloorcar");
	{
		SP_KV("base", "metal");
		SP_KV("bulletimpact", "Metal_SeafloorCar.BulletImpact");
	}

	SP("gunship");
	{
		SP_KV("base", "metal");
		SP_KV("friction", "0.3");
		SP_KV("impacthard", "Gunship.Impact");
		SP_KV("scraperough", "Gunship.Scrape");
	}

	SP("strider");
	{
		SP_KV("base", "metal");

		SP_KV("impacthard", "Strider.Impact");
		SP_KV("scraperough", "Strider.Scrape");
	}

	SP("antlion");
	{
		SP_KV("base", "alienflesh");

		SP_KV("gamematerial", "A");
	}

	SP("combine_metal");
	{
		SP_KV("base", "solidmetal");
	}

	SP("combine_glass");
	{
		SP_KV("base", "glass");
	}

	SP("zombieflesh");
	{
		SP_KV("base", "flesh");
		SP_KV("impacthard", "Flesh_Bloody.ImpactHard");
	}
	SP("brass_bell_large");
	{
		SP_KV("bulletimpact", "BrassBell.C");
	}

	SP("brass_bell_medium");
	{
		SP_KV("bulletimpact", "BrassBell.D");
	}

	SP("brass_bell_small");
	{
		SP_KV("bulletimpact", "BrassBell.E");
	}

	SP("brass_bell_smallest");
	{
		SP_KV("bulletimpact", "BrassBell.F");
	}

	SP("hay");
	{
		SP_KV("base", "wood");
		SP_KV("stepleft", "Grass.StepLeft");
		SP_KV("stepright", "Grass.StepRight");
	}
	SP("grenade_napalm");
	{
		SP_KV("base", "grenade");
		SP_KV("friction", "0.05");
		SP_KV("elasticity", "2");
	}

	SP("wrecking_ball");
	{
		SP_KV("base", "metal");

		SP_KV("friction", "0");
		SP_KV("elasticity", "0");
	}

	SP("demoman_grenade");
	{
		SP_KV("base", "metal");

		SP_KV("bulletimpact", "Grenade.ImpactHard");
		SP_KV("scraperough", "Grenade.ScrapeRough");
		SP_KV("scrapesmooth", "Grenade.ScrapeSmooth");
		SP_KV("impacthard", "Grenade.ImpactHard");
		SP_KV("impactsoft", "Grenade.ImpactSoft");
		SP_KV("rolling", "Grenade.Roll");
	}

	SP("scout_baseball");
	{
		SP_KV("base", "rubber");

		SP_KV("bulletimpact", "Weapon_Baseball.HitWorld");
		SP_KV("scraperough", "Grenade.ScrapeRough");
		SP_KV("scrapesmooth", "Grenade.ScrapeSmooth");
		SP_KV("impacthard", "Weapon_Baseball.HitWorld");
		SP_KV("impactsoft", "Weapon_Baseball.HitWorld");
		SP_KV("rolling", "Grenade.Roll");
	}

	SP("scout_ornament");
	{
		SP_KV("base", "glass");

		SP_KV("bulletimpact", "BallBuster.Ball_HitWorld");
		SP_KV("impacthard", "BallBuster.Ball_HitWorld");
		SP_KV("impactsoft", "BallBuster.Ball_HitWorld");
	}

	SP("ball_bouncy");
	{
		SP_KV("base", "rubber");

		SP_KV("friction", ".3");
		SP_KV("elasticity", "3");
		SP_KV("density", "500");
	}

	SP("passtime_ball");
	{
		SP_KV("base", "rubber");
		SP_KV("elasticity", "1");
		SP_KV("friction", "1");

		SP_KV("bulletimpact", "Grenade.ImpactHard");
		SP_KV("scraperough", "Grenade.ScrapeRough");
		SP_KV("scrapesmooth", "Grenade.ScrapeSmooth");
		SP_KV("impacthard", "Grenade.ImpactHard");
		SP_KV("impactsoft", "Grenade.ImpactSoft");
		SP_KV("rolling", "Grenade.Roll");
	}

	FOR_EACH_SUBKEY(propsKV, propKV)
	{
		propKV->RecursiveSaveToFile(out, 0);
	}
}

void PhysParseSurfaceData(IPhysicsSurfaceProps *pProps, IFileSystem *pFileSystem)
{
	CUtlBuffer buf;
	buf.SetBufferType(true, true);
	GetSurfaceProperties(buf);
	pProps->ParseSurfaceData("surfaceprops.txt", buf.String());
}

void PhysCreateVirtualTerrain( CBaseEntity *pWorld, const objectparams_t &defaultParams )
{
	if ( !physenv )
		return;

	char nameBuf[1024];
	for ( int i = 0; i < MAX_MAP_DISPINFO; i++ )
	{
		CPhysCollide *pCollide = modelinfo->GetCollideForVirtualTerrain( i );
		if ( pCollide )
		{
			solid_t solid;
			solid.params = defaultParams;
			solid.params.enableCollisions = true;
			solid.params.pGameData = static_cast<void *>(pWorld);
			Q_snprintf(nameBuf, sizeof(nameBuf), "vdisp_%04d", i );
			solid.params.pName = nameBuf;
			int surfaceData = physprops->GetSurfaceIndex( "default" );
			// create this as part of the world
			IPhysicsObject *pObject = physenv->CreatePolyObjectStatic( pCollide, surfaceData, vec3_origin, vec3_angle, &solid.params );
			pObject->SetCallbackFlags( pObject->GetCallbackFlags() | CALLBACK_NEVER_DELETED );
		}
	}
}

IPhysicsObject *PhysCreateWorld_Shared( CBaseEntity *pWorld, vcollide_t *pWorldCollide, const objectparams_t &defaultParams )
{
	solid_t solid;
	fluid_t fluid;

	if ( !physenv )
		return NULL;

	int surfaceData = physprops->GetSurfaceIndex( "default" );

	objectparams_t params = defaultParams;
	params.pGameData = static_cast<void *>(pWorld);
	params.pName = "world";

	IPhysicsObject *pWorldPhysics = physenv->CreatePolyObjectStatic( 
		pWorldCollide->solids[0], surfaceData, vec3_origin, vec3_angle, &params );

	// hint - saves vphysics some work
	pWorldPhysics->SetCallbackFlags( pWorldPhysics->GetCallbackFlags() | CALLBACK_NEVER_DELETED );

	//PhysCheckAdd( world, "World" );
	// walk the world keys in case there are some fluid volumes to create
	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( pWorldCollide->pKeyValues );

	bool bCreateVirtualTerrain = false;
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();

		if ( !strcmpi( pBlock, "solid" ) || !strcmpi( pBlock, "staticsolid" ) )
		{
			solid.params = defaultParams;
			pParse->ParseSolid( &solid, &g_SolidSetup );
			solid.params.enableCollisions = true;
			solid.params.pGameData = static_cast<void *>(pWorld);
			solid.params.pName = "world";

			// already created world above
			if ( solid.index == 0 )
				continue;

			if ( !pWorldCollide->solids[solid.index] )
			{
				// this implies that the collision model is a mopp and the physics DLL doesn't support that.
				bCreateVirtualTerrain = true;
				continue;
			}
			// create this as part of the world
			IPhysicsObject *pObject = physenv->CreatePolyObjectStatic( pWorldCollide->solids[solid.index], 
				surfaceData, vec3_origin, vec3_angle, &solid.params );

			// invalid collision model or can't create, ignore
			if (!pObject)
				continue;

			pObject->SetCallbackFlags( pObject->GetCallbackFlags() | CALLBACK_NEVER_DELETED );
			Assert( g_SolidSetup.GetContentsMask() != 0 );
			pObject->SetContents( g_SolidSetup.GetContentsMask() );

			if ( !pWorldPhysics )
			{
				pWorldPhysics = pObject;
			}
		}
		else if ( !strcmpi( pBlock, "fluid" ) )
		{
			pParse->ParseFluid( &fluid, NULL );

			// create a fluid for floating
			if ( fluid.index > 0 )
			{
				solid.params = defaultParams;	// copy world's params
				solid.params.enableCollisions = true;
				solid.params.pName = "fluid";
				solid.params.pGameData = static_cast<void *>(pWorld);
				fluid.params.pGameData = static_cast<void *>(pWorld);
				int surfaceDataProp = physprops->GetSurfaceIndex( fluid.surfaceprop );
				// create this as part of the world
				IPhysicsObject *pWater = physenv->CreatePolyObjectStatic( pWorldCollide->solids[fluid.index], 
					surfaceDataProp, vec3_origin, vec3_angle, &solid.params );

				pWater->SetCallbackFlags( pWater->GetCallbackFlags() | CALLBACK_NEVER_DELETED );
				physenv->CreateFluidController( pWater, &fluid.params );
			}
		}
		else if ( !strcmpi( pBlock, "materialtable" ) )
		{
			int surfaceTable[128];
			memset( surfaceTable, 0, sizeof(surfaceTable) );

			pParse->ParseSurfaceTable( surfaceTable, NULL );
			physprops->SetWorldMaterialIndexTable( surfaceTable, 128 );
		}
		else if ( !strcmpi(pBlock, "virtualterrain" ) )
		{
			bCreateVirtualTerrain = true;
			pParse->SkipBlock();
		}
		else
		{
			// unknown chunk???
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );

	if ( bCreateVirtualTerrain && physcollision->SupportsVirtualMesh() )
	{
		PhysCreateVirtualTerrain( pWorld, defaultParams );
	}
	return pWorldPhysics;
}


//=============================================================================
//
// Physics Game Trace
//
class CPhysicsGameTrace : public IPhysicsGameTrace
{
public:

	void VehicleTraceRay( const Ray_t &ray, void *pVehicle, trace_t *pTrace );
	void VehicleTraceRayWithWater( const Ray_t &ray, void *pVehicle, trace_t *pTrace );
	bool VehiclePointInWater( const Vector &vecPoint );
};

CPhysicsGameTrace g_PhysGameTrace;
IPhysicsGameTrace *physgametrace = &g_PhysGameTrace;

//-----------------------------------------------------------------------------
// Purpose: Game ray-traces in vphysics.
//-----------------------------------------------------------------------------
void CPhysicsGameTrace::VehicleTraceRay( const Ray_t &ray, void *pVehicle, trace_t *pTrace )
{
	CBaseEntity *pBaseEntity = static_cast<CBaseEntity*>( pVehicle );
	UTIL_TraceRay( ray, MASK_SOLID, pBaseEntity, COLLISION_GROUP_NONE, pTrace );
}

//-----------------------------------------------------------------------------
// Purpose: Game ray-traces in vphysics.
//-----------------------------------------------------------------------------
void CPhysicsGameTrace::VehicleTraceRayWithWater( const Ray_t &ray, void *pVehicle, trace_t *pTrace )
{
	CBaseEntity *pBaseEntity = static_cast<CBaseEntity*>( pVehicle );
	UTIL_TraceRay( ray, MASK_SOLID|MASK_WATER, pBaseEntity, COLLISION_GROUP_NONE, pTrace );
}

//-----------------------------------------------------------------------------
// Purpose: Test to see if a vehicle point is in water.
//-----------------------------------------------------------------------------
bool CPhysicsGameTrace::VehiclePointInWater( const Vector &vecPoint )
{
	return ( ( UTIL_PointContents( vecPoint ) & MASK_WATER ) != 0 );
}

void PhysRecheckObjectPair( IPhysicsObject *pObject0, IPhysicsObject *pObject1 )
{
	if ( !pObject0->IsStatic() )
	{
		pObject0->RecheckCollisionFilter();
	}
	if ( !pObject1->IsStatic() )
	{
		pObject1->RecheckCollisionFilter();
	}
}

void PhysEnableEntityCollisions( IPhysicsObject *pObject0, IPhysicsObject *pObject1 )
{
	if ( !pObject0 || !pObject1 )
		return;

	g_EntityCollisionHash->RemoveObjectPair( pObject0->GetGameData(), pObject1->GetGameData() );
	PhysRecheckObjectPair( pObject0, pObject1 );
}

// disables collisions between entities (each entity may contain multiple objects)
void PhysDisableEntityCollisions( IPhysicsObject *pObject0, IPhysicsObject *pObject1 )
{
	if ( !pObject0 || !pObject1 )
		return;

	g_EntityCollisionHash->AddObjectPair( pObject0->GetGameData(), pObject1->GetGameData() );
	PhysRecheckObjectPair( pObject0, pObject1 );
}

void PhysDisableEntityCollisions( CBaseEntity *pEntity0, CBaseEntity *pEntity1 )
{
	if ( !pEntity0 || !pEntity1 )
		return;

	g_EntityCollisionHash->AddObjectPair( pEntity0, pEntity1 );
#ifndef CLIENT_DLL
	pEntity0->CollisionRulesChanged();
	pEntity1->CollisionRulesChanged();
#endif
}


void PhysEnableEntityCollisions( CBaseEntity *pEntity0, CBaseEntity *pEntity1 )
{
	if ( !pEntity0 || !pEntity1 )
		return;

	g_EntityCollisionHash->RemoveObjectPair( pEntity0, pEntity1 );
#ifndef CLIENT_DLL
	pEntity0->CollisionRulesChanged();
	pEntity1->CollisionRulesChanged();
#endif
}

bool PhysEntityCollisionsAreDisabled( CBaseEntity *pEntity0, CBaseEntity *pEntity1 )
{
	return g_EntityCollisionHash->IsObjectPairInHash( pEntity0, pEntity1 );
}

void PhysEnableObjectCollisions( IPhysicsObject *pObject0, IPhysicsObject *pObject1 )
{
	if ( !pObject0 || !pObject1 )
		return;

	g_EntityCollisionHash->RemoveObjectPair( pObject0, pObject1 );
	PhysRecheckObjectPair( pObject0, pObject1 );
}

// disables collisions between entities (each entity may contain multiple objects)
void PhysDisableObjectCollisions( IPhysicsObject *pObject0, IPhysicsObject *pObject1 )
{
	if ( !pObject0 || !pObject1 )
		return;

	g_EntityCollisionHash->AddObjectPair( pObject0, pObject1 );
	PhysRecheckObjectPair( pObject0, pObject1 );
}

void PhysComputeSlideDirection( IPhysicsObject *pPhysics, const Vector &inputVelocity, const AngularImpulse &inputAngularVelocity, 
							   Vector *pOutputVelocity, Vector *pOutputAngularVelocity, float minMass )
{
	Vector velocity = inputVelocity;
	AngularImpulse angVel = inputAngularVelocity;
	Vector pos;

	IPhysicsFrictionSnapshot *pSnapshot = pPhysics->CreateFrictionSnapshot();
	while ( pSnapshot->IsValid() )
	{
		IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
		if ( !pOther->IsMoveable() || pOther->GetMass() > minMass )
		{
			Vector normal;
			pSnapshot->GetSurfaceNormal( normal );

			// BUGBUG: Figure out the correct rotation clipping equation
			if ( pOutputAngularVelocity )
			{
				angVel = normal * DotProduct( angVel, normal );
#if 0
				pSnapshot->GetContactPoint( point );
				Vector point, dummy;
				AngularImpulse angularClip, clip2;

				pPhysics->CalculateVelocityOffset( normal, point, dummy, angularClip );
				VectorNormalize( angularClip );
				float proj = DotProduct( angVel, angularClip );
				if ( proj > 0 )
				{
					angVel -= angularClip * proj;
				}
				CrossProduct( angularClip, normal, clip2 );
				proj = DotProduct( angVel, clip2 );
				if ( proj > 0 )
				{
					angVel -= clip2 * proj;
				}
				//NDebugOverlay::Line( point, point - normal * 20, 255, 0, 0, true, 0.1 );
#endif
			}

			// Determine how far along plane to slide based on incoming direction.
			// NOTE: Normal points away from this object
			float proj = DotProduct( velocity, normal );
			if ( proj > 0.0f )
			{
				velocity -= normal * proj;
			}
		}
		pSnapshot->NextFrictionData();
	}
	pPhysics->DestroyFrictionSnapshot( pSnapshot );

	//NDebugOverlay::Line( pos, pos + unitVel * 20, 0, 0, 255, true, 0.1 );
	
	if ( pOutputVelocity )
	{
		*pOutputVelocity = velocity;
	}
	if ( pOutputAngularVelocity )
	{
		*pOutputAngularVelocity = angVel;
	}
}

bool PhysHasContactWithOtherInDirection( IPhysicsObject *pPhysics, const Vector &dir )
{
	bool hit = false;
	void *pGameData = pPhysics->GetGameData();
	IPhysicsFrictionSnapshot *pSnapshot = pPhysics->CreateFrictionSnapshot();
	while ( pSnapshot->IsValid() )
	{
		IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
		if ( pOther->GetGameData() != pGameData )
		{
			Vector normal;
			pSnapshot->GetSurfaceNormal( normal );
			if ( DotProduct(normal,dir) > 0 )
			{
				hit = true;
				break;
			}
		}
		pSnapshot->NextFrictionData();
	}
	pPhysics->DestroyFrictionSnapshot( pSnapshot );

	return hit;
}


void PhysForceClearVelocity( IPhysicsObject *pPhys )
{
	IPhysicsFrictionSnapshot *pSnapshot = pPhys->CreateFrictionSnapshot();
	// clear the velocity of the rigid body
	Vector vel;
	AngularImpulse angVel;
	vel.Init();
	angVel.Init();
	pPhys->SetVelocity( &vel, &angVel );
	// now clear the "strain" stored in the contact points
	while ( pSnapshot->IsValid() )
	{
		pSnapshot->ClearFrictionForce();
		pSnapshot->RecomputeFriction();
		pSnapshot->NextFrictionData();
	}
	pPhys->DestroyFrictionSnapshot( pSnapshot );
}


void PhysFrictionEffect( Vector &vecPos, Vector vecVel, float energy, int surfaceProps, int surfacePropsHit )
{
	Vector invVecVel = -vecVel;
	VectorNormalize( invVecVel );

	surfacedata_t *psurf = physprops->GetSurfaceData( surfaceProps );
	surfacedata_t *phit = physprops->GetSurfaceData( surfacePropsHit );

	switch ( phit->game.material )
	{
	case CHAR_TEX_DIRT:
		
		if ( energy < MASS10_SPEED2ENERGY(15) )
			break;
		
		g_pEffects->Dust( vecPos, invVecVel, 1, 16 );
		break;

	case CHAR_TEX_CONCRETE:
		
		if ( energy < MASS10_SPEED2ENERGY(28) )
			break;
		
		g_pEffects->Dust( vecPos, invVecVel, 1, 16 );
		break;
	}
	
	//Metal sparks
	if ( energy > MASS10_SPEED2ENERGY(50) )
	{
		// make sparks for metal/concrete scrapes with enough energy
		if ( psurf->game.material == CHAR_TEX_METAL || psurf->game.material == CHAR_TEX_GRATE )
		{	
			switch ( phit->game.material )
			{
			case CHAR_TEX_CONCRETE:
			case CHAR_TEX_METAL:

				g_pEffects->MetalSparks( vecPos, invVecVel );
				break;									
			}
		}
	}
}

void PhysFrictionSound( CBaseEntity *pEntity, IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit )
{
	if ( !pEntity || energy < 75.0f || surfaceProps < 0 )
		return;
	
	// don't make noise for hidden/invisible/sky materials
	surfacedata_t *phit = physprops->GetSurfaceData( surfacePropsHit );
	surfacedata_t *psurf = physprops->GetSurfaceData( surfaceProps );

	if ( phit->game.material == 'X' || psurf->game.material == 'X' )
		return;

	// rescale the incoming energy
	energy *= ENERGY_VOLUME_SCALE;

	// volume of scrape is proportional to square of energy (steeper rolloff at low energies)
	float volume = energy * energy;
		
	unsigned short soundName = psurf->sounds.scrapeRough;
	short *soundHandle = &psurf->soundhandles.scrapeRough;

	if ( psurf->sounds.scrapeSmooth && phit->audio.roughnessFactor < psurf->audio.roughThreshold )
	{
		soundName = psurf->sounds.scrapeSmooth;
		soundHandle = &psurf->soundhandles.scrapeRough;
	}

	const char *pSoundName = physprops->GetString( soundName );

	PhysFrictionSound( pEntity, pObject, pSoundName, *soundHandle, volume );
}

//-----------------------------------------------------------------------------
// Purpose: Precaches a surfaceproperties string name if it's set.
// Input  : idx - 
// Output : static void
//-----------------------------------------------------------------------------
static HSOUNDSCRIPTHANDLE PrecachePhysicsSoundByStringIndex( int idx )
{
	// Only precache if a value was set in the script file...
	if ( idx != 0 )
	{
		return CBaseEntity::PrecacheScriptSound( physprops->GetString( idx ) );
	}

	return SOUNDEMITTER_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Purpose: Iterates all surfacedata sounds and precaches them
// Output : static void
//-----------------------------------------------------------------------------
void PrecachePhysicsSounds()
{
	// precache the surface prop sounds
	for ( int i = 0; i < physprops->SurfacePropCount(); i++ )
	{
		surfacedata_t *pprop = physprops->GetSurfaceData( i );
		Assert( pprop );

		pprop->soundhandles.stepleft = PrecachePhysicsSoundByStringIndex( pprop->sounds.stepleft );
		pprop->soundhandles.stepright = PrecachePhysicsSoundByStringIndex( pprop->sounds.stepright );
		pprop->soundhandles.impactSoft = PrecachePhysicsSoundByStringIndex( pprop->sounds.impactSoft );
		pprop->soundhandles.impactHard = PrecachePhysicsSoundByStringIndex( pprop->sounds.impactHard );
		pprop->soundhandles.scrapeSmooth = PrecachePhysicsSoundByStringIndex( pprop->sounds.scrapeSmooth );
		pprop->soundhandles.scrapeRough = PrecachePhysicsSoundByStringIndex( pprop->sounds.scrapeRough );
		pprop->soundhandles.bulletImpact = PrecachePhysicsSoundByStringIndex( pprop->sounds.bulletImpact );
		pprop->soundhandles.rolling = PrecachePhysicsSoundByStringIndex( pprop->sounds.rolling );
		pprop->soundhandles.breakSound = PrecachePhysicsSoundByStringIndex( pprop->sounds.breakSound );
		pprop->soundhandles.strainSound = PrecachePhysicsSoundByStringIndex( pprop->sounds.strainSound );
	}
}


