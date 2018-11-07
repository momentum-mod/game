#pragma once

#include <KeyValues.h>

struct JsonNode;

class CJsonToKeyValues
{
  public:
    CJsonToKeyValues(){};
    ~CJsonToKeyValues(){};

    // Converts an input char buffer JSON object to keyvalues. 
    // Does NOT memory manage either input nor output!
    static bool ConvertJsonToKeyValues(char *pInput, KeyValues *pOut);

  private:
    static void Convert(JsonNode*, KeyValues *);
    // Maps a node (and later its children) to a keyvalue
    static KeyValues *MapNode(JsonNode *);
    // Maps a node and its children to the given keyvalue (recursively)
    static void MapNode(JsonNode *, KeyValues *);
};