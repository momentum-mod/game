#include "cbase.h"
#include "c_mom_triggers.h"
#include "model_types.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

const int SF_TELEPORT_PRESERVE_ANGLES = 0x20; // Preserve angles even when a local landmark is not specified

static ConVar mom_startzone_outline_enable("mom_startzone_outline_enable", "1",
                                           FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                           "Enable outline for start zone.");

static ConVar mom_endzone_outline_enable("mom_endzone_outline_enable", "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                         "Enable outline for end zone.");

static ConVar mom_startzone_color("mom_startzone_color", "00FF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Color of the start zone.");

static ConVar mom_endzone_color("mom_endzone_color", "FF0000FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                "Color of the end zone.");

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, C_TriggerTimerStart);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStart, DT_TriggerTimerStart, CTriggerTimerStart)
END_RECV_TABLE();

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, C_TriggerTimerStop);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStop, DT_TriggerTimerStop, CTriggerTimerStop)
END_RECV_TABLE();

static class CTriggerOutlineRenderer : public IBrushRenderer, public CAutoGameSystem
{
  public:
    CTriggerOutlineRenderer()
    {
        m_pVertices = NULL;
        m_verticeCount = 0;
    }

    bool RenderBrushModelSurface(IClientEntity *pBaseEntity, IBrushSurface *pBrushSurface) OVERRIDE
    {
        const int vertices = pBrushSurface->GetVertexCount();
        if (vertices > m_verticeCount)
        {
            m_verticeCount = vertices;
            if (m_pVertices == NULL)
                m_pVertices =
                    static_cast<BrushVertex_t *>(MemAlloc_AllocAligned(sizeof(BrushVertex_t) * m_verticeCount, 64));
            else
                m_pVertices = static_cast<BrushVertex_t *>(
                    MemAlloc_ReallocAligned(m_pVertices, sizeof(BrushVertex_t) * m_verticeCount, 64));
        }
        pBrushSurface->GetVertexData(m_pVertices);
        CMatRenderContextPtr pRenderContext(materials);

        CMeshBuilder builder;
        builder.Begin(pRenderContext->GetDynamicMesh(true, 0, 0, outlineMaterial), MATERIAL_LINE_LOOP, vertices);
        for (int i = 0; i < vertices; i++)
        {
            const BrushVertex_t &vertex = m_pVertices[i];
            builder.Position3fv(vertex.m_Pos.Base());
            builder.Normal3fv(vertex.m_Normal.Base());
            builder.Color4ub(outlineColor.r(), outlineColor.g(), outlineColor.b(), outlineColor.a());
            builder.AdvanceVertex();
        }

        builder.End(false, true);

        return false;
    }

    bool Init() OVERRIDE
    {
        KeyValues *pVMTKeyValues = new KeyValues("unlitgeneric");
        pVMTKeyValues->SetString("$vertexcolor", "1");
        pVMTKeyValues->SetString("$vertexalpha", "1");
        pVMTKeyValues->SetString("$additive", "1");
        pVMTKeyValues->SetString("$ignorez", "0"); // Change this to 1 to see it through walls
        pVMTKeyValues->SetString("$halflambert", "1");
        pVMTKeyValues->SetString("$selfillum", "1");
        pVMTKeyValues->SetString("$nofog", "1");
        pVMTKeyValues->SetString("$nocull", "1");
        pVMTKeyValues->SetString("$model", "1");
        outlineMaterial.Init("__utilOutlineColor", pVMTKeyValues);
        outlineMaterial->Refresh();

        return true;
    }

    void Shutdown() OVERRIDE
    {
        MemAlloc_FreeAligned(m_pVertices);
        m_pVertices = NULL;
        outlineMaterial.Shutdown();
    }

    Color outlineColor;

  private:
    CMaterialReference outlineMaterial;
    BrushVertex_t *m_pVertices;
    int m_verticeCount;
} outlineRenderer;

bool C_TriggerTimerStart::ShouldDraw() { return true; }

int C_TriggerTimerStart::DrawModel(int flags)
{
    if (mom_startzone_outline_enable.GetBool() && (flags & STUDIO_RENDER) != 0 &&
        (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0)
    {
        if (g_pMomentumUtil->GetColorFromHex(mom_startzone_color.GetString(), outlineRenderer.outlineColor))
        {
            render->InstallBrushSurfaceRenderer(&outlineRenderer);
            BaseClass::DrawModel(STUDIO_RENDER);
            render->InstallBrushSurfaceRenderer(nullptr);
            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

bool C_TriggerTimerStop::ShouldDraw() { return true; }

int C_TriggerTimerStop::DrawModel(int flags)
{
    if (mom_startzone_outline_enable.GetBool() && (flags & STUDIO_RENDER) != 0 &&
        (flags & (STUDIO_SHADOWDEPTHTEXTURE | STUDIO_SHADOWDEPTHTEXTURE)) == 0)
    {
        if (g_pMomentumUtil->GetColorFromHex(mom_endzone_color.GetString(), outlineRenderer.outlineColor))
        {
            render->InstallBrushSurfaceRenderer(&outlineRenderer);
            BaseClass::DrawModel(STUDIO_RENDER);
            render->InstallBrushSurfaceRenderer(nullptr);
            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
	RecvPropBool(RECVINFO(m_bStuckOnGround)),
	RecvPropBool(RECVINFO(m_bAllowingJump)),
	RecvPropBool(RECVINFO(m_bDisableGravity)),
	RecvPropBool(RECVINFO(m_bFixUpsideSlope)),
END_RECV_TABLE();
