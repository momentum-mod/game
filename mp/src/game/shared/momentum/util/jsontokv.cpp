#include "cbase.h"

#include "jsontokv.h"
KeyValues *CJsonToKeyValues::ConvertJsonToKeyValues(JsonNode *node)
{
    // This simply calls MapNode(node), which handles creating the new keyvalue and making sure node is not nullptr
    return MapNode(node);
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
        for (auto i : value)
        {
            // If what we're going to parse is an object, then we need to add it as a subkey.
            if (i->value.getTag() == JSON_OBJECT || i->value.getTag() == JSON_ARRAY)
            {
                KeyValues *pSub = MapNode(i);
                if (pSub)
                {
                    kv->AddSubKey(pSub);
                }
            }
            else // Otherwise (string, numbers, booleans) we just add them as an entry of the current key
            {
                MapNode(i, kv);
            }
        }
        break;
    case JSON_TRUE:
        kv->SetBool(node->key, true);
        break;
    case JSON_FALSE:
        kv->SetBool(node->key, false);
        break;
    case JSON_NULL:
        kv->SetString(node->key, nullptr);
        break;
    }
}

KeyValues *CJsonToKeyValues::MapNode(JsonNode *node)
{
    // The parent KV holds the name of the first parent (Which, in case there is only 1 node/value, it will be the same
    // as the only value it has)
    if (!node)
        return nullptr;

    // @Ruben: When node->key is null on the json, key is not nullptr, but 0xffeeffee.
    // MOM_TODO: Is it always that adress? If not, when / how does it change?

    // Parent keyvalue.
    KeyValues *pNodeValues =
        new KeyValues((node->key == nullptr || POINTER_TO_INT(node->key) == 0xffeeffee) ? nullptr : node->key);

    MapNode(node, pNodeValues);
    return pNodeValues;
}