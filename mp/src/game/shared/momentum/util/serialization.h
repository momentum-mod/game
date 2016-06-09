#pragma once

#include "cbase.h"
#include "binary_reader.h"
#include "binary_writer.h"

class ISerializable
{
public:
    virtual void Serialize(CBinaryWriter* writer) = 0;
};