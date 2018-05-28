#pragma once

#include "cbase.h"

class CMomSlideData
{
  public:
    CMomSlideData();
    ~CMomSlideData();

    FORCEINLINE void Reset()
    {
        m_bStuckToGround = false;
        m_bAllowingJump = false;
        m_bEnableGravity = false;
        m_bFixUpsideSlope = false;
        m_flDistance = 0.0f;
        m_iEntityIndex = -1;
    }

    FORCEINLINE bool IsStuckGround() { return m_bStuckToGround; }

    FORCEINLINE bool IsAllowingJump() { return m_bAllowingJump; }

    FORCEINLINE bool IsGravityEnabled() { return m_bEnableGravity; }

    FORCEINLINE bool IsFixUpsideSlope() { return m_bFixUpsideSlope; }

    FORCEINLINE float GetDistance() { return m_flDistance; }

    FORCEINLINE int GetEntityIndex() { return m_iEntityIndex; }

    FORCEINLINE void SetStuckToGround(bool bEnable = true) { m_bStuckToGround = bEnable; }

    FORCEINLINE void SetAllowingJump(bool bEnable = true) { m_bAllowingJump = bEnable; }

    FORCEINLINE void SetEnableGravity(bool bEnable = true) { m_bEnableGravity = bEnable; }

    FORCEINLINE void SetFixUpsideSlope(bool bEnable = true) { m_bFixUpsideSlope = bEnable; }

    FORCEINLINE void SetDistance(float flValue) { m_flDistance = flValue; }

    FORCEINLINE void SetEntityIndex(int iEntityIndex) { m_iEntityIndex = iEntityIndex; }

  private:
    bool m_bStuckToGround;
    bool m_bAllowingJump;
    bool m_bEnableGravity;
    bool m_bFixUpsideSlope;
    float m_flDistance;
    int m_iEntityIndex;
    // MOM_TODO: Not sure if mapper would like to have a gravity value here, but he could do it with another trigger
    // anyway. float m_flGravity;
};

class CMomPlayerSlideData
{
  public:
    CMomPlayerSlideData() : m_vecSlideData()
    {
        SetDisabled();
    }

    ~CMomPlayerSlideData() { SetDisabled(); }

    FORCEINLINE bool IsEnabled() { return m_bIsEnabled; }

    FORCEINLINE void SetEnabled() { m_bIsEnabled = true; }

    FORCEINLINE void SetDisabled()
    {
        m_bIsEnabled = false;
        m_vecSlideData.RemoveAll();
    }

    FORCEINLINE CMomSlideData *GetCurrent() { return &m_vecSlideData.Element(0); }

    CUtlVector<CMomSlideData> m_vecSlideData;

  private:
    bool m_bIsEnabled;
};