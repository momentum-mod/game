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

enum ZoneDrawMode_t
{
    MOM_ZONE_DRAW_MODE_NONE = 0,
    MOM_ZONE_DRAW_MODE_OUTLINE,
    MOM_ZONE_DRAW_MODE_OUTLINE_OVERLAY,
    MOM_ZONE_DRAW_MODE_FACES,
    MOM_ZONE_DRAW_MODE_FACES_OVERLAY,

    MOM_ZONE_DRAW_MODE_FIRST = MOM_ZONE_DRAW_MODE_NONE,
    MOM_ZONE_DRAW_MODE_LAST = MOM_ZONE_DRAW_MODE_FACES_OVERLAY
};

static MAKE_CONVAR(mom_zone_start_draw_mode, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Changes the drawing mode for start zones.\n"
                   "0 = Off; None, 1 = Outlines (Default), 2 = Outlines as overlays (see through walls), 3 = Side faces, 4 = Side faces as overlays (see through walls).\n", MOM_ZONE_DRAW_MODE_FIRST, MOM_ZONE_DRAW_MODE_LAST);
static MAKE_CONVAR(mom_zone_end_draw_mode, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Changes the drawing mode for end zones.\n"
                   "0 = Off; None, 1 = Outlines (Default), 2 = Outlines as overlays (see through walls), 3 = Side faces, 4 = Side faces as overlays (see through walls).\n", MOM_ZONE_DRAW_MODE_FIRST, MOM_ZONE_DRAW_MODE_LAST);
static MAKE_CONVAR(mom_zone_stage_draw_mode, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Changes the drawing mode for stage zones.\n"
                   "0 = Off; None, 1 = Outlines (Default), 2 = Outlines as overlays (see through walls), 3 = Side faces, 4 = Side faces as overlays (see through walls).\n", MOM_ZONE_DRAW_MODE_FIRST, MOM_ZONE_DRAW_MODE_LAST);
static MAKE_CONVAR(mom_zone_checkpoint_draw_mode, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Changes the drawing mode for checkpoint zones.\n"
                   "0 = Off; None, 1 = Outlines (Default), 2 = Outlines as overlays (see through walls), 3 = Side faces, 4 = Side faces as overlays (see through walls).\n", MOM_ZONE_DRAW_MODE_FIRST, MOM_ZONE_DRAW_MODE_LAST);

static ConVar mom_zone_start_draw_color("mom_zone_start_draw_color", "00FF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the start zones.\n");
static ConVar mom_zone_end_draw_color("mom_zone_end_draw_color", "FF0000FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the end zones.\n");
static ConVar mom_zone_stage_draw_color("mom_zone_stage_draw_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the stage zones.\n");
static ConVar mom_zone_checkpoint_draw_color("mom_zone_checkpoint_draw_color", "FFFF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the checkpoint zones.\n");

static MAKE_TOGGLE_CONVAR(mom_zone_draw_alpha_override_toggle, "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Toggles the alpha override for drawing zone faces.\n");
static MAKE_CONVAR(mom_zone_draw_faces_alpha_override, "160", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Alpha override for drawing zone faces.\n", 0, 255);

static MAKE_TOGGLE_CONVAR(mom_teledests_draw, "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "Toggles drawing teleport destination markings\n");
static MAKE_CONVAR(mom_teledests_dimensions, "32", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Changes the dimensions of drawn teleport destination markings\n", 1, 128);
static ConVar mom_teledests_color("mom_teledests_color", "FFFFFFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Color of the teleport destination markings.\n");

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

    bool bDrawingFace = m_iRenderMode == MOM_ZONE_DRAW_MODE_FACES || m_iRenderMode == MOM_ZONE_DRAW_MODE_FACES_OVERLAY;
    bool bOverlay = m_iRenderMode == MOM_ZONE_DRAW_MODE_FACES_OVERLAY || m_iRenderMode == MOM_ZONE_DRAW_MODE_OUTLINE_OVERLAY;
    const auto pMaterial = materials->FindMaterial(bOverlay ? MOM_ZONE_DRAW_MATERIAL_OVERLAY : MOM_ZONE_DRAW_MATERIAL, TEXTURE_GROUP_OTHER);

    CMeshBuilder builder;
    builder.Begin(pRenderContext->GetDynamicMesh(true, nullptr, nullptr, pMaterial),
                  bDrawingFace ? MATERIAL_POLYGON : MATERIAL_LINE_LOOP, vertices);

    int iAlpha = bDrawingFace && mom_zone_draw_alpha_override_toggle.GetBool() ? mom_zone_draw_faces_alpha_override.GetInt() : m_Color.a();

    for (int i = 0; i < vertices; i++)
    {
        const BrushVertex_t& vertex = m_pVertices[i];

        builder.Position3fv(vertex.m_Pos.Base());
        builder.Normal3fv(vertex.m_Normal.Base());
        builder.Color4ub(m_Color.r(), m_Color.g(), m_Color.b(), iAlpha);
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

void C_BaseMomZoneTrigger::DrawZoneOutlines(bool bOverlay)
{
    const int iNum = m_vecZonePoints.Count();
    if (iNum <= 2)
        return;

    Color color = m_ZoneModelRenderer.m_Color;

    CMatRenderContextPtr pRenderContext(materials);
    const auto pMaterial = materials->FindMaterial(bOverlay ? MOM_ZONE_DRAW_MATERIAL_OVERLAY : MOM_ZONE_DRAW_MATERIAL, TEXTURE_GROUP_OTHER);
    IMesh *pMesh = pRenderContext->GetDynamicMesh(true, nullptr, nullptr, pMaterial);
    CMeshBuilder builder;

    // Bottom
    builder.Begin(pMesh, MATERIAL_LINE_LOOP, iNum);
    for (int i = 0; i < iNum; i++)
    {
        builder.Position3fv(m_vecZonePoints[i].Base());
        builder.Color4ub(color.r(), color.g(), color.b(), color.a());
        builder.AdvanceVertex();
    }
    builder.End(false, true);

    // Connecting lines
    for (int i = 0; i < iNum; i++)
    {
        // Connecting lines
        builder.Begin(pMesh, MATERIAL_LINES, 2);

        builder.Position3fv(m_vecZonePoints[i].Base());
        builder.Color4ub(color.r(), color.g(), color.b(), color.a());
        builder.AdvanceVertex();

        builder.Position3f(m_vecZonePoints[i].x, m_vecZonePoints[i].y, m_vecZonePoints[i].z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), color.a());
        builder.AdvanceVertex();

        builder.End(false, true);
    }

    // Top
    builder.Begin(pMesh, MATERIAL_LINE_LOOP, iNum);
    for (int i = 0; i < iNum; i++)
    {
        builder.Position3f(m_vecZonePoints[i].x, m_vecZonePoints[i].y, m_vecZonePoints[i].z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), color.a());
        builder.AdvanceVertex();
    }
    builder.End(false, true);
}

void C_BaseMomZoneTrigger::DrawZoneFaces(bool bOverlay)
{
    const int iNum = m_vecZonePoints.Count();
    if (iNum <= 2)
        return;

    Color color = m_ZoneModelRenderer.m_Color;

    const int faceAlpha = mom_zone_draw_alpha_override_toggle.GetBool() ? mom_zone_draw_faces_alpha_override.GetInt() : color.a();

    CMatRenderContextPtr pRenderContext(materials);
    const auto pMaterial = materials->FindMaterial(bOverlay ? MOM_ZONE_DRAW_MATERIAL_OVERLAY : MOM_ZONE_DRAW_MATERIAL, TEXTURE_GROUP_OTHER);
    IMesh *pMesh = pRenderContext->GetDynamicMesh(true, nullptr, nullptr, pMaterial);
    CMeshBuilder builder;

    Vector center = m_vecZonePoints[0];
    for (int i = 1; i < iNum; i++)
        center += m_vecZonePoints[i];
    center /= iNum;

    // Bottom
    builder.Begin(pMesh, MATERIAL_TRIANGLES, iNum);
    for (int i = iNum - 1; i >= 0; --i)
    {
        const auto &vecCurr = m_vecZonePoints[i];
        const auto &vecNext = m_vecZonePoints[(i + 1) % iNum];

        builder.Position3fv(center.Base());
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3fv(vecCurr.Base());
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3fv(vecNext.Base());
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();
    }
    builder.End(false, true);
    if (!bOverlay)
    {
        builder.Begin(pMesh, MATERIAL_TRIANGLES, iNum);
        for (int i = iNum - 1; i >= 0; --i)
        {
            const auto &vecCurr = m_vecZonePoints[i];
            const auto &vecNext = m_vecZonePoints[(i + 1) % iNum];

            builder.Position3fv(center.Base());
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3fv(vecNext.Base());
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3fv(vecCurr.Base());
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();
        }
        builder.End(false, true);
        
    }

    // Middle
    for (int i = 0; i < iNum; i++)
    {
        const auto& vecCurr = m_vecZonePoints[i];
        const auto& vecNext = m_vecZonePoints[(i + 1) % iNum];

        builder.Begin(pMesh, MATERIAL_QUADS, 4);

        builder.Position3fv(vecCurr.Base());
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3f(vecCurr.x, vecCurr.y, vecCurr.z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3f(vecNext.x, vecNext.y, vecNext.z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3fv(vecNext.Base());
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();
        
        builder.End(false, true);
    }
    if (!bOverlay)
    {
        for (int i = 0; i < iNum; i++)
        {
            const auto &vecCurr = m_vecZonePoints[i];
            const auto &vecNext = m_vecZonePoints[(i + 1) % iNum];

            builder.Begin(pMesh, MATERIAL_QUADS, 4);

            builder.Position3fv(vecNext.Base());
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3f(vecNext.x, vecNext.y, vecNext.z + m_flZoneHeight);
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3f(vecCurr.x, vecCurr.y, vecCurr.z + m_flZoneHeight);
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3fv(vecCurr.Base());
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.End(false, true);
        }
    }

    // Top
    builder.Begin(pMesh, MATERIAL_TRIANGLES, iNum);
    for (int i = iNum - 1; i >= 0; --i)
    {
        const auto &vecCurr = m_vecZonePoints[i];
        const auto &vecNext = m_vecZonePoints[(i + 1) % iNum];

        builder.Position3f(center.x, center.y, center.z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3f(vecNext.x, vecNext.y, vecNext.z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();

        builder.Position3f(vecCurr.x, vecCurr.y, vecCurr.z + m_flZoneHeight);
        builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
        builder.AdvanceVertex();
    }
    builder.End(false, true);
    if (!bOverlay)
    {
        builder.Begin(pMesh, MATERIAL_TRIANGLES, iNum);
        for (int i = iNum - 1; i >= 0; --i)
        {
            const auto &vecCurr = m_vecZonePoints[i];
            const auto &vecNext = m_vecZonePoints[(i + 1) % iNum];

            builder.Position3f(center.x, center.y, center.z + m_flZoneHeight);
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3f(vecCurr.x, vecCurr.y, vecCurr.z + m_flZoneHeight);
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();

            builder.Position3f(vecNext.x, vecNext.y, vecNext.z + m_flZoneHeight);
            builder.Color4ub(color.r(), color.g(), color.b(), faceAlpha);
            builder.AdvanceVertex();
        }
        builder.End(false, true);
    }
}

int C_BaseMomZoneTrigger::DrawModel(int flags)
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return 0;

    const auto pRunEntData = pPlayer->GetCurrentUIEntData();
    if (!pRunEntData)
        return 0;

    if (GetZoneType() != ZONE_TYPE_START && m_iTrackNumber != pRunEntData->m_iCurrentTrack)
        return 0;

    int iRenderMode = GetDrawMode();
    if (iRenderMode == MOM_ZONE_DRAW_MODE_NONE)
        return 1;

    if (iRenderMode && flags & STUDIO_RENDER && (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0 && GetDrawColor())
    {
        if (GetModel())
        {
            m_ZoneModelRenderer.m_iRenderMode = iRenderMode;
            render->InstallBrushSurfaceRenderer(&m_ZoneModelRenderer);
            BaseClass::DrawModel(flags);
            render->InstallBrushSurfaceRenderer(nullptr);
        }
        else
        {
            if (iRenderMode >= MOM_ZONE_DRAW_MODE_FACES)
            {
                DrawZoneFaces((iRenderMode & 1) == 0); // overlays are even
            }
            else
            {
                DrawZoneOutlines((iRenderMode & 1) == 0); // overlays are even
            }
            return 1;
        }
    }

    if (IsEffectActive(EF_NODRAW))
        return 1;

    return BaseClass::DrawModel(flags);
}

void C_BaseMomZoneTrigger::Spawn()
{
    BaseClass::Spawn();
    Precache();
}

void C_BaseMomZoneTrigger::Precache()
{
    BaseClass::Precache();
    PrecacheMaterial(MOM_ZONE_DRAW_MATERIAL);
    PrecacheMaterial(MOM_ZONE_DRAW_MATERIAL_OVERLAY);
}

int C_BaseMomZoneTrigger::GetZoneType()
{
    return ZONE_TYPE_INVALID;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, C_TriggerTimerStart);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStart, DT_TriggerTimerStart, CTriggerTimerStart)
END_RECV_TABLE();

bool C_TriggerTimerStart::GetDrawColor()
{
    return MomUtil::GetColorFromHex(mom_zone_start_draw_color.GetString(), m_ZoneModelRenderer.m_Color);
}

int C_TriggerTimerStart::GetDrawMode()
{
    return mom_zone_start_draw_mode.GetInt();
}

int C_TriggerTimerStart::GetZoneType()
{
    return ZONE_TYPE_START;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, C_TriggerTimerStop);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStop, DT_TriggerTimerStop, CTriggerTimerStop)
END_RECV_TABLE();

bool C_TriggerTimerStop::GetDrawColor()
{
    return MomUtil::GetColorFromHex(mom_zone_end_draw_color.GetString(), m_ZoneModelRenderer.m_Color);
}

int C_TriggerTimerStop::GetDrawMode()
{
    return mom_zone_end_draw_mode.GetInt();
}

int C_TriggerTimerStop::GetZoneType()
{
    return ZONE_TYPE_STOP;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stage, C_TriggerStage);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerStage, DT_TriggerStage, CTriggerStage)
END_RECV_TABLE();

bool C_TriggerStage::GetDrawColor()
{
    return MomUtil::GetColorFromHex(mom_zone_stage_draw_color.GetString(), m_ZoneModelRenderer.m_Color);
}

int C_TriggerStage::GetDrawMode()
{
    return mom_zone_stage_draw_mode.GetInt();
}

int C_TriggerStage::GetZoneType()
{
    return ZONE_TYPE_STAGE;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, C_TriggerCheckpoint);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerCheckpoint, DT_TriggerCheckpoint, CTriggerCheckpoint)
END_RECV_TABLE();

bool C_TriggerCheckpoint::GetDrawColor()
{
    return MomUtil::GetColorFromHex(mom_zone_checkpoint_draw_color.GetString(), m_ZoneModelRenderer.m_Color);
}

int C_TriggerCheckpoint::GetDrawMode()
{
    return mom_zone_checkpoint_draw_mode.GetInt();
}

int C_TriggerCheckpoint::GetZoneType()
{
    return ZONE_TYPE_CHECKPOINT;
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

int C_TriggerTrickZone::GetDrawMode()
{
    return m_iDrawState > TRICK_DRAW_NONE;
}

int C_TriggerTrickZone::GetZoneType()
{
    return ZONE_TYPE_TRICK;
}

bool C_TriggerTrickZone::GetDrawColor()
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

    m_ZoneModelRenderer.m_Color = clr;
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

// ======================================

LINK_ENTITY_TO_CLASS(info_teleport_destination, C_TeleportDestination);

IMPLEMENT_CLIENTCLASS_DT(C_TeleportDestination, DT_TeleportDestination, CTeleportDestination)
END_RECV_TABLE();

int C_TeleportDestination::DrawModel(int flags)
{
    if (!mom_teledests_draw.GetBool())
        return 1;

    Color color;
    if (!MomUtil::GetColorFromHex(mom_teledests_color.GetString(), color))
        return 1;

    const auto origin = GetAbsOrigin();
    float dim = mom_teledests_dimensions.GetFloat() / 2.0f;

    CMatRenderContextPtr pRenderContext(materials);
    const auto pMaterial = materials->FindMaterial(MOM_ZONE_DRAW_MATERIAL, TEXTURE_GROUP_OTHER);
    IMesh *pMesh = pRenderContext->GetDynamicMesh(true, nullptr, nullptr, pMaterial);
    CMeshBuilder builder;

    builder.Begin(pMesh, MATERIAL_QUADS, 4);

    builder.Position3f(origin.x + dim, origin.y + dim, origin.z);
    builder.Color4ub(color.r(), color.g(), color.b(), color.a());
    builder.AdvanceVertex();

    builder.Position3f(origin.x + dim, origin.y - dim, origin.z);
    builder.Color4ub(color.r(), color.g(), color.b(), color.a());
    builder.AdvanceVertex();

    builder.Position3f(origin.x - dim, origin.y - dim, origin.z);
    builder.Color4ub(color.r(), color.g(), color.b(), color.a());
    builder.AdvanceVertex();

    builder.Position3f(origin.x - dim, origin.y + dim, origin.z);
    builder.Color4ub(color.r(), color.g(), color.b(), color.a());
    builder.AdvanceVertex();

    builder.End(false, true);

    return 1;
}

void C_TeleportDestination::Spawn()
{
    Precache();
    BaseClass::Spawn();
}

void C_TeleportDestination::Precache()
{
    BaseClass::Precache();
    PrecacheMaterial(MOM_ZONE_DRAW_MATERIAL);
}
