#include "cbase.h"

#include "jsontokv.h"

KeyValues *CJsonToKeyValues::ConvertJsonToKeyValues(JsonNode *node)
{
    //This iterates through the base JsonObject node
    KeyValues *pKvToReturn = new KeyValues("Response");
    while (node)
    {
        MapNode(node, pKvToReturn);
        node = node->next;
    }
    return pKvToReturn;
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
        kv->AddSubKey(MapNode(node));
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

KeyValues *CJsonToKeyValues::MapNode(JsonNode *node)
{
    // The parent KV holds the name of the first parent (Which, in case there is only 1 node/value, it will be the same
    // as the only value it has)
    if (!node)
        return nullptr;

    // @Ruben: When node->key is null on the json, key is not nullptr, but 0xffeeffee.
    // MOM_TODO: Is it always that address? If not, when / how does it change?
    bool isKeyNull = node->key == nullptr || POINTER_TO_INT(node->key) == 0xffeeffee;

    // @Gocnak: Note: The key should never be null in here. The only time it would be null is either
    // you pass the first node into this method (handled by the Convert method), or if the response from the API is bad!
    // But you never know, so *shrug*

    // Parent keyvalue.
    KeyValues *pNodeValues = new KeyValues(isKeyNull ? "(null)" : node->key);

    for (auto i : node->value)
    {
        // If what we're going to parse is an object, then we need to add it as a subkey.
        if (i->value.getTag() == JSON_OBJECT || i->value.getTag() == JSON_ARRAY)
        {
            //Array or just normal object, make this into a subkey
            KeyValues *pSub = MapNode(i);
            //Add it to our parent
            pNodeValues->AddSubKey(pSub);
            //Iterate through it
            MapNode(i->value.toNode(), pSub);
        }
        else 
        {
            // Otherwise (strings, numbers, booleans) we just add them as an entry of the current key
            MapNode(i, pNodeValues);
        }
    }

    return pNodeValues;
}