#pragma once

#include "cbase.h"

class CMomPlayerSlideData
{
  public:
    CMomPlayerSlideData();
    ~CMomPlayerSlideData();

    CMomPlayerSlideData(const CMomPlayerSlideData &);

    CMomPlayerSlideData &operator=(const CMomPlayerSlideData &);

    FORCEINLINE void Reset()
    {
        m_bIsEnabled = false;
        m_bStuckToGround = false;
        m_bAllowingJump = false;
        m_bEnableGravity = false;
        m_bFixUpsideSlope = false;
        m_uiTouchCounter = 0;
        // m_flGravity = 1.0f;
    }

    FORCEINLINE bool IsEnabled() { return m_bIsEnabled; }

    FORCEINLINE bool IsStuckGround() { return m_bStuckToGround; }

    FORCEINLINE bool IsAllowingJump() { return m_bAllowingJump; }

    FORCEINLINE bool IsGravityEnabled() { return m_bEnableGravity; }

    FORCEINLINE bool IsFixUpsideSlope() { return m_bFixUpsideSlope; }

    FORCEINLINE void SetEnabled() { m_bIsEnabled = true; }

    FORCEINLINE void SetDisabled() { m_bIsEnabled = false; }

    FORCEINLINE void SetStuckToGround(bool bEnable = true) { m_bStuckToGround = bEnable; }

    FORCEINLINE void SetAllowingJump(bool bEnable = true) { m_bAllowingJump = bEnable; }

    FORCEINLINE void SetEnableGravity(bool bEnable = true) { m_bEnableGravity = bEnable; }

    FORCEINLINE void SetFixUpsideSlope(bool bEnable = true) { m_bFixUpsideSlope = bEnable; }

    FORCEINLINE void IncTouchCounter() { m_uiTouchCounter++; }

    FORCEINLINE void DecTouchCounter() { m_uiTouchCounter--; }

    FORCEINLINE int GetTouchCounter() { return m_uiTouchCounter; }

    // FORCEINLINE void SetGravity(float flGravity) { m_flGravity = flGravity; }

  private:
    bool m_bIsEnabled;
    bool m_bStuckToGround;
    bool m_bAllowingJump;
    bool m_bEnableGravity;
    bool m_bFixUpsideSlope;
    uint m_uiTouchCounter;
    // MOM_TODO: Not sure if mapper would like to have a gravity value here, but he could do it with another trigger
    // anyway. float m_flGravity;
};
