//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Exploding bomb
//
//=============================================================================//
#pragma once

#include "props.h"

class CMomGenericBomb : public CDynamicProp
{
  public:
    DECLARE_CLASS(CMomGenericBomb, CDynamicProp);
    DECLARE_DATADESC();

    CMomGenericBomb();
    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    void Event_Killed(const CTakeDamageInfo &info) OVERRIDE;
    void InputDetonate(inputdata_t &inputdata);

  private:
    float m_flDamage;
    float m_flRadius;
    string_t m_strExplodeSoundName;
    COutputEvent m_Detonate;
};