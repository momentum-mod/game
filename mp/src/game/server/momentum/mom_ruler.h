#pragma once

#include "cbase.h"
#include "beam_shared.h"

class CMOMRulerToolMarker : public CBaseAnimating
{
public:
    // We're friends with the tool so it can remove us
    DECLARE_CLASS(CMOMRulerToolMarker, CBaseAnimating);

    void Precache() OVERRIDE;
    void Spawn() OVERRIDE;
    void MoveTo(const Vector &dest);
};

class CMOMRulerToolBeam : public CBeam
{
public:
    DECLARE_CLASS(CMOMRulerToolBeam, CBeam);

    // Overridden here because Valve didn't make these virtual for some reason
    bool IsOn() const { return !IsEffectActive(EF_NODRAW); }
    void TurnOn() { RemoveEffects(EF_NODRAW); }
    void TurnOff() { AddEffects(EF_NODRAW); }

    // We want it to just turn off after a delay, not be removed.
    void SUB_TurnOff() { TurnOff(); }

    // As to not hide Valve's "LiveForTime" which removes the ent at the end of the duration
    void OnForDuration(float duration)
    {
        SetThink(&CMOMRulerToolBeam::SUB_TurnOff);
        SetNextThink(gpGlobals->curtime + duration);
    }

    static CMOMRulerToolBeam *CreateBeam(const char *pModel, float width)
    {
        return static_cast<CMOMRulerToolBeam*>(BeamCreate(pModel, width));
    }

};

class CMOMRulerTool : CAutoGameSystem
{
public:
    CMOMRulerTool(const char* pName);
    ~CMOMRulerTool();

    void PostInit() OVERRIDE;

    void ConnectMarks();
    void Reset();

    void DoTrace(const bool bFirst);
    void Measure();

    void LevelShutdownPreEntity() OVERRIDE
    { 
        Reset(); 
    }

private:
    char m_szDistanceFormat[BUFSIZ];
    Vector m_vFirstPoint;
    Vector m_vSecondPoint;

    CMOMRulerToolBeam *m_pBeamConnector;
    CMOMRulerToolMarker *m_pFirstMark;
    CMOMRulerToolMarker *m_pSecondMark;
};

extern CMOMRulerTool *g_MOMRulerTool;