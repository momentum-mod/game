#pragma once

#include "cbase.h"
#include "mom_player_shared.h"

class CMOMRulerTool : CAutoGameSystem
{
public:
    CMOMRulerTool(const char* pName);

    ~CMOMRulerTool()
    {
    }


    Vector m_vFirstPoint;
    Vector m_vSecondPoint;
};

extern CMOMRulerTool *g_MOMRulerTool;