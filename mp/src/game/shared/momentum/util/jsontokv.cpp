#include "cbase.h"

#include "jsontokv.h"
#include "fmtstr.h"

#include "tier0/valve_minmax_off.h"
// This is wrapped by minmax_off due to Valve making a macro for min and max...
#include "rapidjson/document.h"
// Now we can unwrap
#include "tier0/valve_minmax_on.h"

#include "tier0/memdbgon.h"

using namespace rapidjson;

inline void IterObject(const char *pName, Value::ConstObject obj, KeyValues *kv);

inline void MapNode(Value::ConstMemberIterator itr, KeyValues *kv)
{
    switch (itr->value.GetType())
    {
    case kNumberType:
        {
            if (itr->value.IsInt())
                kv->SetInt(itr->name.GetString(), itr->value.GetInt());
            else if (itr->value.IsUint64())
                kv->SetUint64(itr->name.GetString(), itr->value.GetUint64());
            else if (itr->value.IsLosslessFloat())
                kv->SetFloat(itr->name.GetString(), itr->value.GetFloat());
            else // Default to float
                kv->SetFloat(itr->name.GetString(), itr->value.GetDouble());
        }
        break;
    case kStringType:
        kv->SetString(itr->name.GetString(), itr->value.GetString());
        break;
    case kArrayType:
        {
            KeyValues *pKv = kv->CreateNewKey();
            pKv->SetName(itr->name.GetString());
            for (Value::ConstValueIterator arrItr = itr->value.Begin(); arrItr != itr->value.End(); ++arrItr)
            {
                // We can only support arrays of objects... for now
                if (arrItr->GetType() == kObjectType)
                    IterObject(nullptr, arrItr->GetObject(), pKv);
                // MOM_TODO: theoretically any array can work, it's just the keys would be disregarded, like the array of objects are
                // Consider creating garbage keys for used values, and iterating over only values of the array.
                // The thing to keep in mind is that whoever is reading the array would know what data type it should be...
            }
        }
        break;
    case kObjectType:
        IterObject(itr->name.GetString(), itr->value.GetObject(), kv);
        break;
    case kTrueType:
    case kFalseType:
        kv->SetBool(itr->name.GetString(), itr->value.GetType() == kTrueType);
        break;
    case kNullType:
        kv->SetString(itr->name.GetString(), nullptr);
        break;
    }
}

inline void IterObject(const char *pName, Value::ConstObject obj, KeyValues *kv)
{
    KeyValues *pKv = kv->CreateNewKey();
    if (pName)
        pKv->SetName(pName);

    for (Value::ConstMemberIterator itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr)
    {
        MapNode(itr, pKv);
    }
}

bool CJsonToKeyValues::ConvertJsonToKeyValues(char *pInput, KeyValues *pOut)
{
    Document doc;
    doc.Parse(pInput);
    if (doc.HasParseError())
    {
        pOut->SetString("err_parse", CFmtStr("Error parsing JSON object! Code: %d", doc.GetParseError()).Get());
    }
    else
    {
        pOut->UsesEscapeSequences(true);
        for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
        {
            MapNode(itr, pOut);
        }
        return true;
    }

    return false;
}