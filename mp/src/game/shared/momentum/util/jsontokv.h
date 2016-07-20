#pragma once

#include "cbase.h"

#include "gason.h"
#include <KeyValues.h>

class CJsonToKeyValues
{
  public:
    DECLARE_CLASS_NOBASE(CJsonToKeyValues)

    CJsonToKeyValues(){};
    ~CJsonToKeyValues(){};

    // Given a node, converts it and all its childern to KeyValues.
    // Caller has to make sure the returned value is not of nullptr
    KeyValues *ConvertJsonToKeyValues(JsonNode *);

  private:
    // Maps a node (and later its childern) to a keyvalue
    KeyValues *MapNode(JsonNode *);
    // Maps a node and its children to the given keyvalue (recursively)
    void MapNode(JsonNode *, KeyValues *);
};