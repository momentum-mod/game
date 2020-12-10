//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "datadesc_mod.h"
#include "saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Sets a field's value to a specific string.
char *Datadesc_SetFieldString( const char *szValue, CBaseEntity *pObject, typedescription_t *pField, fieldtype_t *pFieldType )
{
	// Copied from ::ParseKeyvalue...
	fieldtype_t fieldtype = FIELD_VOID;
	int fieldOffset = pField->fieldOffset[ TD_OFFSET_NORMAL ];
	switch( pField->fieldType )
	{
	case FIELD_MODELNAME:
	case FIELD_SOUNDNAME:
	case FIELD_STRING:
		(*(string_t *)((char *)pObject + fieldOffset)) = AllocPooledString( szValue );
		fieldtype = FIELD_STRING;
		break;

	case FIELD_TIME:
	case FIELD_FLOAT:
		(*(float *)((char *)pObject + fieldOffset)) = Q_atof( szValue );
		fieldtype = FIELD_FLOAT;
		break;

	case FIELD_BOOLEAN:
		(*(bool *)((char *)pObject + fieldOffset)) = (bool)(Q_atoi( szValue ) != 0);
		fieldtype = FIELD_BOOLEAN;
		break;

	case FIELD_CHARACTER:
		(*(char *)((char *)pObject + fieldOffset)) = (char)Q_atoi( szValue );
		fieldtype = FIELD_CHARACTER;
		break;

	case FIELD_SHORT:
		(*(short *)((char *)pObject + fieldOffset)) = (short)Q_atoi( szValue );
		fieldtype = FIELD_SHORT;
		break;

	case FIELD_INTEGER:
	case FIELD_TICK:
		(*(int *)((char *)pObject + fieldOffset)) = Q_atoi( szValue );
		fieldtype = FIELD_INTEGER;
		break;

	case FIELD_POSITION_VECTOR:
	case FIELD_VECTOR:
		UTIL_StringToVector( (float *)((char *)pObject + fieldOffset), szValue );
		fieldtype = FIELD_VECTOR;
		break;

	case FIELD_VMATRIX:
	case FIELD_VMATRIX_WORLDSPACE:
		UTIL_StringToFloatArray( (float *)((char *)pObject + fieldOffset), 16, szValue );
		fieldtype = FIELD_VMATRIX; // ???
		break;

	case FIELD_MATRIX3X4_WORLDSPACE:
		UTIL_StringToFloatArray( (float *)((char *)pObject + fieldOffset), 12, szValue );
		fieldtype = FIELD_VMATRIX; // ???
		break;

	case FIELD_COLOR32:
		UTIL_StringToColor32( (color32 *) ((char *)pObject + fieldOffset), szValue );
		fieldtype = FIELD_COLOR32;
		break;

	case FIELD_CUSTOM:
	{
		SaveRestoreFieldInfo_t fieldInfo =
		{
			(char *)pObject + fieldOffset,
			pObject,
			pField
		};
		pField->pSaveRestoreOps->Parse( fieldInfo, szValue );
		fieldtype = FIELD_STRING;
		break;
	}

	default:
	case FIELD_INTERVAL:
	case FIELD_CLASSPTR:
	case FIELD_MODELINDEX:
	case FIELD_MATERIALINDEX:
	case FIELD_EDICT:
		return NULL;
		//Warning( "%s cannot set field of type %i.\n", GetDebugName(), dmap->dataDesc[i].fieldType );
		break;
	}

	if (pFieldType)
		*pFieldType = fieldtype;

	return ((char*)pObject) + fieldOffset;
}

//-----------------------------------------------------------------------------
// Purpose: ReadUnregisteredKeyfields() was a feeble attempt to obtain non-keyfield keyvalues from KeyValue() with variant_t.
// 
// I didn't know about GetKeyValue() until 9/29/2018.
// I don't remember why I decided to write down the date I found out about it. Maybe I considered that monumental of a discovery.
// 
// However, we still use ReadUnregisteredKeyfields() since GetKeyValue() only supports a string while this function was used for entire variant_ts.
// It now calls GetKeyValue() and returns it as an allocated string.
//-----------------------------------------------------------------------------
bool ReadUnregisteredKeyfields(CBaseEntity *pTarget, const char *szKeyName, variant_t *variant)
{
	if (!pTarget)
		return false;

	char szValue[256];
	if (pTarget->GetKeyValue(szKeyName, szValue, sizeof(szValue)))
	{
		variant->SetString(AllocPooledString(szValue)); // MAKE_STRING causes badness, must pool
		return true;
	}

	return false;
}
