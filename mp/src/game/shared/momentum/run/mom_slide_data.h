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
        m_flCurrentTriggerMinZ = 0.0f;
        memset(m_bTouchingTrigger, 0, sizeof(m_bTouchingTrigger));
        // m_iTouchCounter = 0;
        // m_flGravity = 1.0f;
    }

    FORCEINLINE bool IsEnabled() { return m_bIsEnabled; }

    FORCEINLINE bool IsStuckGround() { return m_bStuckToGround; }

    FORCEINLINE bool IsAllowingJump() { return m_bAllowingJump; }

    FORCEINLINE bool IsGravityEnabled() { return m_bEnableGravity; }

    FORCEINLINE bool IsFixUpsideSlope() { return m_bFixUpsideSlope; }

    FORCEINLINE float GetCurrentTriggerMinZ() { return m_flCurrentTriggerMinZ; }

    FORCEINLINE void SetEnabled() { m_bIsEnabled = true; }

    FORCEINLINE void SetDisabled() { m_bIsEnabled = false; }

    FORCEINLINE void SetStuckToGround(bool bEnable = true) { m_bStuckToGround = bEnable; }

    FORCEINLINE void SetAllowingJump(bool bEnable = true) { m_bAllowingJump = bEnable; }

    FORCEINLINE void SetEnableGravity(bool bEnable = true) { m_bEnableGravity = bEnable; }

    FORCEINLINE void SetFixUpsideSlope(bool bEnable = true) { m_bFixUpsideSlope = bEnable; }

    FORCEINLINE void IncTouchCounter() { m_uiTouchCounter++; }

    FORCEINLINE void DecTouchCounter() { m_uiTouchCounter--; }
    FORCEINLINE void SetCurrentTriggerMinZ(float flValue) { m_flCurrentTriggerMinZ = flValue; }

    bool IsTouchingOneTrigger()
    {
        bool bReturn = false;

        for (int i = 0; i != MAX_EDICTS + 1; i++)
        {
            if (m_bTouchingTrigger[i])
            {
                bReturn = true;
                break;
            }
        }

        return bReturn;
    }

    /*

    FORCEINLINE void IncTouchCounter() { m_iTouchCounter++; }

    FORCEINLINE void DecTouchCounter() { m_iTouchCounter--; }

    FORCEINLINE int GetTouchCounter() { return m_iTouchCounter; }

    FORCEINLINE int GetTouchCounter() { return m_uiTouchCounter; }

    // FORCEINLINE void SetGravity(float flGravity) { m_flGravity = flGravity; }

  private:
    bool m_bIsEnabled;
    bool m_bStuckToGround;
    bool m_bAllowingJump;
    bool m_bEnableGravity;
    bool m_bFixUpsideSlope;
    uint m_uiTouchCounter;
    // TO BE SURE THAT IT WON'T BUG AGAIN WITH CAPS.
    bool m_bTouchingTrigger[MAX_EDICTS + 1];
    float m_flCurrentTriggerMinZ;
    // int m_iTouchCounter;
    // MOM_TODO: Not sure if mapper would like to have a gravity value here, but he could do it with another trigger
    // anyway. float m_flGravity;
};
