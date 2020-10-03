#include "cbase.h"
#include "c_mom_triggers.h"
#include "model_types.h"
#include "util/mom_util.h"

#include "momentum/mom_system_tricks.h"
#undef CTriggerTrickZone

#include "mom_shareddefs.h"
#include "dt_utlvector_recv.h"
#include "debugoverlay_shared.h"
#include "mom_player_shared.h"

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
    builder.Begin(pRenderContext->GetDynamicMesh(true, nullptr, nullptr, 
                                                 materials->FindMaterial("momentum/zone_outline", TEXTURE_GROUP_OTHER)),
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

    if (iNum <= 2)
        return;

    // Bottom
    for (int i = 0; i < iNum; i++)
    {
        const auto cur = m_vecZonePoints[i];
        const auto next = i == (iNum - 1) ? m_vecZonePoints[0] : m_vecZonePoints[i+1];
        debugoverlay->AddLineOverlayAlpha(cur, next, outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a(), false, 0.0f);
    }

    // Connecting lines
    for (int i = 0; i < iNum; i++)
    {
        const Vector &cur = m_vecZonePoints[i];
        const Vector next(cur.x, cur.y, cur.z + m_flZoneHeight);
        debugoverlay->AddLineOverlayAlpha(cur, next, outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a(), false, 0.0f);
    }

    // Top
    for (int i = 0; i < iNum; i++)
    {
        auto cur = m_vecZonePoints[i];
        auto next = i == (iNum - 1) ? m_vecZonePoints[0] : m_vecZonePoints[i + 1];
        Vector curUp(cur.x, cur.y, cur.z + m_flZoneHeight);
        Vector nextUp(next.x, next.y, next.z + m_flZoneHeight);
        debugoverlay->AddLineOverlayAlpha(curUp, nextUp, outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a(), false, 0.0f);
    }
}

bool C_BaseMomZoneTrigger::ShouldDraw()
{
    return true;
}

int C_BaseMomZoneTrigger::DrawModel(int flags)
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return 0;

    const auto pRunEntData = pPlayer->GetCurrentUIEntData();
    if (!pRunEntData)
        return 0;

    const auto pStartZone = dynamic_cast<C_TriggerTimerStart*>(this);
    if (!pStartZone && m_iTrackNumber != pRunEntData->m_iCurrentTrack)
        return 0;

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

// ====================================

LINK_ENTITY_TO_CLASS(trigger_momentum_trick, C_TriggerTrickZone);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTrickZone, DT_TriggerTrickZone, CTriggerTrickZone)
RecvPropInt(RECVINFO(m_iID)),
RecvPropString(RECVINFO(m_szZoneName)),
RecvPropInt(RECVINFO(m_iDrawState)),
END_RECV_TABLE();

C_TriggerTrickZone::C_TriggerTrickZone()
{
    m_iID = -1;
    m_szZoneName.GetForModify()[0] = '\0';
    m_iDrawState = TRICK_DRAW_NONE;
}

bool C_TriggerTrickZone::ShouldDrawOutline()
{
    return m_iDrawState > TRICK_DRAW_NONE;
}

bool C_TriggerTrickZone::GetOutlineColor()
{
    // MOM_TODO allow customization here

    Color clr;
    if (m_iDrawState == TRICK_DRAW_REQUIRED)
        clr = COLOR_ORANGE;
    else if (m_iDrawState == TRICK_DRAW_OPTIONAL)
        clr = COLOR_WHITE;
    else if (m_iDrawState == TRICK_DRAW_END)
        clr = COLOR_RED;
    else
        clr = COLOR_GREEN;

    m_OutlineRenderer.outlineColor = clr;
    return true;
}

void C_TriggerTrickZone::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    if (type == DATA_UPDATE_CREATED)
    {
        g_pTrickSystem->AddZone(this);
    }
}

// ======================================
LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
RecvPropBool(RECVINFO(m_bStuckOnGround)),
RecvPropBool(RECVINFO(m_bAllowingJump)),
RecvPropBool(RECVINFO(m_bDisableGravity)),
END_RECV_TABLE();