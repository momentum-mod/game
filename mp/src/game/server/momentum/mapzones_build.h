#ifndef MAPZONES_BUILD_H
#define MAPZONES_BUILD_H
#ifdef _WIN32
#pragma once
#endif


class CBaseMomentumTrigger;


// These are used for convenience-sake, only allocating once.

// A fixed array of vertices
struct vertarray_t
{
    int nVerts;
    Vector *pVerts;

    static vertarray_t* Create(int num);
    void                Copy(const CUtlVector<Vector> &verts);

private:
    vertarray_t() {}
};

// A fixed array of pointers to vertices
struct pvertarray_t
{
    int nVerts;
    Vector** ppVerts;

    static pvertarray_t* Create(int num);

private:
    pvertarray_t() {}
};

// Used by box builder
#define BUILDSTAGE_START        0
//#define BUILDSTAGE_ROTATE     1
#define BUILDSTAGE_END          1
#define BUILDSTAGE_HEIGHT       2
#define BUILDSTAGE_DONE         3


// Handles zone creation & loading/saving from kv
class CMomBaseZoneBuilder
{
public:
    DECLARE_CLASS_NOBASE(CMomBaseZoneBuilder);

    virtual ~CMomBaseZoneBuilder() {}

    virtual bool BuildZone(CBasePlayer *pPlayer = nullptr, const Vector *vecAim = nullptr) = 0;
    virtual void Add(CBasePlayer *pPlayer, const Vector &vecAim) = 0;
    virtual void Remove(CBasePlayer *pPlayer, const Vector &vecAim) = 0;

    virtual void OnFrame(CBasePlayer *pPlayer, const Vector &vecAim) {}


    virtual bool LoadFromZone(const CBaseMomentumTrigger *pEnt) { return false; }
    virtual bool Load(KeyValues *kv) { return false; }
    virtual bool Save(KeyValues *kv) { return false; }


    virtual void Reset() = 0;

    // When player marks a zone point, do we check if we're ready?
    virtual bool CheckOnMark() const { return false; }

    // We're ready to be built?
    virtual bool IsReady() const = 0;
    // We've created the zone entity?
    virtual bool IsDone() const = 0;

    virtual void FinishZone(CBaseMomentumTrigger *pEnt) {}



    static CMomBaseZoneBuilder* GetZoneBuilder(KeyValues *kv);

    static void DrawZoneLine(const Vector &start, const Vector &end, float t);
};



// Constructs a zone from points
class CMomPointZoneBuilder : public CMomBaseZoneBuilder
{
public:
    DECLARE_CLASS( CMomPointZoneBuilder, CMomBaseZoneBuilder );


    typedef CUtlVector<vertarray_t*> CMomHulls_t;


    CMomPointZoneBuilder();
    ~CMomPointZoneBuilder();

    virtual bool BuildZone(CBasePlayer *pPlayer = nullptr, const Vector *vecAim = nullptr) OVERRIDE;
    virtual void Add(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;
    virtual void Remove(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;

    virtual void OnFrame(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;


    virtual bool LoadFromZone(const CBaseMomentumTrigger *pEnt) OVERRIDE;
    virtual bool Load(KeyValues *kv) OVERRIDE;
    virtual bool Save(KeyValues *kv) OVERRIDE;


    virtual void Reset() OVERRIDE;

    virtual bool IsReady() const OVERRIDE;
    virtual bool IsDone() const OVERRIDE;

    virtual void FinishZone(CBaseMomentumTrigger *pEnt) OVERRIDE;



    const CUtlVector<Vector>&   GetPoints() const { return m_vPoints; }
    void                        CopyPoints(const CUtlVector<Vector>& vec);

    CPhysCollide    *GetPhysCollide() const { return m_pPhysCollide; }

    virtual float   GetHeight() const { return m_flHeight; }
    virtual void    SetHeight(float h) { m_flHeight = h; }


    static bool LinesIntersect(const Vector2D &l1s, const Vector2D &l1e, const Vector2D &l2s, const Vector2D &l2e);
protected:
    void Init(CUtlVector<Vector> &points);
    void ResetMe();


    int GetSelectedPoint(const Vector &pos, const Vector &fwd) const;

    virtual void    BuildVertArray(CMomHulls_t &hulls, pvertarray_t ***pppHulls, int nHulls);

    void            Decompose(CUtlVector<Vector> &points, CMomHulls_t &hulls);
    CPhysCollide    *BuildPhysCollide(pvertarray_t **ppHulls, int nHulls);
    void            FixPointOrder(CUtlVector<Vector> &points);
    void            DrawDebugLines(CMomHulls_t &hulls) const;


private:
    CPhysCollide *m_pPhysCollide;
    bool m_bFreePhysCollide;


    CUtlVector<Vector> m_vPoints;

    Vector m_vecCenter;
    Vector m_vecMins;
    Vector m_vecMaxs;

    float m_flHeight;
    bool m_bGetHeight;
};



// The old box method
class CMomBoxZoneBuilder : public CMomBaseZoneBuilder
{
public:
    DECLARE_CLASS( CMomBoxZoneBuilder, CMomBaseZoneBuilder );


    CMomBoxZoneBuilder();

    virtual bool BuildZone(CBasePlayer *pPlayer = nullptr, const Vector *vecAim = nullptr) OVERRIDE;
    virtual void Add(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;
    virtual void Remove(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;

    virtual void OnFrame(CBasePlayer *pPlayer, const Vector &vecAim) OVERRIDE;


    virtual bool LoadFromZone(const CBaseMomentumTrigger *pEnt) OVERRIDE;
    virtual bool Load(KeyValues *kv) OVERRIDE;
    virtual bool Save(KeyValues *kv) OVERRIDE;


    virtual void Reset() OVERRIDE;

    virtual bool CheckOnMark() const OVERRIDE { return true; }

    virtual bool IsReady() const OVERRIDE;
    virtual bool IsDone() const OVERRIDE;

    virtual void FinishZone(CBaseMomentumTrigger *pEnt) OVERRIDE;


    void    SetBounds(const Vector &wmins, const Vector &wmaxs);
    void    SetBounds(const Vector &center, const Vector &mins, const Vector &maxs);

    float   GetHeight() const { return m_flHeight; }
    void    SetHeight(float h) { m_flHeight = h; }

    int GetBuildStage() const { return m_iBuildStage; }

    static float GetZoneHeightToPlayer(CBasePlayer *pPlayer, const Vector &vecPos);

    void DrawLines(const Vector &start, const Vector &end);

private:
    void ResetMe();


    Vector m_vecStart;
    Vector m_vecEnd;
    float m_flHeight;

    int m_iBuildStage;


    Vector m_vecMins;
    Vector m_vecMaxs;
    Vector m_vecCenter;
    QAngle m_angRot;
};

CMomBaseZoneBuilder *CreateZoneBuilderFromExisting(CBaseMomentumTrigger *pEnt);

#endif // MAPZONES_BUILD_H
