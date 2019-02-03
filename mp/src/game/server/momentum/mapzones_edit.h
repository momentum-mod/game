#ifndef MAPZONES_EDIT_H
#define MAPZONES_EDIT_H
#ifdef _WIN32
#pragma once
#endif

class CBaseMomentumTrigger;
class CMomentumPlayer;
class CMomBaseZoneBuilder;

class CMomZoneEdit : public CAutoGameSystemPerFrame
{
public:
    CMomZoneEdit();
    ~CMomZoneEdit();


    bool IsEditing() const { return m_bEditing; }
    void StopEditing();


    virtual void LevelInitPostEntity() OVERRIDE;
    virtual void FrameUpdatePostEntityThink() OVERRIDE;


    void OnCreate(int zonetype);
    void OnMark(int zonetype);
    void OnRemove();
    void OnCancel();

    bool GetCurrentBuildSpot(CMomentumPlayer *pPlayer, Vector &vecPos);


    void IncreaseZoom( float dist ) { m_flReticleDist = fminf( m_flReticleDist + dist, 2048.0f ); }
    void DecreaseZoom( float dist ) { m_flReticleDist = fmaxf( m_flReticleDist - dist, 16.0f ); }


    static void     VectorSnapToGrid(Vector &dest, float gridsize);
    static float    SnapToGrid(float fl, float gridsize);
    static void     DrawReticle(const Vector &pos, float retsize);
    static void     DrawZoneLine(const Vector &start, const Vector &end, float t);

    static int                      GetEntityZoneType(CBaseEntity *pEnt);
    static CBaseMomentumTrigger     *CreateZoneEntity(int type);
    static void                     SetZoneProps(CBaseEntity *pEnt);
    static int                      ShortNameToZoneType(const char *in);

    CMomBaseZoneBuilder     *GetBuilder();
    void                    SetBuilder(CMomBaseZoneBuilder *pNewBuilder);
    CMomentumPlayer         *GetPlayerBuilder() const;

private:
    bool m_bEditing;
    static bool m_bFirstEdit;

    float m_flReticleDist;

    CMomBaseZoneBuilder* m_pBuilder;
};

extern CMomZoneEdit g_MomZoneEdit;

#endif // MAPZONES_EDIT_H
