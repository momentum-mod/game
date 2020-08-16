#pragma once

class CBaseMomentumTrigger;

// Provides the necessary out parameter types for use with
// IEngineTrace::EnumerateEntities(...), which will intersect with
// the respective trigger (unlike regular traces).
//
// The Engine function will call EnumEntity until it returns false
// which signals that it either traced all the way without
// hitting anything, hit a solid or found the trigger

class CTeleportTriggerTraceEnum : public IEntityEnumerator
{
public:
    CTeleportTriggerTraceEnum(Ray_t *pRay);

    bool EnumEntity(IHandleEntity *pHandleEntity) override;
    CBaseEntity *GetTeleportEntity() { return m_pTeleportEnt; }

private:
    void SetTeleportEntity(CBaseEntity *pEnt) { m_pTeleportEnt = pEnt; }
    
    CBaseEntity *m_pTeleportEnt;
    Ray_t *m_pRay;
};

class CZoneTriggerTraceEnum : public IEntityEnumerator
{
public:
    CZoneTriggerTraceEnum();

    bool EnumEntity(IHandleEntity *pHandleEntity) override;
    CBaseMomentumTrigger *GetZone() const { return m_pZone; }

private:
    CBaseMomentumTrigger *m_pZone;
};

class CTimeTriggerTraceEnum : public IEntityEnumerator
{
public:
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity);

    bool EnumEntity(IHandleEntity *pHandleEntity) override;
    float GetOffset() { return m_flOffset; }

private:
    float m_flOffset;
    Vector m_vecVelocity;
    Ray_t *m_pRay;
};
