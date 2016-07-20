// ******************************************************
//
// Purpose:
//		-	Connects the shader editor
//		-	Sends data from the main viewsetup
//		-	exposes client callbacks to shaders
//
// ******************************************************

#include "cbase.h"

#include "ShaderEditor/IVShaderEditor.h"
#include "ShaderEditor/SEdit_ModelRender.h"
#include "beamdraw.h"
#include "c_rope.h"
#include "c_sun.h"
#include "client_factorylist.h"
#include "iviewrender.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "rendertexture.h"
#include "tier0/icommandline.h"
#include "view.h"
#include "view_scene.h"
#include "view_shared.h"
#include "viewrender.h"

#define Editor_MainViewOrigin MainViewOrigin()
#define Editor_MainViewForward MainViewForward()

ShaderEditorHandler __g_ShaderEditorSystem("ShEditUpdate");
ShaderEditorHandler *g_ShaderEditorSystem = &__g_ShaderEditorSystem;

CSysModule *shaderEditorModule = nullptr;
IVShaderEditor *shaderEdit = nullptr;

ShaderEditorHandler::ShaderEditorHandler(char const *name) : CAutoGameSystemPerFrame(name)
{
    m_bReady = false;
    m_piCurrentViewId = nullptr;
}

ShaderEditorHandler::~ShaderEditorHandler() {}

bool ShaderEditorHandler::IsReady() const { return m_bReady; }

bool ShaderEditorHandler::Init()
{
    factorylist_t factories;
    FactoryList_Retrieve(factories);

    ConVarRef devEnabled("developer", true);
    bool bShowPrimDebug = devEnabled.GetInt() != 0;

    bool bCreateEditor = (CommandLine() != nullptr) && (CommandLine()->FindParm("-shaderedit") != 0);
    SEDIT_SKYMASK_MODE iEnableSkymask = SKYMASK_OFF;

#ifdef SHADEREDITOR_FORCE_ENABLED
    bCreateEditor = true;
    iEnableSkymask = SKYMASK_QUARTER;
#endif

    char modulePath[MAX_PATH * 4];
    Q_snprintf(modulePath, sizeof(modulePath), "%s/bin/shadereditor_2013.dll\0", engine->GetGameDirectory());
    shaderEditorModule = Sys_LoadModule(modulePath);
    if (shaderEditorModule)
    {
        CreateInterfaceFn shaderEditorDLLFactory = Sys_GetFactory(shaderEditorModule);
        shaderEdit = shaderEditorDLLFactory
                         ? static_cast<IVShaderEditor *>(shaderEditorDLLFactory(SHADEREDIT_INTERFACE_VERSION, nullptr))
                         : NULL;

        if (!shaderEdit)
        {
            Warning("Unable to pull IVShaderEditor interface.\n");
        }
        else if (!shaderEdit->Init(factories.appSystemFactory, gpGlobals, sEditMRender, bCreateEditor, bShowPrimDebug,
                                   iEnableSkymask))
        {
            Warning("Cannot initialize IVShaderEditor.\n");
            shaderEdit = nullptr;
        }
    }
    else
    {
        Warning("Cannot load shadereditor.dll from %s!\n", modulePath);
    }

    m_bReady = shaderEdit != nullptr;

    RegisterCallbacks();
    RegisterViewRenderCallbacks();

    if (IsReady())
    {
        shaderEdit->PrecacheData();
    }

    return true;
}

#ifdef SHADEREDITOR_FORCE_ENABLED
CON_COMMAND(sedit_debug_toggle_ppe, "")
{
    if (!g_ShaderEditorSystem->IsReady())
        return Warning("lib not ready.\n");

    if (args.ArgC() < 2)
        return;

    const int idx = shaderEdit->GetPPEIndex(args[1]);
    if (idx < 0)
        return Warning("can't find ppe named: %s\n", args[1]);

    shaderEdit->SetPPEEnabled(idx, !shaderEdit->IsPPEEnabled(idx));
}
#endif

void ShaderEditorHandler::Shutdown()
{
    if (shaderEdit)
        shaderEdit->Shutdown();
    if (shaderEditorModule)
        Sys_UnloadModule(shaderEditorModule);
}

void ShaderEditorHandler::Update(float frametime)
{
    if (IsReady())
        shaderEdit->OnFrame(frametime);
}

CThreadMutex m_Lock;

void ShaderEditorHandler::PreRender()
{
    if (IsReady() && view)
    {
        // make sure the class matches
        const CViewSetup *v = view->GetPlayerViewSetup();
        CViewSetup_SEdit_Shared stableVSetup(*v);
        shaderEdit->OnPreRender(&stableVSetup);

        m_Lock.Lock();
        PrepareCallbackData();
        m_Lock.Unlock();
    }
}
void ShaderEditorHandler::PostRender() {}

void ShaderEditorHandler::CustomViewRender(int *viewId, const VisibleFogVolumeInfo_t &fogVolumeInfo,
                                           const WaterRenderInfo_t &waterRenderInfo)
{
    m_piCurrentViewId = viewId;
    m_tFogVolumeInfo = fogVolumeInfo;
    m_tWaterRenderInfo = waterRenderInfo;

    if (IsReady())
        shaderEdit->OnSceneRender();
}
void ShaderEditorHandler::UpdateSkymask(bool bCombineMode) const
{
    if (IsReady())
        shaderEdit->OnUpdateSkymask(bCombineMode);
}
void ShaderEditorHandler::CustomPostRender() const
{
    if (IsReady())
        shaderEdit->OnPostRender(true);
}

struct CallbackData_t
{
    void Reset()
    {
        sun_data.Init();
        sun_dir.Init();

        player_speed.Init();
        player_pos.Init();
    };
    Vector4D sun_data;
    Vector sun_dir;

    Vector4D player_speed;
    Vector player_pos;
};

static CallbackData_t clCallback_data;

void ShaderEditorHandler::PrepareCallbackData() const
{
    clCallback_data.Reset();

    float flSunAmt_Goal = 0;
    static float s_flSunAmt_Last = 0;

    C_BaseEntity *pEnt = ClientEntityList().FirstBaseEntity();
    while (pEnt)
    {
        if (!Q_stricmp(pEnt->GetClassname(), "class C_Sun"))
        {
            C_Sun *pSun = static_cast<C_Sun *>(pEnt);
            Vector dir = pSun->m_vDirection;
            dir.NormalizeInPlace();

            Vector screen;

            if (ScreenTransform(Editor_MainViewOrigin + dir * 512, screen))
                ScreenTransform((Editor_MainViewOrigin - dir * 512), screen);

            screen = screen * Vector(0.5f, -0.5f, 0) + Vector(0.5f, 0.5f, 0);

            Q_memcpy(clCallback_data.sun_data.Base(), screen.Base(), sizeof(float) * 2);
            clCallback_data.sun_data[2] = DotProduct(dir, Editor_MainViewForward);
            clCallback_data.sun_dir = dir;

            trace_t tr;
            UTIL_TraceLine(Editor_MainViewOrigin, Editor_MainViewOrigin + dir * MAX_TRACE_LENGTH, MASK_SOLID, nullptr,
                           COLLISION_GROUP_DEBRIS, &tr);
            if (!tr.DidHitWorld())
                break;

            if (tr.surface.flags & SURF_SKY)
                flSunAmt_Goal = 1;

            break;
        }
        pEnt = ClientEntityList().NextBaseEntity(pEnt);
    }

    if (s_flSunAmt_Last != flSunAmt_Goal)
        s_flSunAmt_Last =
            Approach(flSunAmt_Goal, s_flSunAmt_Last, gpGlobals->frametime * ((!!flSunAmt_Goal) ? 4.0f : 0.75f));

    clCallback_data.sun_data[3] = s_flSunAmt_Last;

    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if (pPlayer)
    {
        Vector velo = pPlayer->GetLocalVelocity();
        clCallback_data.player_speed[3] = velo.NormalizeInPlace();
        Q_memcpy(clCallback_data.player_speed.Base(), velo.Base(), sizeof(float) * 3);

        clCallback_data.player_pos = pPlayer->GetLocalOrigin();
    }
}

pFnClCallback_Declare(ClCallback_SunData)
{
    m_Lock.Lock();
    Q_memcpy(pfl4, clCallback_data.sun_data.Base(), sizeof(float) * 4);
    m_Lock.Unlock();
}

pFnClCallback_Declare(ClCallback_SunDirection)
{
    m_Lock.Lock();
    Q_memcpy(pfl4, clCallback_data.sun_dir.Base(), sizeof(float) * 3);
    m_Lock.Unlock();
}

pFnClCallback_Declare(ClCallback_PlayerVelocity)
{
    m_Lock.Lock();
    Q_memcpy(pfl4, clCallback_data.player_speed.Base(), sizeof(float) * 4);
    m_Lock.Unlock();
}

pFnClCallback_Declare(ClCallback_PlayerPos)
{
    m_Lock.Lock();
    Q_memcpy(pfl4, clCallback_data.player_pos.Base(), sizeof(float) * 3);
    m_Lock.Unlock();
}

void ShaderEditorHandler::RegisterCallbacks() const
{
    if (!IsReady())
        return;

    // 4 components max
    shaderEdit->RegisterClientCallback("sun data", ClCallback_SunData, 4);
    shaderEdit->RegisterClientCallback("sun dir", ClCallback_SunDirection, 3);
    shaderEdit->RegisterClientCallback("local player velocity", ClCallback_PlayerVelocity, 4);
    shaderEdit->RegisterClientCallback("local player position", ClCallback_PlayerPos, 3);

    shaderEdit->LockClientCallbacks();
}

extern bool DoesViewPlaneIntersectWater(float waterZ, int leafWaterDataID);

// copy pasta from baseworldview
class CBaseVCallbackView : public CRendering3dView
{
    DECLARE_CLASS(CBaseVCallbackView, CRendering3dView);

  protected:
    CBaseVCallbackView(CViewRender *pMainView) : CRendering3dView(pMainView){};

    virtual bool AdjustView(float waterHeight) { return false; };

    virtual void CallbackInitRenderList(int viewId) { BuildRenderableRenderLists(viewId); };

    virtual bool ShouldDrawParticles() { return true; };

    virtual bool ShouldDrawRopes() { return true; };

    virtual bool ShouldDrawWorld() { return true; };

    virtual bool ShouldDrawTranslucents() { return true; };

    virtual bool ShouldDrawTranslucentWorld() { return true; };

    void DrawSetup(float waterHeight, int nSetupFlags, float waterZAdjust, int iForceViewLeaf = -1)
    {
        int savedViewID = g_ShaderEditorSystem->GetViewIdForModify();

        g_ShaderEditorSystem->GetViewIdForModify() = VIEW_ILLEGAL;

        render->BeginUpdateLightmaps();

        bool bDrawEntities = (nSetupFlags & DF_DRAW_ENTITITES) != 0;
        BuildWorldRenderLists(bDrawEntities, iForceViewLeaf, true, false, nullptr);
        PruneWorldListInfo();

        if (bDrawEntities)
            CallbackInitRenderList(savedViewID);

        render->EndUpdateLightmaps();

        g_ShaderEditorSystem->GetViewIdForModify() = savedViewID;
    };

    void DrawExecute(float waterHeight, view_id_t viewID, float waterZAdjust)
    {
        // ClientWorldListInfo_t is defined in viewrender.cpp...
        // g_pClientShadowMgr->ComputeShadowTextures( *this, m_pWorldListInfo->m_LeafCount,
        // m_pWorldListInfo->m_pLeafList );

        engine->Sound_ExtraUpdate();

        int savedViewID = g_ShaderEditorSystem->GetViewIdForModify();
        g_ShaderEditorSystem->GetViewIdForModify() = viewID;

        int iDrawFlagsBackup = m_DrawFlags;
        m_DrawFlags |= m_pMainView->GetBaseDrawFlags();

        PushView(waterHeight);

        CMatRenderContextPtr pRenderContext(materials);

        ITexture *pSaveFrameBufferCopyTexture = pRenderContext->GetFrameBufferCopyTexture(0);
        if (engine->GetDXSupportLevel() >= 80)
        {
            pRenderContext->SetFrameBufferCopyTexture(GetPowerOfTwoFrameBufferTexture());
        }

        pRenderContext.SafeRelease();

        static ConVarRef translucentNoWorld("r_drawtranslucentworld");
        const int tnoWorldSaved = translucentNoWorld.GetInt();
        translucentNoWorld.SetValue(ShouldDrawWorld() ? 0 : 1);

        if (m_DrawFlags & DF_DRAW_ENTITITES)
        {
            if (ShouldDrawWorld())
                DrawWorld(waterZAdjust);

            DrawOpaqueRenderables_Custom(false);

            if (ShouldDrawTranslucents() && ShouldDrawTranslucentWorld())
                DrawTranslucentRenderables(false, false);
            else if (ShouldDrawTranslucents())
                DrawTranslucentRenderablesNoWorld(false);
            else if (ShouldDrawTranslucentWorld())
                DrawTranslucentWorldInLeaves(false);
        }
        else if (ShouldDrawWorld())
        {
            DrawWorld(waterZAdjust);

            if (ShouldDrawTranslucentWorld())
                DrawTranslucentWorldInLeaves(false);
        }

        translucentNoWorld.SetValue(tnoWorldSaved);

        if (CurrentViewID() != VIEW_MAIN && CurrentViewID() != VIEW_INTRO_CAMERA)
            PixelVisibility_EndCurrentView();

        pRenderContext.GetFrom(materials);
        pRenderContext->SetFrameBufferCopyTexture(pSaveFrameBufferCopyTexture);
        PopView();

        m_DrawFlags = iDrawFlagsBackup;

        g_ShaderEditorSystem->GetViewIdForModify() = savedViewID;
    };

    virtual void PushView(float waterHeight)
    {
        float spread = 2.0f;
        if (m_DrawFlags & DF_FUDGE_UP)
        {
            waterHeight += spread;
        }
        else
        {
            waterHeight -= spread;
        }

        MaterialHeightClipMode_t clipMode = MATERIAL_HEIGHTCLIPMODE_DISABLE;

        if ((m_DrawFlags & DF_CLIP_Z))
        {
            if (m_DrawFlags & DF_CLIP_BELOW)
            {
                clipMode = MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT;
            }
            else
            {
                clipMode = MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT;
            }
        }

        CMatRenderContextPtr pRenderContext(materials);

        if (m_ClearFlags & (VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR | VIEW_CLEAR_STENCIL))
        {
            if (m_ClearFlags & VIEW_CLEAR_OBEY_STENCIL)
            {
                pRenderContext->ClearBuffersObeyStencil((m_ClearFlags & VIEW_CLEAR_COLOR) != 0,
                                                        (m_ClearFlags & VIEW_CLEAR_DEPTH) != 0);
            }
            else
            {
                pRenderContext->ClearBuffers((m_ClearFlags & VIEW_CLEAR_COLOR) != 0,
                                             (m_ClearFlags & VIEW_CLEAR_DEPTH) != 0,
                                             (m_ClearFlags & VIEW_CLEAR_STENCIL) != 0);
            }
        }

        pRenderContext->SetHeightClipMode(clipMode);
        if (clipMode != MATERIAL_HEIGHTCLIPMODE_DISABLE)
        {
            pRenderContext->SetHeightClipZ(waterHeight);
        }
    };

    virtual void PopView()
    {
        CMatRenderContextPtr pRenderContext(materials);
        pRenderContext->SetHeightClipMode(MATERIAL_HEIGHTCLIPMODE_DISABLE);
    };

    void DrawOpaqueRenderables_Custom(bool bShadowDepth)
    {
        // if( !r_drawopaquerenderables.GetBool() )
        //	return;

        ConVarRef drawEntities("r_drawentities");
        if (!drawEntities.GetBool())
            return;

        render->SetBlend(1);

        const bool bRopes = ShouldDrawRopes();
        const bool bParticles = ShouldDrawParticles();

        //
        // Prepare to iterate over all leaves that were visible, and draw opaque things in them.
        //
        if (bRopes)
            RopeManager()->ResetRenderCache();
        if (bParticles)
            g_pParticleSystemMgr->ResetRenderCache();

        bool const bDrawopaquestaticpropslast = false; // r_drawopaquestaticpropslast.GetBool();

        //
        // First do the brush models
        //
        {
            CClientRenderablesList::CEntry *pEntitiesBegin, *pEntitiesEnd;
            pEntitiesBegin = m_pRenderablesList->m_RenderGroups[RENDER_GROUP_OPAQUE_BRUSH];
            pEntitiesEnd = pEntitiesBegin + m_pRenderablesList->m_RenderGroupCounts[RENDER_GROUP_OPAQUE_BRUSH];
            DrawOpaqueRenderables_DrawBrushModels(pEntitiesBegin, pEntitiesEnd, bShadowDepth);
        }

        //
        // Sort everything that's not a static prop
        //
        int numOpaqueEnts = 0;
        for (int bucket = 0; bucket < RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS; ++bucket)
            numOpaqueEnts += m_pRenderablesList->m_RenderGroupCounts[RENDER_GROUP_OPAQUE_ENTITY_HUGE + 2 * bucket];

        CUtlVector<C_BaseAnimating *> arrBoneSetupNpcsLast(
            static_cast<C_BaseAnimating **>(_alloca(numOpaqueEnts * sizeof(C_BaseAnimating *))), numOpaqueEnts,
            numOpaqueEnts);
        CUtlVector<CClientRenderablesList::CEntry> arrRenderEntsNpcsFirst(
            static_cast<CClientRenderablesList::CEntry *>(
                _alloca(numOpaqueEnts * sizeof(CClientRenderablesList::CEntry))),
            numOpaqueEnts, numOpaqueEnts);
        int numNpcs = 0, numNonNpcsAnimating = 0;

        for (int bucket = 0; bucket < RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS; ++bucket)
        {
            for (CClientRenderablesList::CEntry *const
                     pEntitiesBegin = m_pRenderablesList->m_RenderGroups[RENDER_GROUP_OPAQUE_ENTITY_HUGE + 2 * bucket],
                     *const pEntitiesEnd =
                         pEntitiesBegin +
                         m_pRenderablesList->m_RenderGroupCounts[RENDER_GROUP_OPAQUE_ENTITY_HUGE + 2 * bucket],
                     *itEntity = pEntitiesBegin;
                 itEntity < pEntitiesEnd; ++itEntity)
            {
                C_BaseEntity *pEntity =
                    itEntity->m_pRenderable ? itEntity->m_pRenderable->GetIClientUnknown()->GetBaseEntity() : NULL;
                if (pEntity)
                {
                    if (pEntity->IsNPC())
                    {
                        C_BaseAnimating *pba = assert_cast<C_BaseAnimating *>(pEntity);
                        arrRenderEntsNpcsFirst[numNpcs++] = *itEntity;
                        arrBoneSetupNpcsLast[numOpaqueEnts - numNpcs] = pba;

                        itEntity->m_pRenderable = nullptr; // We will render NPCs separately
                        itEntity->m_RenderHandle = NULL;

                        continue;
                    }
                    else if (pEntity->GetBaseAnimating())
                    {
                        C_BaseAnimating *pba = assert_cast<C_BaseAnimating *>(pEntity);
                        arrBoneSetupNpcsLast[numNonNpcsAnimating++] = pba;
                        // fall through
                    }
                }
            }
        }

        //
        // Draw static props + opaque entities from the biggest bucket to the smallest
        //
        {
            CClientRenderablesList::CEntry *pEnts[RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS][2];
            CClientRenderablesList::CEntry *pProps[RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS][2];

            for (int bucket = 0; bucket < RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS; ++bucket)
            {
                pEnts[bucket][0] = m_pRenderablesList->m_RenderGroups[RENDER_GROUP_OPAQUE_ENTITY_HUGE + 2 * bucket];
                pEnts[bucket][1] =
                    pEnts[bucket][0] +
                    m_pRenderablesList->m_RenderGroupCounts[RENDER_GROUP_OPAQUE_ENTITY_HUGE + 2 * bucket];

                pProps[bucket][0] = m_pRenderablesList->m_RenderGroups[RENDER_GROUP_OPAQUE_STATIC_HUGE + 2 * bucket];
                pProps[bucket][1] =
                    pProps[bucket][0] +
                    m_pRenderablesList->m_RenderGroupCounts[RENDER_GROUP_OPAQUE_STATIC_HUGE + 2 * bucket];
            }

            for (int bucket = 0; bucket < RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS; ++bucket)
            {
                if (bDrawopaquestaticpropslast)
                {
                    DrawOpaqueRenderables_Range(pEnts[bucket][0], pEnts[bucket][1], bShadowDepth);
                    DrawOpaqueRenderables_DrawStaticProps(pProps[bucket][0], pProps[bucket][1], bShadowDepth);
                }
                else
                {
                    DrawOpaqueRenderables_Range(pEnts[bucket][0], pEnts[bucket][1], bShadowDepth);
                    DrawOpaqueRenderables_DrawStaticProps(pProps[bucket][0], pProps[bucket][1], bShadowDepth);
                }
            }
        }

        //
        // Draw NPCs now
        //
        DrawOpaqueRenderables_Range(arrRenderEntsNpcsFirst.Base(), arrRenderEntsNpcsFirst.Base() + numNpcs,
                                    bShadowDepth);
        //
        // Ropes and particles
        //
        if (bRopes)
            RopeManager()->DrawRenderCache(bShadowDepth);
        if (bParticles)
            g_pParticleSystemMgr->DrawRenderCache(bShadowDepth);
    };
    void DrawOpaqueRenderable(IClientRenderable *pEnt, bool bTwoPass, bool bShadowDepth)
    {
        float color[3];

        pEnt->GetColorModulation(color);
        render->SetColorModulation(color);

        int flags = STUDIO_RENDER;
        if (bTwoPass)
        {
            flags |= STUDIO_TWOPASS;
        }

        if (bShadowDepth)
        {
            flags |= STUDIO_SHADOWDEPTHTEXTURE;
        }

        float *pRenderClipPlane = pEnt->GetRenderClipPlane();

        if (pRenderClipPlane)
        {
            CMatRenderContextPtr pRenderContext(materials);
            if (!materials->UsingFastClipping()) // do NOT change the fast clip plane mid-scene, depth problems result.
                                                 // Regular user clip planes are fine though
                pRenderContext->PushCustomClipPlane(pRenderClipPlane);
#if DEBUG
            else
                AssertMsg(0, "can't link DrawClippedDepthBox externally so you either have to cope with even more "
                             "redundancy or move all this crap to viewrender");
#endif
            //	DrawClippedDepthBox( pEnt, pRenderClipPlane );
            Assert(view->GetCurrentlyDrawingEntity() == NULL);
            view->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
            pEnt->DrawModel(flags);
            view->SetCurrentlyDrawingEntity(nullptr);
            if (pRenderClipPlane && !materials->UsingFastClipping())
                pRenderContext->PopCustomClipPlane();
        }
        else
        {
            Assert(view->GetCurrentlyDrawingEntity() == NULL);
            view->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
            pEnt->DrawModel(flags);
            view->SetCurrentlyDrawingEntity(nullptr);
        }
    }

    void DrawOpaqueRenderables_DrawBrushModels(CClientRenderablesList::CEntry *pEntitiesBegin,
                                               CClientRenderablesList::CEntry *pEntitiesEnd, bool bShadowDepth)
    {
        for (CClientRenderablesList::CEntry *itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
            DrawOpaqueRenderable(itEntity->m_pRenderable, false, bShadowDepth);
    }

    void DrawOpaqueRenderables_DrawStaticProps(CClientRenderablesList::CEntry *pEntitiesBegin,
                                               CClientRenderablesList::CEntry *pEntitiesEnd, bool bShadowDepth)
    {
        if (pEntitiesEnd == pEntitiesBegin)
            return;

        float one[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        render->SetColorModulation(one);
        render->SetBlend(1.0f);

        const int MAX_STATICS_PER_BATCH = 512;
        IClientRenderable *pStatics[MAX_STATICS_PER_BATCH];

        int numScheduled = 0, numAvailable = MAX_STATICS_PER_BATCH;

        for (CClientRenderablesList::CEntry *itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
        {
            if (itEntity->m_pRenderable)
                NULL;
            else
                continue;

            pStatics[numScheduled++] = itEntity->m_pRenderable;
            if (--numAvailable > 0)
                continue; // place a hint for compiler to predict more common case in the loop

            staticpropmgr->DrawStaticProps(pStatics, numScheduled, bShadowDepth, vcollide_wireframe.GetBool());
            numScheduled = 0;
            numAvailable = MAX_STATICS_PER_BATCH;
        }

        if (numScheduled)
            staticpropmgr->DrawStaticProps(pStatics, numScheduled, bShadowDepth, vcollide_wireframe.GetBool());
    }

    void DrawOpaqueRenderables_Range(CClientRenderablesList::CEntry *pEntitiesBegin,
                                     CClientRenderablesList::CEntry *pEntitiesEnd, bool bShadowDepth)
    {
        for (CClientRenderablesList::CEntry *itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
            if (itEntity->m_pRenderable)
                DrawOpaqueRenderable(itEntity->m_pRenderable, (itEntity->m_TwoPass != 0), bShadowDepth);
    }
};

class CSimpleVCallbackView : public CBaseVCallbackView
{
    DECLARE_CLASS(CSimpleVCallbackView, CBaseVCallbackView);

  public:
    CSimpleVCallbackView(CViewRender *pMainView) : CBaseVCallbackView(pMainView) {}

    struct EditorViewSettings
    {
        bool bDrawPlayers;
        bool bDrawWeapons;
        bool bDrawStaticProps;
        bool bDrawMisc;
        bool bDrawTranslucents;
        bool bDrawWater;
        bool bDrawWorld;
        bool bDrawParticles;
        bool bDrawRopes;
        bool bDrawSkybox;
        bool bClipSkybox;
        bool bClearColor;
        bool bClearDepth;
        bool bClearStencil;
        bool bClearObeyStencil;
        bool bFogOverride;
        bool bFogEnabled;

        int iClearColorR;
        int iClearColorG;
        int iClearColorB;
        int iClearColorA;
        int iFogColorR;
        int iFogColorG;
        int iFogColorB;

        float flFogStart;
        float flFogEnd;
        float flFogDensity;
    };

    EditorViewSettings settings;

    void Setup(const CViewSetup &view, CSimpleVCallbackView::EditorViewSettings settings,
               const VisibleFogVolumeInfo_t &fogInfo, const WaterRenderInfo_t &info)
    {
        this->settings = settings;

        BaseClass::Setup(view);

        m_ClearFlags = (settings.bClearColor ? VIEW_CLEAR_COLOR : 0) | (settings.bClearDepth ? VIEW_CLEAR_DEPTH : 0) |
                       (settings.bClearStencil ? VIEW_CLEAR_STENCIL : 0) |
                       (settings.bClearObeyStencil ? VIEW_CLEAR_OBEY_STENCIL : 0);

        m_DrawFlags = (settings.bDrawPlayers || settings.bDrawStaticProps || settings.bDrawTranslucents ||
                       settings.bDrawWeapons || settings.bDrawMisc)
                          ? DF_DRAW_ENTITITES
                          : 0;

        // if ( settings.bDrawWorld )
        {
            if (!info.m_bOpaqueWater)
            {
                m_DrawFlags |= DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER;
            }
            else
            {
                bool bViewIntersectsWater =
                    DoesViewPlaneIntersectWater(fogInfo.m_flWaterHeight, fogInfo.m_nVisibleFogVolume);
                if (bViewIntersectsWater)
                {
                    // have to draw both sides if we can see both.
                    m_DrawFlags |= DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER;
                }
                else if (fogInfo.m_bEyeInFogVolume)
                {
                    m_DrawFlags |= DF_RENDER_UNDERWATER;
                }
                else
                {
                    m_DrawFlags |= DF_RENDER_ABOVEWATER;
                }
            }
        }

        if (info.m_bDrawWaterSurface && settings.bDrawWater)
        {
            m_DrawFlags |= DF_RENDER_WATER;
        }

        if (!fogInfo.m_bEyeInFogVolume && settings.bDrawSkybox)
        {
            m_DrawFlags |= DF_DRAWSKYBOX;
        }

        if (settings.bClipSkybox)
            m_DrawFlags |= DF_CLIP_SKYBOX;

        m_pCustomVisibility = nullptr;
        m_fogInfo = fogInfo;
    };

    void Draw() override
    {
        DrawSetup(0, m_DrawFlags, 0);

        CMatRenderContextPtr pRenderContext(materials);

        pRenderContext->ClearColor4ub(
            static_cast<unsigned char>(settings.iClearColorR), static_cast<unsigned char>(settings.iClearColorG),
            static_cast<unsigned char>(settings.iClearColorB), static_cast<unsigned char>(settings.iClearColorA));

        if (settings.bFogOverride)
        {
            if (!settings.bFogEnabled)
                pRenderContext->FogMode(MATERIAL_FOG_NONE);
            else
            {
                pRenderContext->FogMode(MATERIAL_FOG_LINEAR);
                pRenderContext->FogColor3ub(static_cast<unsigned char>(settings.iFogColorR),
                                            static_cast<unsigned char>(settings.iFogColorG),
                                            static_cast<unsigned char>(settings.iFogColorB));
                pRenderContext->FogStart(settings.flFogStart);
                pRenderContext->FogEnd(settings.flFogEnd);
                pRenderContext->FogMaxDensity(settings.flFogDensity);
            }
        }
        else if (!m_fogInfo.m_bEyeInFogVolume)
        {
            EnableWorldFog();
        }
        else
        {
            m_ClearFlags |= VIEW_CLEAR_COLOR;

            SetFogVolumeState(m_fogInfo, false);

            pRenderContext.GetFrom(materials);

            unsigned char ucFogColor[3];
            pRenderContext->GetFogColor(ucFogColor);
            pRenderContext->ClearColor4ub(ucFogColor[0], ucFogColor[1], ucFogColor[2], 255);
        }

        pRenderContext.SafeRelease();

        DrawExecute(0, CurrentViewID(), 0);

        pRenderContext.GetFrom(materials);
        pRenderContext->ClearColor4ub(0, 0, 0, 255);

        m_pMainView->DisableFog();
    };

    void CallbackInitRenderList(int viewId) override
    {
        BaseClass::CallbackInitRenderList(viewId);

        if (settings.bDrawPlayers && settings.bDrawStaticProps && settings.bDrawTranslucents && settings.bDrawWeapons &&
            settings.bDrawMisc)
            return;

        for (int i = 0; i < RENDER_GROUP_COUNT; i++)
        {
            const bool bStaticProp = i == 0 || i == 2 || i == 4 || i == 6;

            for (int e = 0; e < m_pRenderablesList->m_RenderGroupCounts[i]; e++)
            {
                CClientRenderablesList::CEntry *pEntry = m_pRenderablesList->m_RenderGroups[i] + e;

                if (!pEntry || !pEntry->m_pRenderable)
                    continue;

                bool bRemove = false;
                if (bStaticProp)
                    bRemove = !settings.bDrawStaticProps;
                else
                {
                    IClientUnknown *pUnknown = pEntry->m_pRenderable->GetIClientUnknown();

                    if (!pUnknown || !pUnknown->GetBaseEntity())
                        continue;

                    C_BaseEntity *pEntity = pUnknown->GetBaseEntity();

                    if (pEntity->IsPlayer())
                        bRemove = !settings.bDrawPlayers;
                    else if (dynamic_cast<CBaseCombatWeapon *>(pEntity) != nullptr)
                        bRemove = !settings.bDrawWeapons;
                    else if (pEntry->m_pRenderable->IsTransparent())
                        bRemove = !settings.bDrawTranslucents;
                    else
                        bRemove = !settings.bDrawMisc;
                }

                if (bRemove)
                {
                    pEntry->m_pRenderable = nullptr;
                    pEntry->m_RenderHandle = NULL;
                }
            }

            int eLast = -1;
            for (int e = 0; e < m_pRenderablesList->m_RenderGroupCounts[i]; e++)
            {
                CClientRenderablesList::CEntry *pEntry = m_pRenderablesList->m_RenderGroups[i] + e;

                if (!pEntry || !pEntry->m_pRenderable || !pEntry->m_RenderHandle)
                {
                    for (int e2 = e + 1; e2 < m_pRenderablesList->m_RenderGroupCounts[i]; e2++)
                    {
                        CClientRenderablesList::CEntry *pEntry2 = m_pRenderablesList->m_RenderGroups[i] + e2;
                        if (pEntry2 && pEntry2->m_pRenderable && pEntry2->m_RenderHandle)
                        {
                            CClientRenderablesList::CEntry tmp = *pEntry;
                            *pEntry = *pEntry2;
                            *pEntry2 = tmp;
                            break;
                        }
                    }
                }

                if (pEntry && pEntry->m_pRenderable && pEntry->m_RenderHandle)
                    eLast = e;
            }

            m_pRenderablesList->m_RenderGroupCounts[i] = eLast + 1;
        }
    };

    bool ShouldDrawParticles() override { return settings.bDrawParticles; };

    bool ShouldDrawRopes() override { return settings.bDrawRopes; };

    bool ShouldDrawWorld() override { return settings.bDrawWorld; };

    bool ShouldDrawTranslucents() override { return settings.bDrawTranslucents; };

    bool ShouldDrawTranslucentWorld() override { return settings.bDrawWorld && settings.bDrawTranslucents; };

  private:
    VisibleFogVolumeInfo_t m_fogInfo;
};

static bool UpdateRefractIfNeededByList(CUtlVector<IClientRenderable *> &list)
{
    int nCount = list.Count();
    for (int i = 0; i < nCount; ++i)
    {
        IClientUnknown *pUnk = list[i]->GetIClientUnknown();
        Assert(pUnk);

        IClientRenderable *pRenderable = pUnk->GetClientRenderable();
        Assert(pRenderable);

        if (pRenderable->UsesPowerOfTwoFrameBufferTexture())
        {
            UpdateRefractTexture();
            return true;
        }
    }

    return false;
}
static void DrawRenderablesInList(CUtlVector<IClientRenderable *> &list, int flags = 0)
{
    CViewRender *pCView = assert_cast<CViewRender *>(view);
    Assert(pCView->GetCurrentlyDrawingEntity() == NULL);

    int nCount = list.Count();
    for (int i = 0; i < nCount; ++i)
    {
        IClientUnknown *pUnk = list[i]->GetIClientUnknown();
        Assert(pUnk);

        IClientRenderable *pRenderable = pUnk->GetClientRenderable();
        Assert(pRenderable);

        // Non-view models wanting to render in view model list...
        if (pRenderable->ShouldDraw())
        {
            pCView->SetCurrentlyDrawingEntity(pUnk->GetBaseEntity());
            pRenderable->DrawModel(STUDIO_RENDER | flags);
        }
    }
    pCView->SetCurrentlyDrawingEntity(nullptr);
}

int &ShaderEditorHandler::GetViewIdForModify() const
{
    Assert(m_piCurrentViewId != NULL);

    return *m_piCurrentViewId;
}

const VisibleFogVolumeInfo_t &ShaderEditorHandler::GetFogVolumeInfo() const { return m_tFogVolumeInfo; }
const WaterRenderInfo_t &ShaderEditorHandler::GetWaterRenderInfo() const { return m_tWaterRenderInfo; }

pFnVrCallback_Declare(VrCallback_General)
{
    CViewRender *pCView = assert_cast<CViewRender *>(view);
    Assert(pCView->GetViewSetup() != NULL);

    const CViewSetup *setup = pCView->GetViewSetup();

    CSimpleVCallbackView::EditorViewSettings settings;

    settings.bDrawPlayers = pbOptions[0];
    settings.bDrawWeapons = pbOptions[1];
    settings.bDrawStaticProps = pbOptions[2];
    settings.bDrawMisc = pbOptions[3];
    settings.bDrawTranslucents = pbOptions[4];
    settings.bDrawWater = pbOptions[5];
    settings.bDrawWorld = pbOptions[6];
    settings.bDrawParticles = pbOptions[7];
    settings.bDrawRopes = pbOptions[8];
    settings.bDrawSkybox = pbOptions[9];
    settings.bClipSkybox = pbOptions[10];
    settings.bClearColor = pbOptions[11];
    settings.bClearDepth = pbOptions[12];
    settings.bClearStencil = pbOptions[13];
    settings.bClearObeyStencil = pbOptions[14];
    settings.bFogOverride = pbOptions[15];
    settings.bFogEnabled = pbOptions[16];

    settings.iClearColorR = piOptions[0];
    settings.iClearColorG = piOptions[1];
    settings.iClearColorB = piOptions[2];
    settings.iClearColorA = piOptions[3];
    settings.iFogColorR = piOptions[4];
    settings.iFogColorG = piOptions[5];
    settings.iFogColorB = piOptions[6];

    settings.flFogStart = pflOptions[0];
    settings.flFogEnd = pflOptions[1];
    settings.flFogDensity = pflOptions[2];

    if (settings.flFogEnd < 0)
        settings.flFogEnd = setup->zFar;

    CRefPtr<CSimpleVCallbackView> pGeneralCallbackView = new CSimpleVCallbackView(pCView);
    pGeneralCallbackView->Setup(*setup, settings, g_ShaderEditorSystem->GetFogVolumeInfo(),
                                g_ShaderEditorSystem->GetWaterRenderInfo());
    pCView->AddViewToScene(pGeneralCallbackView);
}

pFnVrCallback_Declare(VrCallback_ViewModel)
{
    CViewRender *pCView = assert_cast<CViewRender *>(view);
    Assert(pCView->GetViewSetup() != NULL);

    CMatRenderContextPtr pRenderContext(materials);

    static ConVarRef drawVM("r_drawviewmodel");

    const bool bHideVM = pbOptions[0];
    const bool bFogOverride = pbOptions[5];
    const int iClearFlags = (pbOptions[1] ? VIEW_CLEAR_COLOR : 0) | (pbOptions[2] ? VIEW_CLEAR_DEPTH : 0) |
                            (pbOptions[3] ? VIEW_CLEAR_STENCIL : 0) | (pbOptions[4] ? VIEW_CLEAR_OBEY_STENCIL : 0);

    drawVM.SetValue(!bHideVM);

    if (bFogOverride)
    {
        if (!pbOptions[6])
            pCView->DisableFog();
        else
        {
            pRenderContext->FogMode(MATERIAL_FOG_LINEAR);
            pRenderContext->FogColor3ub(static_cast<unsigned char>(piOptions[4]),
                                        static_cast<unsigned char>(piOptions[5]),
                                        static_cast<unsigned char>(piOptions[6]));
            pRenderContext->FogStart(pflOptions[0]);
            pRenderContext->FogEnd(pflOptions[1]);
            pRenderContext->FogMaxDensity(pflOptions[2]);
        }
    }

    int bbx, bby;
    materials->GetBackBufferDimensions(bbx, bby);

    // Restore the matrices
    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PushMatrix();

    ITexture *pTex = pRenderContext->GetRenderTarget();
    const CViewSetup &view = *pCView->GetViewSetup();
    CViewSetup viewModelSetup(view);
    viewModelSetup.zNear = view.zNearViewmodel;
    viewModelSetup.zFar = view.zFarViewmodel;
    viewModelSetup.fov = view.fovViewmodel;
    viewModelSetup.m_flAspectRatio = engine->GetScreenAspectRatio();
    viewModelSetup.width = pTex ? pTex->GetActualWidth() : bbx;
    viewModelSetup.height = pTex ? pTex->GetActualHeight() : bby;

    if (iClearFlags & VIEW_CLEAR_COLOR)
    {
        pRenderContext->ClearColor4ub(
            static_cast<unsigned char>(piOptions[0]), static_cast<unsigned char>(piOptions[1]),
            static_cast<unsigned char>(piOptions[2]), static_cast<unsigned char>(piOptions[3]));
    }

    render->Push3DView(viewModelSetup, iClearFlags, pTex, pCView->GetFrustum());
    const bool bUseDepthHack = true;

    float depthmin = 0.0f;
    float depthmax = 1.0f;

    // HACK HACK:  Munge the depth range to prevent view model from poking into walls, etc.
    // Force clipped down range
    if (bUseDepthHack)
        pRenderContext->DepthRange(0.0f, 0.1f);

    CUtlVector<IClientRenderable *> opaqueViewModelList(32);
    CUtlVector<IClientRenderable *> translucentViewModelList(32);
    ClientLeafSystem()->CollateViewModelRenderables(opaqueViewModelList, translucentViewModelList);

    const bool bUpdateRefractForOpaque = UpdateRefractIfNeededByList(opaqueViewModelList);
    DrawRenderablesInList(opaqueViewModelList);

    if (!bUpdateRefractForOpaque)
        UpdateRefractIfNeededByList(translucentViewModelList);

    DrawRenderablesInList(translucentViewModelList, STUDIO_TRANSPARENCY);

    // Reset the depth range to the original values
    if (bUseDepthHack)
        pRenderContext->DepthRange(depthmin, depthmax);

    render->PopView(pCView->GetFrustum());

    // Restore the matrices
    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PopMatrix();

    if (bFogOverride)
        pCView->DisableFog();
}

void ShaderEditorHandler::RegisterViewRenderCallbacks() const
{
    if (!IsReady())
        return;

    const char *boolNames_generalVrc[] = {
        "Draw players", "Draw weapons",   "Draw static props",  "Draw misc",        "Draw translucents", "Draw water",
        "Draw world",   "Draw particles", "Draw ropes",         "Draw skybox (2D)", "Clip skybox",       "Clear color",
        "Clear depth",  "Clear stencil",  "Clear obey stencil", "Fog override",     "Fog force enabled",
    };
    const bool boolDefaults_generalVrc[] = {
        true, true, true, true, true, true, true, true, true, true, false, true, true, false, false, false, true,
    };
    const char *intNames_generalVrc[] = {
        "Clear color R (0-255)", "Clear color G (0-255)", "Clear color B (0-255)", "Clear color A (0-255)",
        "Fog color R (0-255)",   "Fog color G (0-255)",   "Fog color B (0-255)",
    };

    const char *floatNames_generalVrc[] = {
        "Fog start (units)", "Fog end (units)", "Fog density (0-1)",
    };
    const float floatDefaults_generalVrc[] = {
        0, 2000, 1,
    };

    const char *boolNames_vmVrc[] = {
        "Hide default viewmodel", "Clear color",  "Clear depth",       "Clear stencil",
        "Clear obey stencil",     "Fog override", "Fog force enabled",
    };
    const bool boolDefaults_vmVrc[] = {
        false, true, true, false, false, false, true,
    };

    Assert(ARRAYSIZE(boolNames_generalVrc) == ARRAYSIZE(boolDefaults_generalVrc));
    Assert(ARRAYSIZE(floatNames_generalVrc) == ARRAYSIZE(floatDefaults_generalVrc));
    Assert(ARRAYSIZE(boolNames_vmVrc) == ARRAYSIZE(boolDefaults_vmVrc));

    shaderEdit->RegisterViewRenderCallback(
        "General view", VrCallback_General, boolNames_generalVrc, boolDefaults_generalVrc,
        ARRAYSIZE(boolNames_generalVrc), intNames_generalVrc, nullptr, ARRAYSIZE(intNames_generalVrc),
        floatNames_generalVrc, floatDefaults_generalVrc, ARRAYSIZE(floatNames_generalVrc));

    shaderEdit->RegisterViewRenderCallback("Viewmodel view", VrCallback_ViewModel, boolNames_vmVrc, boolDefaults_vmVrc,
                                           ARRAYSIZE(boolNames_vmVrc), intNames_generalVrc, nullptr,
                                           ARRAYSIZE(intNames_generalVrc), floatNames_generalVrc,
                                           floatDefaults_generalVrc, ARRAYSIZE(floatNames_generalVrc));

    shaderEdit->LockViewRenderCallbacks();
}