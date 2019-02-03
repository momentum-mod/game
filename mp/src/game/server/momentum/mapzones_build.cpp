#include "cbase.h"

#include "mom_triggers.h"
#include "mapzones_build.h"


#include "tier0/memdbgon.h"

vertarray_t* vertarray_t::Create(int num)
{
    Assert(num > 0);
    auto hull = (vertarray_t*)malloc(sizeof(pvertarray_t) + sizeof(Vector) * (size_t)num);
    hull->nVerts = num;
    // Point the array to the memory next to it
    hull->pVerts = (Vector*)((char*)&hull->pVerts + sizeof(Vector*));

    return hull;
}

void vertarray_t::Copy(const CUtlVector<Vector> &verts)
{
    Assert(verts.Count() == nVerts);

    int n = verts.Count();
    for (int i = 0; i < n; i++)
    {
        pVerts[i] = verts[i];
    }
}

pvertarray_t* pvertarray_t::Create(int num)
{
    Assert(num > 0);
    auto hull = (pvertarray_t*)malloc(sizeof(pvertarray_t) + (sizeof(Vector*) + sizeof(Vector)) * (size_t)num);
    hull->nVerts = num;

    // Point the pointers to the memory next to it
    hull->ppVerts = (Vector**)((char*)&hull->ppVerts + sizeof(Vector**));
    for (int i = 0; i < num; i++)
    {
        hull->ppVerts[i] = (Vector*)((char*)hull->ppVerts + ((size_t)num * sizeof(Vector**) + (size_t)i * sizeof(Vector)));
    }

    return hull;
}


// Base
CMomBaseZoneBuilder* CMomBaseZoneBuilder::GetZoneBuilder(KeyValues *kv)
{
    if (kv->FindKey("point_points"))
    {
        return new CMomPointZoneBuilder;
    }
    else
    {
        return new CMomBoxZoneBuilder;
    }
}

void CMomBaseZoneBuilder::DrawZoneLine(const Vector &start, const Vector &end, float t)
{
    DebugDrawLine(start, end, 255, 255, 255, true, t);
}


// Point
CMomPointZoneBuilder::CMomPointZoneBuilder()
{
    m_bFreePhysCollide = false;
    m_pPhysCollide = nullptr;

    ResetMe();
}

CMomPointZoneBuilder::~CMomPointZoneBuilder()
{
    ResetMe();
}

void CMomPointZoneBuilder::Init(CUtlVector<Vector> &points)
{
    // Center the points relative to OUR center and fix the point ordering

    int nPoints = m_vPoints.Count();

    Assert(points.Count() == nPoints);

    if (!nPoints)
        return;


    m_vecCenter = vec3_origin;
    // We're also going to need the bounds
    m_vecMins.Init(FLT_MAX,FLT_MAX,FLT_MAX);
    m_vecMaxs.Init(-FLT_MAX,-FLT_MAX,-FLT_MAX);


    for (int i = 0; i < nPoints; i++)
    {
        m_vecCenter += m_vPoints[i];
    }

    m_vecCenter /= nPoints;


    for (int i = 0; i < nPoints; i++)
    {
        auto& p = points[i];
        p = m_vPoints[i] - m_vecCenter;


        if (p.x < m_vecMins.x)
            m_vecMins.x = p.x;
        if (p.y < m_vecMins.y)
            m_vecMins.y = p.y;
        if (p.z < m_vecMins.z)
            m_vecMins.z = p.z;

        if (p.x > m_vecMaxs.x)
            m_vecMaxs.x = p.x;
        if (p.y > m_vecMaxs.y)
            m_vecMaxs.y = p.y;
        if (p.z > m_vecMaxs.z)
            m_vecMaxs.z = p.z;
    }

    m_vecMaxs.z += GetHeight();

    FixPointOrder(points);
}

void CMomPointZoneBuilder::CopyPoints(const CUtlVector<Vector>& vec)
{
    m_vPoints.Purge();
    m_vPoints.CopyArray(vec.Base(), vec.Count());
}

void CMomPointZoneBuilder::FixPointOrder(CUtlVector<Vector> &points)
{
    // Correct the points to be in clockwise order

    int len = points.Count();
    if (len < 3)
        return;


    // Add all the dot products together.
    // This magically figures out which way we're going
    float total_dot = 0.0f;

    for (int i = 0; i < len; i++)
    {
        int inext = (i+1 >= len) ? 0 : i+1;
        int iprev = (i-1 < 0) ? len-1 : i-1;

        auto& next = points[inext].AsVector2D();
        auto& prev = points[iprev].AsVector2D();
        auto& cur = points[i].AsVector2D();

        auto n = next - cur;
        n.NormalizeInPlace();
        auto p = prev - cur;
        p.NormalizeInPlace();

        auto pleft = Vector2D(-p.y, p.x);



        auto pdot = n.Dot(pleft);
        total_dot += pdot;
    }

    if (total_dot >= 0.0f)
        return;


    // Switch em up.

    DevMsg("Fixing points to clockwise order...\n");
        
    Vector temp;
    for (int i = 0; i < (len/2); i++)
    {
        int n = len-i-1;

        temp = points[n];
        points[n] = points[i];
        points[i] = temp;
    }
}

bool CMomPointZoneBuilder::BuildZone(CBasePlayer *pPlayer, const Vector *vecAim)
{
    if (!IsReady())
    {
        Warning("Not enough points to build a convex shape!\n");
        return false;
    }


    if (pPlayer)
    {
        if (!m_bGetHeight)
        {
            m_bGetHeight = true;
            return false;
        }

        m_flHeight = CMomBoxZoneBuilder::GetZoneHeightToPlayer(pPlayer, m_vPoints[0]);
    }


    const float flDefHeight = 128.0f;
    const float epsilon = 0.1f;
    if (fabsf(m_flHeight) < epsilon)
    {
        m_flHeight = flDefHeight;
    }


    // Get the points relative to our center
    CUtlVector<Vector> relpoints(m_vPoints.Count(), m_vPoints.Count());
    relpoints.SetSize(m_vPoints.Count());
    Init(relpoints);


    // Divide into convex shapes
    CMomHulls_t hulls;
    Decompose(relpoints, hulls);
    

    
    int nHulls = hulls.Count();

    Assert(nHulls >= 1);

    if (!nHulls)
    {
        Warning("No convex hulls were found from points!\n");
        return false;
    }

    DrawDebugLines(hulls);


    pvertarray_t **ppHulls = nullptr;
    // Build the actual three dimensional vertex arrays
    BuildVertArray(hulls, &ppHulls, nHulls);

    m_pPhysCollide = BuildPhysCollide(ppHulls, nHulls);


    hulls.PurgeAndDeleteElements();
    delete[] ppHulls;


    return m_pPhysCollide != nullptr;
}

bool CMomPointZoneBuilder::IsReady() const
{
    return m_vPoints.Count() >= 3;
}

bool CMomPointZoneBuilder::IsDone() const
{
    return m_pPhysCollide != nullptr;
}

void CMomPointZoneBuilder::Reset()
{
    ResetMe();
}

void CMomPointZoneBuilder::ResetMe()
{
    if (m_bFreePhysCollide && m_pPhysCollide)
    {
        physcollision->DestroyCollide(m_pPhysCollide);
    }

    m_pPhysCollide = nullptr;
    m_bFreePhysCollide = true;

    m_bGetHeight = false;
    m_flHeight = 0.0f;

    m_vecCenter = vec3_origin;
    m_vecMins.Init(FLT_MAX, FLT_MAX, FLT_MAX);
    m_vecMaxs.Init(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    m_vPoints.Purge();
}

void CMomPointZoneBuilder::FinishZone(CBaseMomentumTrigger *pEnt)
{
    m_bGetHeight = false;

    pEnt->SetAbsOrigin( m_vecCenter );

    // We're handing the phys collide object to the entity now.
    m_bFreePhysCollide = false;
    pEnt->InitCustomCollision(GetPhysCollide(), m_vecMins, m_vecMaxs);

    pEnt->m_vZonePoints.CopyArray( m_vPoints.Base(), m_vPoints.Count() );
    pEnt->m_flPointZoneHeight = GetHeight();
}

void CMomPointZoneBuilder::Add(CBasePlayer *pPlayer, const Vector &vecAim)
{
    if (!m_bGetHeight && !IsDone())
    {
        m_vPoints.AddToTail(vecAim);
    }
}

void CMomPointZoneBuilder::Remove(CBasePlayer *pPlayer, const Vector &vecAim)
{
    if (m_bGetHeight)
    {
        m_bGetHeight = false;
        return;
    }


    if (!pPlayer)
        return;

    Vector fwdply;
    pPlayer->EyeVectors(&fwdply);

    int index = GetSelectedPoint(pPlayer->EyePosition(), fwdply);
    if (index != -1)
    {
        m_vPoints.Remove(index);
    }
}

void CMomPointZoneBuilder::OnFrame(CBasePlayer *pPlayer, const Vector &vecAim)
{
    int nPoints = m_vPoints.Count();
    int i;

    // Draw bottom
    for (i = 0; i < nPoints-1; i++)
    {
        CMomBaseZoneBuilder::DrawZoneLine(m_vPoints[i], m_vPoints[i+1], -1.0f);
    }

    if (nPoints > 0 && (m_bGetHeight || IsDone()))
    {
        CMomBaseZoneBuilder::DrawZoneLine(m_vPoints[0], m_vPoints[nPoints-1], -1.0f);
    }


    // Draw top
    if (m_bGetHeight)
    {
        float h = CMomBoxZoneBuilder::GetZoneHeightToPlayer(pPlayer, m_vPoints[0]);
        Vector p0, p1;

        // Draw top
        for (i = 0; i < nPoints-1; i++)
        {
            p0 = m_vPoints[i];
            p0.z += h;
            p1 = m_vPoints[i+1];
            p1.z += h;
            CMomBaseZoneBuilder::DrawZoneLine(p0, p1, -1.0f);
        }

        p0 = m_vPoints[0];
        p0.z += h;
        p1 = m_vPoints[nPoints-1];
        p1.z += h;

        CMomBaseZoneBuilder::DrawZoneLine(p0, p1, -1.0f);


        // Draw bottom to top
        for (i = 0; i < nPoints; i++)
        {
            p1 = m_vPoints[i];
            p1.z += h;
            CMomBaseZoneBuilder::DrawZoneLine(m_vPoints[i], p1, -1.0f);
        }
    }


    if (IsDone() || m_bGetHeight)
        return;


    // Draw the aim spot
    if (nPoints > 0)
    {
        CMomBaseZoneBuilder::DrawZoneLine(m_vPoints[0], vecAim, -1.0f);
        if (m_vPoints.Count() > 1)
            CMomBaseZoneBuilder::DrawZoneLine(m_vPoints[nPoints-1], vecAim, -1.0f);
    }
    

    // Draw the selected point
    Vector fwdply;
    pPlayer->EyeVectors(&fwdply);

    
    int iPoint = GetSelectedPoint(pPlayer->EyePosition(), fwdply);
    if (iPoint != -1)
    {
        NDebugOverlay::Circle(m_vPoints[iPoint], 4.0f, 255, 255, 255, 255, true, -1.0f);
    }
}


bool CMomPointZoneBuilder::LoadFromZone(const CBaseMomentumTrigger *pEnt)
{
    // No points to save!
    if (!pEnt->m_vZonePoints.Count())
        return false;


    m_vPoints.CopyArray(pEnt->m_vZonePoints.Base(), pEnt->m_vZonePoints.Count());
    SetHeight( pEnt->m_flPointZoneHeight );

    return true;
}

bool CMomPointZoneBuilder::Load(KeyValues *kv)
{
    SetHeight(kv->GetFloat("point_height"));


    auto sub = kv->FindKey("point_points");
    if (!sub)
        return false;

    auto s = sub->GetFirstSubKey();
    if (!s)
        return false;


    do
    {
        const char* val = s->GetString();
        if (val && *val)
        {
            Vector pos;
            pos.Init();
            sscanf(val, "%f %f %f", &pos.x, &pos.y, &pos.z);

            m_vPoints.AddToTail(pos);
        }
    }
    while ((s = s->GetNextKey()) != nullptr);

    return true;
}

bool CMomPointZoneBuilder::Save(KeyValues *kv)
{
    auto sub = new KeyValues("point_points");

    char szName[64];
    char szValue[128];
    for (int i = 0; i < m_vPoints.Count(); i++)
    {
        auto& pos = m_vPoints[i];

        Q_snprintf(szName, sizeof(szName), "p%i", i);
        Q_snprintf(szValue, sizeof(szValue), "%.1f %.1f %.1f", pos.x, pos.y, pos.z);
        sub->SetString(szName, szValue);
    }


    kv->SetFloat("point_height", m_flHeight);
    kv->AddSubKey(sub);

    return true;
}

int CMomPointZoneBuilder::GetSelectedPoint(const Vector &pos, const Vector &fwd) const
{
    // We have to be looking fairly close to the point
    const float flMinSelectionDot = 0.992f;

    int closest_index = -1;
    float closest_dot = flMinSelectionDot;

    Vector myfwd;
    FOR_EACH_VEC(m_vPoints,i)
    {
        myfwd = m_vPoints[i] - pos;
        myfwd.NormalizeInPlace();
        
        float dot = fwd.Dot(myfwd);
        if (dot > closest_dot)
        {
            closest_index = i;
            closest_dot = dot;
        }
    }

    return closest_index;
}

void CMomPointZoneBuilder::BuildVertArray(CMomHulls_t &hulls, pvertarray_t ***pppHulls, int nHulls)
{
    auto pHulls = new pvertarray_t*[nHulls];


    float height = GetHeight();

    for (int i = 0; i < nHulls; i++)
    {
        auto& points = hulls[i];
        int nPoints = points->nVerts;
        Assert(nPoints >= 3);

        pHulls[i] = pvertarray_t::Create(nPoints * 2);

        for (int j = 0; j < nPoints; j++)
        {
            int k = j * 2;

            *pHulls[i]->ppVerts[k] = points->pVerts[j];

            Vector *top = pHulls[i]->ppVerts[k+1];
            *top = points->pVerts[j];
            top->z += height;
        }
    }


    *pppHulls = pHulls;
}

CPhysCollide* CMomPointZoneBuilder::BuildPhysCollide(pvertarray_t **ppHulls, int nHulls)
{
    if (m_pPhysCollide && m_bFreePhysCollide)
    {
        physcollision->DestroyCollide(m_pPhysCollide);
    }
    m_bFreePhysCollide = true;

    // Vertices -> convex
    CPhysConvex** ppPhysConvex = new CPhysConvex*[nHulls];

    int nConvexes = 0;
    for (int i = 0; i < nHulls; i++)
    {
        auto pPhysConvex = physcollision->ConvexFromVerts(ppHulls[i]->ppVerts, ppHulls[i]->nVerts);

        if (!pPhysConvex)
        {
            Warning("Failed to create a convex object from verts! (%i)\n", i);
            continue;
        }

        ppPhysConvex[nConvexes++] = pPhysConvex;
    }


    if (!nConvexes)
    {
        Warning("Failed to create any convex objects from vertices!\n");

        delete[] ppPhysConvex;
        return nullptr;
    }

    // Combine the convex shapes
    CPhysCollide* pPhysCollide = physcollision->ConvertConvexToCollide(ppPhysConvex, nConvexes);
    if (!pPhysCollide)
    {
        Warning("Failed to convert convexes to collide!\n");

        delete[] ppPhysConvex;
        return nullptr;
    }


    delete[] ppPhysConvex;

    return pPhysCollide;
}

bool CMomPointZoneBuilder::LinesIntersect(const Vector2D &l1s, const Vector2D &l1e, const Vector2D &l2s, const Vector2D &l2e)
{
    auto l1delta = l1e - l1s;
    auto l2delta = l2e - l2s;

    auto l1right = Vector2D(l1delta.y, -l1delta.x);
    l1right.NormalizeInPlace();

    auto l2fwd = l2delta;
    l2fwd.NormalizeInPlace();

    // We have to have this hack here, unfortunately.
    // From the "inside" we're not intersecting, from the outside (concave) it is.
    // This only works because we know the order of the line segments.
    if (l1e == l2e || l1e == l2s)
    {
        return l1right.Dot(l2fwd) < 0.0f;
    }


    float k;
    float s, t;

    const float epsilon = 0.0001f;

    k = -l2delta.x * l1delta.y + l1delta.x * l2delta.y;
    if (fabs(k) < epsilon)
        return false;

    s = (-l1delta.y * (l1s.x - l2s.x) + l1delta.x * (l1s.y - l2s.y)) / k;
    t = (l2delta.x * (l1s.y - l2s.y) - l2delta.y * (l1s.x - l2s.x)) / k;

    return s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f;
}

void CMomPointZoneBuilder::Decompose(CUtlVector<Vector> &points, CMomHulls_t &hulls)
{
    int len = points.Count();


    if (len < 3)
    {
        AssertMsg(0, "Can't decompose %i vertices!", len);
        return;
    }


    bool bConvex = true;

    if (len > 3)
    {
        for (int i = 0; i < len; i++)
        {
            int inext = (i+1 >= len) ? 0 : (i+1);
            int iprev = (i-1 < 0) ? (len-1) : (i-1);

            auto& next = points[inext].AsVector2D();
            auto& prev = points[iprev].AsVector2D();
            auto& cur = points[i].AsVector2D();

            auto n = next - cur;
            n.NormalizeInPlace();
            auto p = prev - cur;
            p.NormalizeInPlace();


            // Is the vertex convex?
            auto pright = Vector2D(p.y, -p.x);

            float pdot = n.Dot(pright);
            if (pdot <= 0.0f)
            {
                continue;
            }

            
            // Found reflex vertex
            bConvex = false;

            // Find a point we can connect to
            int connect = -1;

            // We have no other choice, no reason to check for intersecting
            if (len == 4)
            {
                connect = i + 2;
                if (connect >= len)
                    connect = connect % len;
            }
            else
            {

                for (int j = 0; j < len; j++)
                {
                    if (j == i)
                        continue;
                    // No reason to check the points next to us
                    if (j == iprev)
                        continue;
                    if (j == inext)
                        continue;



                    auto& l1s = points[i].AsVector2D();
                    auto& l1e = points[j].AsVector2D();

                    bool intersect = false;
                    for (int k = 0; k < len; k++)
                    {
                        int knext = (k+1 >= len) ? 0 : (k+1);
                        // We're connected to these! Welcome the errors.
                        if (knext == i)
                            continue;
                        if (k == i)
                            continue;

                        auto& l2s = points[k].AsVector2D();
                        auto& l2e = points[knext].AsVector2D();

                        if (LinesIntersect(l1s, l1e, l2s, l2e))
                        {
                            intersect = true;
                            break;
                        }
                    }

                    if (!intersect)
                    {
                        connect = j;
                        break;
                    }
                }
            }


            if (connect != -1)
            {
                CUtlVector<Vector> left;
                CUtlVector<Vector> right;

                for (int j = i;; j++)
                {
                    if (j >= len)
                        j = 0;
                    left.AddToTail(points[j]);
                    if (j == connect)
                        break;
                }

                for (int j = connect;; j++)
                {
                    if (j >= len)
                        j = 0;
                    right.AddToTail(points[j]);
                    if (j == i)
                        break;
                }

                Decompose(left, hulls);
                Decompose(right, hulls);

                break;
            }
            else
            {
                AssertMsg(0, "No connection was made to reflex vertex %i! (Verts: %i)", i, len);
            }
        }
    }


    if (bConvex)
    {
        auto verts = vertarray_t::Create(points.Count());
        verts->Copy(points);

        hulls.AddToTail(verts);
    }
}

void CMomPointZoneBuilder::DrawDebugLines(CMomHulls_t &hulls) const
{
    // Draw the hulls
    // Very useful for debugging
    ConVarRef mom_zone_debug("mom_zone_debug");

    if (mom_zone_debug.GetInt() == 0)
        return;


    float t = mom_zone_debug.GetFloat();
    
    FOR_EACH_VEC(hulls, i)
    {
        int nVerts = hulls[i]->nVerts;
        for (int j = 0; j < nVerts-1; j++)
        {
            DebugDrawLine(m_vecCenter + hulls[i]->pVerts[j], m_vecCenter + hulls[i]->pVerts[j+1], 255, 255, 0, true, t);
        }

        DebugDrawLine(m_vecCenter + hulls[i]->pVerts[0], m_vecCenter + hulls[i]->pVerts[nVerts-1], 255, 255, 0, true, t);
    }
}


CMomBoxZoneBuilder::CMomBoxZoneBuilder()
{
    ResetMe();
}

bool CMomBoxZoneBuilder::BuildZone(CBasePlayer *pPlayer, const Vector *vecAim)
{
    VectorMin(m_vecStart, m_vecEnd, m_vecMins);
    VectorMax(m_vecStart, m_vecEnd, m_vecMaxs);
    m_vecCenter = (m_vecMaxs - m_vecMins) * 0.5f;

    m_vecMins -= m_vecCenter;
    m_vecMaxs -= m_vecCenter;

    return true;
}

bool CMomBoxZoneBuilder::LoadFromZone(const CBaseMomentumTrigger *pEnt)
{
    m_vecCenter = pEnt->GetAbsOrigin();
    m_angRot = pEnt->GetAbsAngles();
    m_vecMins = pEnt->WorldAlignMins();
    m_vecMaxs = pEnt->WorldAlignMaxs();

    return true;
}

bool CMomBoxZoneBuilder::Load(KeyValues *kv)
{
    m_vecCenter = Vector(kv->GetFloat("xPos"), kv->GetFloat("yPos"), kv->GetFloat("zPos"));
    m_angRot = QAngle(kv->GetFloat("xRot"), kv->GetFloat("yRot"), kv->GetFloat("zRot"));
    m_vecMins = Vector(kv->GetFloat("xScaleMins"), kv->GetFloat("yScaleMins"), kv->GetFloat("zScaleMins"));
    m_vecMaxs = Vector(kv->GetFloat("xScaleMaxs"), kv->GetFloat("yScaleMaxs"), kv->GetFloat("zScaleMaxs"));

    return true;
}

bool CMomBoxZoneBuilder::Save(KeyValues *kv)
{
    kv->SetFloat("xPos", m_vecCenter.x);
    kv->SetFloat("yPos", m_vecCenter.y);
    kv->SetFloat("zPos", m_vecCenter.z);
    kv->SetFloat("xRot", m_angRot.x);
    kv->SetFloat("yRot", m_angRot.y);
    kv->SetFloat("zRot", m_angRot.z);
    kv->SetFloat("xScaleMins", m_vecMins.x);
    kv->SetFloat("yScaleMins", m_vecMins.y);
    kv->SetFloat("zScaleMins", m_vecMins.z);
    kv->SetFloat("xScaleMaxs", m_vecMaxs.x);
    kv->SetFloat("yScaleMaxs", m_vecMaxs.y);
    kv->SetFloat("zScaleMaxs", m_vecMaxs.z);

    return true;
}

bool CMomBoxZoneBuilder::IsReady() const
{
    return m_iBuildStage >= BUILDSTAGE_DONE;
}

bool CMomBoxZoneBuilder::IsDone() const
{
    return m_vecCenter != vec3_origin;
}

void CMomBoxZoneBuilder::Reset()
{
    ResetMe();
}

void CMomBoxZoneBuilder::ResetMe()
{
    m_iBuildStage = BUILDSTAGE_START;

    m_flHeight = 0.0f;

    m_vecMins = vec3_origin;
    m_vecMaxs = vec3_origin;
    m_vecCenter = vec3_origin;
    m_angRot = vec3_angle;
}

float CMomBoxZoneBuilder::GetZoneHeightToPlayer(CBasePlayer *pPlayer, const Vector &vecPos)
{
    return pPlayer->GetAbsOrigin().DistTo(vecPos) * tanf(DEG2RAD(-pPlayer->EyeAngles()[0])) + pPlayer->GetViewOffset()[2];
}

void CMomBoxZoneBuilder::OnFrame(CBasePlayer *pPlayer, const Vector &vecAim)
{
    if (IsDone())
    {
        DrawLines(m_vecStart, m_vecEnd);
        return;
    }

    if (m_iBuildStage > BUILDSTAGE_START)
    {
        Vector start = m_vecStart;
        Vector end = vecAim;

        if (m_iBuildStage > BUILDSTAGE_END)
        {
            end = m_vecEnd;
            end.z += GetZoneHeightToPlayer(pPlayer, m_vecStart);
        }


        DrawLines(start, end);
    }
    
}

void CMomBoxZoneBuilder::DrawLines(const Vector &start, const Vector &end)
{
    Vector p0, p1, p2, p3;
    p0 = start;

    p1 = start;
    p1.x = end.x;

    p2 = end;
    p2.z = start.z;

    p3 = end;
    p3.x = start.x;
    p3.z = start.z;

    // Bottom
    DrawZoneLine(p0, p1, -1.0f);
    DrawZoneLine(p1, p2, -1.0f);
    DrawZoneLine(p2, p3, -1.0f);
    DrawZoneLine(p3, p0, -1.0f);

    
    if (start.z != end.z)
    {
        Vector p4, p5, p6, p7;
        p4 = p0;
        p4.z = end.z;
        p5 = p1;
        p5.z = end.z;
        p6 = p2;
        p6.z = end.z;
        p7 = p3;
        p7.z = end.z;

        // Top
        DrawZoneLine(p4, p5, -1.0f);
        DrawZoneLine(p5, p6, -1.0f);
        DrawZoneLine(p6, p7, -1.0f);
        DrawZoneLine(p7, p4, -1.0f);

        // Bottom to top
        DrawZoneLine(p0, p4, -1.0f);
        DrawZoneLine(p1, p5, -1.0f);
        DrawZoneLine(p2, p6, -1.0f);
        DrawZoneLine(p3, p7, -1.0f);
    }
}

void CMomBoxZoneBuilder::Add(CBasePlayer *pPlayer, const Vector &vecAim)
{
    switch (m_iBuildStage++)
    {
    case BUILDSTAGE_START:
        m_vecStart = vecAim; break;
    case BUILDSTAGE_END:
        m_vecEnd = vecAim;
        m_vecEnd.z = m_vecStart.z;
        break;
    case BUILDSTAGE_HEIGHT:
        m_vecEnd.z = m_vecStart.z + GetZoneHeightToPlayer(pPlayer, m_vecStart);
        break;
    default:
        m_iBuildStage = BUILDSTAGE_START;
    }
}

void CMomBoxZoneBuilder::Remove(CBasePlayer *pPlayer, const Vector &vecAim)
{
    if (m_iBuildStage > BUILDSTAGE_START)
        --m_iBuildStage;

    m_vecEnd.z = m_vecStart.z;
}

void CMomBoxZoneBuilder::FinishZone(CBaseMomentumTrigger *pEnt)
{
    pEnt->SetAbsOrigin(m_vecCenter);
    pEnt->SetAbsAngles(m_angRot);

    pEnt->SetCollisionBounds(m_vecMins, m_vecMaxs);
    pEnt->SetSolid(SOLID_BBOX);
}

void CMomBoxZoneBuilder::SetBounds(const Vector &wmins, const Vector &wmaxs)
{
    m_vecStart = wmins;
    m_vecEnd = wmaxs;
}

void CMomBoxZoneBuilder::SetBounds(const Vector &center, const Vector &mins, const Vector &maxs)
{
    m_vecStart = center + mins;
    m_vecEnd = center + maxs;
}

CMomBaseZoneBuilder *CreateZoneBuilderFromExisting(CBaseMomentumTrigger *pEnt)
{
    const CUtlVector<Vector> &points = pEnt->GetZonePoints();

    if (points.Count() > 0)
    {
        auto pBuilder = new CMomPointZoneBuilder;
        pBuilder->LoadFromZone(pEnt);
        return pBuilder;
    }
    else
    {
        auto pBuilder = new CMomBoxZoneBuilder;
        pBuilder->LoadFromZone(pEnt);
        return pBuilder;
    }
}
