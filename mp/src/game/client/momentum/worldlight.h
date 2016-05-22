//========= Copyright (C) 2011, CSProMod Team, All rights reserved. =========//
//
// Purpose: provide world light related functions to the client
//
// Written: November 2011
// Author: Saul Rennison
// https://developer.valvesoftware.com/wiki/Dynamic_RTT_shadow_angles_in_Source_2007
//===========================================================================//

#pragma once

#include "igamesystem.h" // CAutoGameSystem

class Vector;
struct dworldlight_t;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CWorldLights : public CAutoGameSystem
{
  public:
    CWorldLights();
    ~CWorldLights() { Clear(); }

    //-------------------------------------------------------------------------
    // Find the brightest light source at a point
    //-------------------------------------------------------------------------
    bool GetBrightestLightSource(const Vector &vecPosition, Vector &vecLightPos, Vector &vecLightBrightness);

    // CAutoGameSystem overrides
  public:
    bool Init() override;
    void LevelInitPreEntity() override;

    void LevelShutdownPostEntity() override { Clear(); }

  private:
    void Clear();

    int m_nWorldLights;
    dworldlight_t *m_pWorldLights;
};

//-----------------------------------------------------------------------------
// Singleton exposure
//-----------------------------------------------------------------------------
extern CWorldLights *g_pWorldLights;