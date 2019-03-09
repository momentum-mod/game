#include "cbase.h"
#include "c_mom_triggers.h"
#include "dynamicrendertargets.h"
#include "model_types.h"
#include "util/mom_util.h"

#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"
#include "dt_utlvector_recv.h"

static MAKE_TOGGLE_CONVAR(mom_startzone_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable drawing an outline for start zone.");

static MAKE_TOGGLE_CONVAR(mom_endzone_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable outline for end zone.");

static ConVar mom_startzone_color("mom_startzone_color", "00FF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Color of the start zone.");

static ConVar mom_endzone_color("mom_endzone_color", "FF0000FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                "Color of the end zone.");

CTriggerOutlineRenderer::CTriggerOutlineRenderer()
{
    m_pVertices = nullptr;
    m_vertexCount = 0;
}

CTriggerOutlineRenderer::~CTriggerOutlineRenderer()
{
    if (m_pVertices)
        MemAlloc_FreeAligned(m_pVertices);
    m_pVertices = nullptr;
}

bool CTriggerOutlineRenderer::RenderBrushModelSurface(IClientEntity* pBaseEntity, IBrushSurface* pBrushSurface)
{
    const int vertices = pBrushSurface->GetVertexCount();
    if (vertices > m_vertexCount)
    {
        m_vertexCount = vertices;
        if (m_pVertices)
            m_pVertices = static_cast<BrushVertex_t *>(MemAlloc_ReallocAligned(m_pVertices, sizeof(BrushVertex_t) * m_vertexCount, 64));
        else
            m_pVertices = static_cast<BrushVertex_t *>(MemAlloc_AllocAligned(sizeof(BrushVertex_t) * m_vertexCount, 64));
    }
    pBrushSurface->GetVertexData(m_pVertices);
    CMatRenderContextPtr pRenderContext(materials);

    CMeshBuilder builder;
    builder.Begin(pRenderContext->GetDynamicMesh(true, 0, 0, g_pDynamicRenderTargets->GetTriggerOutlineMat()),
                  MATERIAL_LINE_LOOP, vertices);

    for (int i = 0; i < vertices; i++)
    {
        const BrushVertex_t& vertex = m_pVertices[i];

        builder.Position3fv(vertex.m_Pos.Base());
        builder.Normal3fv(vertex.m_Normal.Base());
        builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
        builder.AdvanceVertex();
    }

    builder.End(false, true);
    return false;
}

IMPLEMENT_CLIENTCLASS_DT(C_BaseMomentumTrigger, DT_BaseMomentumTrigger, CBaseMomentumTrigger)
RecvPropUtlVector(RECVINFO_UTLVECTOR(m_vecZonePoints), 32, RecvPropVector(NULL, 0, sizeof(Vector))),
RecvPropFloat(RECVINFO(m_flZoneHeight)),
END_RECV_TABLE();

C_BaseMomentumTrigger::C_BaseMomentumTrigger()
{
    m_flZoneHeight = 0.0f;
}

void C_BaseMomentumTrigger::DrawOutlineModel(const Color& outlineColor)
{
    const int iNum = m_vecZonePoints.Count();

    if (iNum <= 0)
        return;

    CMatRenderContextPtr pRenderContext(materials);
    CMeshBuilder builder;

    // Bottom
    builder.Begin(pRenderContext->GetDynamicMesh(true, 0, 0, g_pDynamicRenderTargets->GetTriggerOutlineMat()),
                  MATERIAL_LINE_LOOP, iNum);
    for (int i = 0; i < iNum; i++)
    {
        const Vector &cur = m_vecZonePoints[i];

        builder.Position3fv(cur.Base());
        builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
        builder.AdvanceVertex();
    }
    builder.End(false, true);

    // Connecting lines
    for (int i = 0; i < iNum; i++)
    {
        const Vector &cur = m_vecZonePoints[i];

        // Connecting lines
        builder.Begin(pRenderContext->GetDynamicMesh(true, 0, 0, g_pDynamicRenderTargets->GetTriggerOutlineMat()),
                          MATERIAL_LINES, 2);

        builder.Position3fv(cur.Base());
        builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
        builder.AdvanceVertex();

        const Vector next(cur.x, cur.y, cur.z + m_flZoneHeight);
        builder.Position3fv(next.Base());
        builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
        builder.AdvanceVertex();

        builder.End(false, true);
    }

    // Top
    builder.Begin(pRenderContext->GetDynamicMesh(true, 0, 0, g_pDynamicRenderTargets->GetTriggerOutlineMat()),
                  MATERIAL_LINE_LOOP, iNum);
    for (int i = 0; i < iNum; i++)
    {
        const Vector next(m_vecZonePoints[i].x, m_vecZonePoints[i].y, m_vecZonePoints[i].z + m_flZoneHeight);
        builder.Position3fv(next.Base());
        builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
        builder.AdvanceVertex();
    }
    builder.End(false, true);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, C_TriggerTimerStart);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStart, DT_TriggerTimerStart, CTriggerTimerStart)
END_RECV_TABLE();

bool C_TriggerTimerStart::ShouldDraw() { return true; }

int C_TriggerTimerStart::DrawModel(int flags)
{
    if (mom_startzone_outline_enable.GetBool() && (flags & STUDIO_RENDER) != 0 &&
        (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0)
    {
        if (g_pMomentumUtil->GetColorFromHex(mom_startzone_color.GetString(), m_OutlineRenderer.outlineColor))
        {
            if (GetModel())
            {
                render->InstallBrushSurfaceRenderer(&m_OutlineRenderer);
                BaseClass::DrawModel(flags);
                render->InstallBrushSurfaceRenderer(nullptr);
            }
            else
            {
                DrawOutlineModel(m_OutlineRenderer.outlineColor);
                return 1;
            }
        }
    }

    if (IsEffectActive(EF_NODRAW))
        return 1;

    return BaseClass::DrawModel(flags);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, C_TriggerTimerStop);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStop, DT_TriggerTimerStop, CTriggerTimerStop)
END_RECV_TABLE();

bool C_TriggerTimerStop::ShouldDraw() { return true; }

int C_TriggerTimerStop::DrawModel(int flags)
{
    if (mom_endzone_outline_enable.GetBool() && (flags & STUDIO_RENDER) != 0 &&
        (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0)
    {
        if (g_pMomentumUtil->GetColorFromHex(mom_endzone_color.GetString(), m_OutlineRenderer.outlineColor))
        {
            if (GetModel())
            {
                render->InstallBrushSurfaceRenderer(&m_OutlineRenderer);
                BaseClass::DrawModel(flags);
                render->InstallBrushSurfaceRenderer(nullptr);
            }
            else
            {
                DrawOutlineModel(m_OutlineRenderer.outlineColor);
                return 1;
            }
        }
    }

    if (IsEffectActive(EF_NODRAW))
        return 1;

    return BaseClass::DrawModel(flags);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
RecvPropBool(RECVINFO(m_bStuckOnGround)),
RecvPropBool(RECVINFO(m_bAllowingJump)),
RecvPropBool(RECVINFO(m_bDisableGravity)),
RecvPropBool(RECVINFO(m_bFixUpsideSlope)), 
END_RECV_TABLE();