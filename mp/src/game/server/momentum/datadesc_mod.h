//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose: 
//
//=============================================================================//

#pragma once

char *Datadesc_SetFieldString( const char *szValue, CBaseEntity *pObject, typedescription_t *pField, fieldtype_t *pFieldType = NULL );

bool ReadUnregisteredKeyfields( CBaseEntity *pTarget, const char *szKeyName, variant_t *variant );
