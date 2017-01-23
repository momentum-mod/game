#pragma once

#include "cbase.h"
#include "mom_player_shared.h"
#include "beam_shared.h"

class CMOMRulerToolMarker : public CBaseAnimating
{
public:
    // We're friends with the tool so it can remove us
    DECLARE_CLASS(CMOMRulerToolMarker, CBaseAnimating)
    friend class CMOMRulerTool;

    void Precache() OVERRIDE;
    void Spawn() OVERRIDE;
    void MoveTo(const Vector &dest);
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

private:
    char m_szDistanceFormat[BUFSIZ];
    Vector m_vFirstPoint;
    Vector m_vSecondPoint;

    CBeam *m_pBeamConnector;
    CMOMRulerToolMarker *m_pFirstMark;
    CMOMRulerToolMarker *m_pSecondMark;
};

extern CMOMRulerTool *g_MOMRulerTool;