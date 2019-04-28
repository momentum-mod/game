#include "cbase.h"
#include "c_mom_triggers.h"
#include "dynamicrendertargets.h"
#include "model_types.h"
#include "util/mom_util.h"

#include "mom_shareddefs.h"
#include "dt_utlvector_recv.h"

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_zone_start_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable drawing an outline for start zone.");
static MAKE_TOGGLE_CONVAR(mom_zone_end_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable outline for end zone.");
static MAKE_TOGGLE_CONVAR(mom_zone_stage_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable outline for stage zone(s).");
static MAKE_TOGGLE_CONVAR(mom_zone_checkpoint_outline_enable, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Enable outline for checkpoint zone(s).");

static ConVar mom_zone_start_outline_color("mom_zone_start_outline_color", "00FF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the start zone.");
static ConVar mom_zone_end_outline_color("mom_zone_end_outline_color", "FF0000FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the end zone.");
static ConVar mom_zone_stage_outline_color("mom_zone_stage_outline_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the stage zone(s).");
static ConVar mom_zone_checkpoint_outline_color("mom_zone_checkpoint_outline_color", "FFFF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the checkpoint zone(s).");

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

IMPLEMENT_CLIENTCLASS_DT(C_BaseMomZoneTrigger, DT_BaseMomZoneTrigger, CBaseMomZoneTrigger)
RecvPropInt(RECVINFO(m_iTrackNumber)),
RecvPropUtlVector(RECVINFO_UTLVECTOR(m_vecZonePoints), 32, RecvPropVector(NULL, 0, sizeof(Vector))),
RecvPropFloat(RECVINFO(m_flZoneHeight)),
END_RECV_TABLE();

C_BaseMomZoneTrigger::C_BaseMomZoneTrigger()
{
    m_flZoneHeight = 0.0f;
    m_iTrackNumber = -1; // TRACK_ALL
}

void C_BaseMomZoneTrigger::DrawOutlineModel(const Color& outlineColor)
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

bool C_BaseMomZoneTrigger::ShouldDraw()
{
    return true;
}

int C_BaseMomZoneTrigger::DrawModel(int flags)
{
    if (ShouldDrawOutline())
    {
        if ((flags & STUDIO_RENDER) && (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0)
        {
            if (GetOutlineColor())
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
    }

    if (IsEffectActive(EF_NODRAW))
        return 1;

    return BaseClass::DrawModel(flags);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, C_TriggerTimerStart);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStart, DT_TriggerTimerStart, CTriggerTimerStart)
END_RECV_TABLE();

bool C_TriggerTimerStart::ShouldDrawOutline()
{
    return mom_zone_start_outline_enable.GetBool();
}

bool C_TriggerTimerStart::GetOutlineColor()
{
    return MomUtil::GetColorFromHex(mom_zone_start_outline_color.GetString(), m_OutlineRenderer.outlineColor);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, C_TriggerTimerStop);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStop, DT_TriggerTimerStop, CTriggerTimerStop)
END_RECV_TABLE();

bool C_TriggerTimerStop::ShouldDrawOutline()
{
    return mom_zone_end_outline_enable.GetBool();
}

bool C_TriggerTimerStop::GetOutlineColor()
{
    return MomUtil::GetColorFromHex(mom_zone_end_outline_color.GetString(), m_OutlineRenderer.outlineColor);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stage, C_TriggerStage);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerStage, DT_TriggerStage, CTriggerStage)
END_RECV_TABLE();

bool C_TriggerStage::ShouldDrawOutline()
{
    return mom_zone_stage_outline_enable.GetBool();
}

bool C_TriggerStage::GetOutlineColor()
{
    return MomUtil::GetColorFromHex(mom_zone_stage_outline_color.GetString(), m_OutlineRenderer.outlineColor);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, C_TriggerCheckpoint);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerCheckpoint, DT_TriggerCheckpoint, CTriggerCheckpoint)
END_RECV_TABLE();

bool C_TriggerCheckpoint::ShouldDrawOutline()
{
    return mom_zone_checkpoint_outline_enable.GetBool();
}

bool C_TriggerCheckpoint::GetOutlineColor()
{
    return MomUtil::GetColorFromHex(mom_zone_checkpoint_outline_color.GetString(), m_OutlineRenderer.outlineColor);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
RecvPropBool(RECVINFO(m_bStuckOnGround)),
RecvPropBool(RECVINFO(m_bAllowingJump)),
RecvPropBool(RECVINFO(m_bDisableGravity)),
RecvPropBool(RECVINFO(m_bFixUpsideSlope)), 
END_RECV_TABLE();