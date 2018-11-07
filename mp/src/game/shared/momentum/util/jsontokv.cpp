#include "jsontokv.h"
#include "gason.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

void CJsonToKeyValues::Convert(JsonNode *node, KeyValues *pOut)
{
    //This iterates through the base JsonObject node
    pOut->UsesEscapeSequences(true);
    while (node)
    {
        MapNode(node, pOut);
        node = node->next;
    }
}

bool CJsonToKeyValues::ConvertJsonToKeyValues(char *pInput, KeyValues *pOut)
{
    JsonAllocator alloc; // Allocator
    JsonValue val; // Outer object
    char *endPtr; // Just for show, I guess

    int status = jsonParse(pInput, &endPtr, &val, alloc);
    if (status == JSON_OK)
    {
        Convert(val.toNode(), pOut);
        return true;
    }

    pOut->SetString("err_parse", CFmtStr("%s at %zd", jsonStrError(status), endPtr - pInput).Get());
    
    return false;
}

void CJsonToKeyValues::MapNode(JsonNode *node, KeyValues *kv)
{
    if (!node || !kv)
        return;
    JsonValue value = node->value;
    // What are we?
    switch (value.getTag())
    {
    case JSON_NUMBER:
        kv->SetFloat(node->key, value.toNumber());
        break;
    case JSON_STRING:
        kv->SetString(node->key, value.toString());
        break;
    case JSON_ARRAY:
    case JSON_OBJECT:
        {
            KeyValues *pKv = kv->CreateNewKey();
            MapArrayOrObject(node, pKv);
        }
        break;
    case JSON_TRUE:
    case JSON_FALSE:
        kv->SetBool(node->key, value.getTag() == JSON_TRUE);
        break;
    case JSON_NULL:
        kv->SetString(node->key, nullptr);
        break;
    }
}

void CJsonToKeyValues::MapArrayOrObject(JsonNode *node, KeyValues* pKvInto)
{
    // When node->key is null on the json, key is not nullptr, but 0xffeeffee.
    // This value happens because JSON arrays of JSON objects don't have any name, and the
    // allocator doesn't properly memset the allocated node to be fully null, so 0xffeeffee is
    // the value.
    bool isKeyNull = node->key == nullptr || POINTER_TO_INT(node->key) == 0xffeeffee;

    // Parent keyvalue.
    if (!isKeyNull)
        pKvInto->SetName(node->key);

    for (auto i : node->value)
    {
        MapNode(i, pKvInto);
    }
}