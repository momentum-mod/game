//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "variant_t.h"
#include "momentum/variant_tools.h"
#include "momentum/matchers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void variant_t::SetEntity( CBaseEntity *val ) 
{ 
	eVal = val;
	fieldType = FIELD_EHANDLE; 
}

const char* variant_t::GetDebug()
{
	const char* fieldtype = "unknown";
	switch (FieldType())
	{
	case FIELD_VOID:			fieldtype = "Void"; break;
	case FIELD_FLOAT:			fieldtype = "Float"; break;
	case FIELD_STRING:			fieldtype = "String"; break;
	case FIELD_INTEGER:			fieldtype = "Integer"; break;
	case FIELD_BOOLEAN:			fieldtype = "Boolean"; break;
	case FIELD_EHANDLE:			fieldtype = "Entity"; break;
	case FIELD_CLASSPTR:		fieldtype = "EntityPtr"; break;
	case FIELD_POSITION_VECTOR:
	case FIELD_VECTOR:			fieldtype = "Vector"; break;
	case FIELD_CHARACTER:		fieldtype = "Character"; break;
	case FIELD_SHORT:			fieldtype = "Short"; break;
	case FIELD_COLOR32:			fieldtype = "Color32"; break;
	default:					fieldtype = UTIL_VarArgs("unknown: %i", FieldType());
	}
	return UTIL_VarArgs("%s (%s)", String(), fieldtype);
}

// cmp1 = val1 float
// cmp2 = val2 float
#define VariantToFloat(val1, val2, lenallowed) \
	float cmp1 = val1.Float() ? val1.Float() : val1.Int(); \
	float cmp2 = val2.Float() ? val2.Float() : val2.Int(); \
	if (lenallowed && val2.FieldType() == FIELD_STRING) \
		cmp2 = Q_strlen(val2.String());

// "intchar" is the result of me not knowing where to find a version of isdigit that applies to negative numbers and floats.
#define intchar(c) (c >= '-' && c <= '9')

// Attempts to determine the field type from whatever is in the string and creates a variant_t with the converted value and resulting field type.
// Right now, Int/Float, String, and Vector are the only fields likely to be used by entities in variant_t parsing, so they're the only ones supported.
// Expand to other fields when necessary.
variant_t Variant_Parse(const char *szValue)
{
	bool isint = true;
	bool isvector = false;
	for (int i = 0; i < Q_strlen(szValue); i++)
	{
		if (!intchar(szValue[i]))
		{
			isint = false;

			if (szValue[i] == ' ')
				isvector = true;
			else
				isvector = false;
		}
	}

	variant_t var;

	if (isint)
	{
		var.SetFloat(Q_atof(szValue));
	}
	else if (isvector)
	{
		var.SetString(MAKE_STRING(szValue));
		var.Convert(FIELD_VECTOR);
	}
	else
	{
		var.SetString(MAKE_STRING(szValue));
	}

	return var;
}

// Passes strings to Variant_Parse, uses the other input data for finding procedural entities.
variant_t Variant_ParseInput(inputdata_t inputdata)
{
	if (inputdata.value.FieldType() == FIELD_STRING)
	{
		if (inputdata.value.String()[0] == '!')
		{
			variant_t var = variant_t();
			var.SetEntity(gEntList.FindEntityProcedural(inputdata.value.String(), inputdata.pCaller, inputdata.pActivator, inputdata.pCaller));
			if (var.Entity())
				return var;
		}
	}

	return Variant_Parse(inputdata.value.String());
}

// Passes string variants to Variant_Parse
variant_t Variant_ParseString(variant_t value)
{
	if (value.FieldType() != FIELD_STRING)
		return value;

	return Variant_Parse(value.String());
}

bool Variant_Equal(variant_t val1, variant_t val2, bool bLenAllowed)
{
	//if (!val2.Convert(val1.FieldType()))
	//	return false;

	// Add more fields if they become necessary
	switch (val1.FieldType())
	{
	case FIELD_INTEGER:
	case FIELD_FLOAT:
	{
		VariantToFloat(val1, val2, bLenAllowed);
		return cmp1 == cmp2;
	}
	case FIELD_BOOLEAN:		return val1.Bool() == val2.Bool();
	case FIELD_EHANDLE:		return val1.Entity() == val2.Entity();
	case FIELD_VECTOR:
	{
		Vector vec1; val1.Vector3D(vec1);
		Vector vec2; val2.Vector3D(vec2);
		return vec1 == vec2;
	}
	// logic_compare allows wildcards on either string
	default:				return Matcher_NamesMatch_MutualWildcard(val1.String(), val2.String());
	}

	return false;
}

// val1 > val2
bool Variant_Greater(variant_t val1, variant_t val2, bool bLenAllowed)
{
	// Add more fields if they become necessary
	switch (val1.FieldType())
	{
	case FIELD_INTEGER:
	case FIELD_FLOAT:
	{
		VariantToFloat(val1, val2, bLenAllowed);
		return cmp1 > cmp2;
	}
	case FIELD_BOOLEAN:		return val1.Bool() && !val2.Bool();
	case FIELD_VECTOR:
	{
		Vector vec1; val1.Vector3D(vec1);
		Vector vec2; val2.Vector3D(vec2);
		return (vec1.x > vec2.x) && (vec1.y > vec2.y) && (vec1.z > vec2.z);
	}
	default:				return Q_strlen(val1.String()) > Q_strlen(val2.String());
	}

	return false;
}

// val1 >= val2
bool Variant_GreaterOrEqual(variant_t val1, variant_t val2, bool bLenAllowed)
{
	// Add more fields if they become necessary
	switch (val1.FieldType())
	{
	case FIELD_INTEGER:
	case FIELD_FLOAT:
	{
		VariantToFloat(val1, val2, bLenAllowed);
		return cmp1 >= cmp2;
	}
	case FIELD_BOOLEAN:		return val1.Bool() >= val2.Bool();
	case FIELD_VECTOR:
	{
		Vector vec1; val1.Vector3D(vec1);
		Vector vec2; val2.Vector3D(vec2);
		return (vec1.x >= vec2.x) && (vec1.y >= vec2.y) && (vec1.z >= vec2.z);
	}
	default:				return Q_strlen(val1.String()) >= Q_strlen(val2.String());
	}

	return false;
}
