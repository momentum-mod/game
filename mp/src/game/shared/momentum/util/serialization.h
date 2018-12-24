#pragma once

class ISerializable
{
public:
    virtual void Serialize(CUtlBuffer &writer) = 0;
};