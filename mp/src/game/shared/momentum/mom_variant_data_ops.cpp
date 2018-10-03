#include "cbase.h"
#include "mom_variant_data_ops.h"

// EVENTS save/restore parsing wrapper

CVariantSaveDataOps g_VariantSaveDataOps;
ISaveRestoreOps *variantFuncs = &g_VariantSaveDataOps;

CEventsSaveDataOps g_EventsSaveDataOps;
ISaveRestoreOps *eventFuncs = &g_EventsSaveDataOps;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// BUGBUG: Add support for function pointer save/restore to variants
// BUGBUG: Must pass datamap_t to read/write fields 
void variant_t::Set( fieldtype_t ftype, void *data )
{
	fieldType = ftype;

	switch ( ftype )
	{
	case FIELD_BOOLEAN:		bVal = *((bool *)data);				break;
	case FIELD_CHARACTER:	iVal = *((char *)data);				break;
	case FIELD_SHORT:		iVal = *((short *)data);			break;
	case FIELD_INTEGER:		iVal = *((int *)data);				break;
	case FIELD_STRING:		iszVal = *((string_t *)data);		break;
	case FIELD_FLOAT:		flVal = *((float *)data);			break;
	case FIELD_COLOR32:		rgbaVal = *((color32 *)data);		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		vecVal[0] = ((float *)data)[0];
		vecVal[1] = ((float *)data)[1];
		vecVal[2] = ((float *)data)[2];
		break;
	}

	case FIELD_EHANDLE:		eVal = *((EHANDLE *)data);			break;
	case FIELD_CLASSPTR:	eVal = *((CBaseEntity **)data);		break;
	case FIELD_VOID:		
	default:
		iVal = 0; fieldType = FIELD_VOID;	
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Copies the value in the variant into a block of memory
// Input  : *data - the block to write into
//-----------------------------------------------------------------------------
void variant_t::SetOther( void *data )
{
	switch ( fieldType )
	{
	case FIELD_BOOLEAN:		*((bool *)data) = bVal != 0;		break;
	case FIELD_CHARACTER:	*((char *)data) = iVal;				break;
	case FIELD_SHORT:		*((short *)data) = iVal;			break;
	case FIELD_INTEGER:		*((int *)data) = iVal;				break;
	case FIELD_STRING:		*((string_t *)data) = iszVal;		break;
	case FIELD_FLOAT:		*((float *)data) = flVal;			break;
	case FIELD_COLOR32:		*((color32 *)data) = rgbaVal;		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		((float *)data)[0] = vecVal[0];
		((float *)data)[1] = vecVal[1];
		((float *)data)[2] = vecVal[2];
		break;
	}

	case FIELD_EHANDLE:		*((EHANDLE *)data) = eVal;			break;
	case FIELD_CLASSPTR:	*((CBaseEntity **)data) = eVal;		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Converts the variant to a new type. This function defines which I/O
//			types can be automatically converted between. Connections that require
//			an unsupported conversion will cause an error message at runtime.
// Input  : newType - the type to convert to
// Output : Returns true on success, false if the conversion is not legal
//-----------------------------------------------------------------------------
bool variant_t::Convert( fieldtype_t newType )
{
	if ( newType == fieldType )
	{
		return true;
	}

	//
	// Converting to a null value is easy.
	//
	if ( newType == FIELD_VOID )
	{
		Set( FIELD_VOID, NULL );
		return true;
	}

	//
	// FIELD_INPUT accepts the variant type directly.
	//
	if ( newType == FIELD_INPUT )
	{
		return true;
	}

	switch ( fieldType )
	{
		case FIELD_INTEGER:
		{
			switch ( newType )
			{
				case FIELD_FLOAT:
				{
					SetFloat( (float) iVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( iVal != 0 );
					return true;
				}
			}
			break;
		}

		case FIELD_FLOAT:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					SetInt( (int) flVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( flVal != 0 );
					return true;
				}
			}
			break;
		}

		//
		// Everyone must convert from FIELD_STRING if possible, since
		// parameter overrides are always passed as strings.
		//
		case FIELD_STRING:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					if (iszVal != NULL_STRING)
					{
						SetInt(atoi(STRING(iszVal)));
					}
					else
					{
						SetInt(0);
					}
					return true;
				}

				case FIELD_FLOAT:
				{
					if (iszVal != NULL_STRING)
					{
						SetFloat(atof(STRING(iszVal)));
					}
					else
					{
						SetFloat(0);
					}
					return true;
				}

				case FIELD_BOOLEAN:
				{
					if (iszVal != NULL_STRING)
					{
						SetBool( atoi(STRING(iszVal)) != 0 );
					}
					else
					{
						SetBool(false);
					}
					return true;
				}

				case FIELD_VECTOR:
				{
					Vector tmpVec = vec3_origin;
					if (sscanf(STRING(iszVal), "[%f %f %f]", &tmpVec[0], &tmpVec[1], &tmpVec[2]) == 0)
					{
						// Try sucking out 3 floats with no []s
						sscanf(STRING(iszVal), "%f %f %f", &tmpVec[0], &tmpVec[1], &tmpVec[2]);
					}
					SetVector3D( tmpVec );
					return true;
				}

				case FIELD_COLOR32:
				{
					int nRed = 0;
					int nGreen = 0;
					int nBlue = 0;
					int nAlpha = 255;

					sscanf(STRING(iszVal), "%d %d %d %d", &nRed, &nGreen, &nBlue, &nAlpha);
					SetColor32( nRed, nGreen, nBlue, nAlpha );
					return true;
				}

				case FIELD_EHANDLE:
				{
					// convert the string to an entity by locating it by classname
					CBaseEntity *ent = NULL;
					if ( iszVal != NULL_STRING )
					{
#ifdef GAME_DLL
						// MOM_TODO: Client needs to do this
						// FIXME: do we need to pass an activator in here?
						ent = gEntList.FindEntityByName( NULL, iszVal );
#endif
					}
					SetEntity( ent );
					return true;
				}
			}
		
			break;
		}

		case FIELD_EHANDLE:
		{
			switch ( newType )
			{
				case FIELD_STRING:
				{
					// take the entities targetname as the string
#ifdef GAME_DLL
					// MOM_TODO: Client needs to do this
					string_t iszStr = NULL_STRING;
#endif
					if ( eVal != NULL )
					{
						SetString( MAKE_STRING(eVal->GetDebugName()) );
					}
					return true;
				}
			}
			break;
		}
	}

	// invalid conversion
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: All types must be able to display as strings for debugging purposes.
// Output : Returns a pointer to the string that represents this value.
//
//			NOTE: The returned pointer should not be stored by the caller as
//				  subsequent calls to this function will overwrite the contents
//				  of the buffer!
//-----------------------------------------------------------------------------
const char *variant_t::ToString( void ) const
{
	COMPILE_TIME_ASSERT( sizeof(string_t) == sizeof(int) );

	static char szBuf[512];

	switch (fieldType)
	{
	case FIELD_STRING:
		{
			return(STRING(iszVal));
		}

	case FIELD_BOOLEAN:
		{
			if (bVal == 0)
			{
				Q_strncpy(szBuf, "false",sizeof(szBuf));
			}
			else
			{
				Q_strncpy(szBuf, "true",sizeof(szBuf));
			}
			return(szBuf);
		}

	case FIELD_INTEGER:
		{
			Q_snprintf( szBuf, sizeof( szBuf ), "%i", iVal );
			return(szBuf);
		}

	case FIELD_FLOAT:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%g", flVal);
			return(szBuf);
		}

	case FIELD_COLOR32:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%d %d %d %d", (int)rgbaVal.r, (int)rgbaVal.g, (int)rgbaVal.b, (int)rgbaVal.a);
			return(szBuf);
		}

	case FIELD_VECTOR:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "[%g %g %g]", (double)vecVal[0], (double)vecVal[1], (double)vecVal[2]);
			return(szBuf);
		}

	case FIELD_VOID:
		{
			szBuf[0] = '\0';
			return(szBuf);
		}

	case FIELD_EHANDLE:
		{
			const char *pszName = (Entity()) ? Entity()->GetDebugName() : "<<null entity>>";
			Q_strncpy( szBuf, pszName, 512 );
			return (szBuf);
		}
	}

	return("No conversion to string");
}

#define classNameTypedef variant_t // to satisfy DEFINE... macros

typedescription_t variant_t::m_SaveBool[] =
{
	DEFINE_FIELD( bVal, FIELD_BOOLEAN ),
};
typedescription_t variant_t::m_SaveInt[] =
{
	DEFINE_FIELD( iVal, FIELD_INTEGER ),
};
typedescription_t variant_t::m_SaveFloat[] =
{
	DEFINE_FIELD( flVal, FIELD_FLOAT ),
};
typedescription_t variant_t::m_SaveEHandle[] =
{
	DEFINE_FIELD( eVal, FIELD_EHANDLE ),
};
typedescription_t variant_t::m_SaveString[] =
{
	DEFINE_FIELD( iszVal, FIELD_STRING ),
};
typedescription_t variant_t::m_SaveColor[] =
{
	DEFINE_FIELD( rgbaVal, FIELD_COLOR32 ),
};

#undef classNameTypedef

//
// Struct for saving and restoring vector variants, since they are
// stored as float[3] and we want to take advantage of position vector
// fixup across level transitions.
//
#define classNameTypedef variant_savevector_t // to satisfy DEFINE... macros

struct variant_savevector_t
{
	Vector vecSave;
};
typedescription_t variant_t::m_SaveVector[] =
{
	// Just here to shut up ClassCheck
//	DEFINE_ARRAY( vecVal, FIELD_FLOAT, 3 ),

	DEFINE_FIELD( vecSave, FIELD_VECTOR ),
};
typedescription_t variant_t::m_SavePositionVector[] =
{
	DEFINE_FIELD( vecSave, FIELD_POSITION_VECTOR ),
};
#undef classNameTypedef

#define classNameTypedef variant_savevmatrix_t // to satisfy DEFINE... macros
struct variant_savevmatrix_t
{
	VMatrix matSave;
};
typedescription_t variant_t::m_SaveVMatrix[] =
{
	DEFINE_FIELD( matSave, FIELD_VMATRIX ),
};
typedescription_t variant_t::m_SaveVMatrixWorldspace[] =
{
	DEFINE_FIELD( matSave, FIELD_VMATRIX_WORLDSPACE ),
};
#undef classNameTypedef

#define classNameTypedef variant_savevmatrix3x4_t // to satisfy DEFINE... macros
struct variant_savevmatrix3x4_t
{
	matrix3x4_t matSave;
};
typedescription_t variant_t::m_SaveMatrix3x4Worldspace[] =
{
	DEFINE_FIELD( matSave, FIELD_MATRIX3X4_WORLDSPACE ),
};
#undef classNameTypedef

void CEventsSaveDataOps::Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
{
	AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");

	CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
	const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
	for ( int i = 0; i < fieldSize; i++, ev++ )
	{
		// save out the number of fields
		int numElements = ev->NumberOfElements();
		pSave->WriteInt( &numElements, 1 );

		// save the event data
		ev->Save( *pSave );
	}
}

void CEventsSaveDataOps::Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
{
	AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");

	CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
	const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
	for ( int i = 0; i < fieldSize; i++, ev++ )
	{
		int nElements = pRestore->ReadInt();

		Assert( nElements < 100 );

		ev->Restore( *pRestore, nElements );
	}
}

bool CEventsSaveDataOps::IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");

	// check all the elements of the array (usually only 1)
	CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
	const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
	for ( int i = 0; i < fieldSize; i++, ev++ )
	{
		// It's not empty if it has events or if it has a non-void variant value
		if (( ev->NumberOfElements() != 0 ) || ( ev->ValueFieldType() != FIELD_VOID ))
			return 0;
	}

	// variant has no data
	return 1;
}

void CEventsSaveDataOps::MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	// Don't no how to. This is okay, since objects of this type
	// are always born clean before restore, and not reused
}

bool CEventsSaveDataOps::Parse( const SaveRestoreFieldInfo_t &fieldInfo, char const* szValue )
{
	CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
	ev->ParseEventAction( szValue );
	return true;
}

// saves the entire array of variables
void CVariantSaveDataOps::Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
{
	variant_t *var = (variant_t*)fieldInfo.pField;

	int type = var->FieldType();
	pSave->WriteInt( &type, 1 );

	switch ( var->FieldType() )
	{
	case FIELD_VOID:
		break;
	case FIELD_BOOLEAN:	
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveBool, 1 );
		break;
	case FIELD_INTEGER:	
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveInt, 1 );
		break;
	case FIELD_FLOAT:
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveFloat, 1 );
		break;
	case FIELD_EHANDLE:
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveEHandle, 1 );
		break;
	case FIELD_STRING:
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveString, 1 );
		break;
	case FIELD_COLOR32:
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveColor, 1 );
		break;
	case FIELD_VECTOR:
	{
		variant_savevector_t Temp;
		var->Vector3D(Temp.vecSave);
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SaveVector, 1 );
		break;
	}

	case FIELD_POSITION_VECTOR:
	{
		variant_savevector_t Temp;
		var->Vector3D(Temp.vecSave);
		pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SavePositionVector, 1 );
		break;
	}

	default:
		Warning( "Bad type %d in saved variant_t\n", var->FieldType() );
		Assert(0);
	}
}

// restores a single instance of the variable
void CVariantSaveDataOps::Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
{
	variant_t *var = (variant_t*)fieldInfo.pField;

	*var = variant_t();

	var->fieldType = (_fieldtypes)pRestore->ReadInt();

	switch ( var->fieldType )
	{
	case FIELD_VOID:
		break;
	case FIELD_BOOLEAN:	
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveBool, 1 );
		break;
	case FIELD_INTEGER:	
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveInt, 1 );
		break;
	case FIELD_FLOAT:
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveFloat, 1 );
		break;
	case FIELD_EHANDLE:
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveEHandle, 1 );
		break;
	case FIELD_STRING:
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveString, 1 );
		break;
	case FIELD_COLOR32:
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveColor, 1 );
		break;
	case FIELD_VECTOR:
	{
		variant_savevector_t Temp;
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SaveVector, 1 );
		var->SetVector3D(Temp.vecSave);
		break;
	}
	case FIELD_POSITION_VECTOR:
	{
		variant_savevector_t Temp;
		pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SavePositionVector, 1 );
		var->SetPositionVector3D(Temp.vecSave);
		break;
	}
	default:
		Warning( "Bad type %d in saved variant_t\n", var->FieldType() );
		Assert(0);
		break;
	}
}


bool CVariantSaveDataOps::IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	// check all the elements of the array (usually only 1)
	variant_t *var = (variant_t*)fieldInfo.pField;
	for ( int i = 0; i < fieldInfo.pTypeDesc->fieldSize; i++, var++ )
	{
		if ( var->FieldType() != FIELD_VOID )
			return 0;
	}

	// variant has no data
	return 1;
}

void CVariantSaveDataOps::MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	// Don't no how to. This is okay, since objects of this type
	// are always born clean before restore, and not reused
}