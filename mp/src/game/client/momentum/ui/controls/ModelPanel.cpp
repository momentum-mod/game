#include "cbase.h"
#include "ModelPanel.h"

#include "model_types.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "view_shared.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define DEFAULT_FOV 70

IMPLEMENT_CLIENTCLASS_DT(C_ModelPanelModel, DT_ModelPanelModel, CModelPanelModel)
END_RECV_TABLE();

CRenderPanel::CRenderPanel(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
{
    render_ang.Init();
    render_pos.Init();
    render_offset.Init();
    render_offset_modelBase.Init();
    m_pModelInstance = nullptr;
    m_bSizeToParent = false;
    m_bShouldAutoRotateModel = false;
    m_angAutoRotation = QAngle(0, 10, 0);

    SetCameraDefaults(70, 45.0f, 180.0f, 100.0f);
    SetDefaultLightAngle(QAngle(45, -135, 0));

    ResetView();

    m_iDragMode = RDRAG_NONE;
    m_iCachedMpos_x = 0;
    m_iCachedMpos_y = 0;
    m_nAllowedDragModes = RDRAG_ALL;
    m_flLastDragTime = 0.0f;
    m_nAllowedRotateModes = ROTATE_ALL;

    m_nFOV = DEFAULT_FOV;

    __view.Identity();
    __proj.Identity();
    __ViewProj.Identity();
    __ViewProjNDC.Identity();

    m_hDefaultCubemap.Init(materials->FindTexture("cubemaps/cubemap_menu_model_bg.hdr", TEXTURE_GROUP_CUBE_MAP));

    ListenForGameEvent("invalid_mdl_cache");
    SetPaintBorderEnabled(true);
}

CRenderPanel::~CRenderPanel()
{
    DestroyModel();
}

void CRenderPanel::UpdateRenderPosition()
{
    if (render_offset_modelBase.IsZero())
        GetModelCenter(render_offset_modelBase);

    QAngle out(m_flPitch, m_flYaw, 0);
    Vector fwd;
    AngleVectors(out, &fwd);

    render_pos = vec3_origin - fwd * m_flDist;
    VectorAngles(fwd, render_ang);

    render_pos += render_offset + render_offset_modelBase;
}

void CRenderPanel::OnThink()
{
    if (m_bSizeToParent)
        SizePanelToParent();

    if (m_pModelInstance && m_iDragMode == RDRAG_NONE)
    {
        m_pModelInstance->SimulateAngles(gpGlobals->frametime);
    }

    if (m_iDragMode == RDRAG_NONE)
        return;

    int mdelta_x, mdelta_y;
    input()->GetCursorPosition(mdelta_x, mdelta_y);
    mdelta_x -= m_iCachedMpos_x;
    mdelta_y -= m_iCachedMpos_y;
    input()->SetCursorPos(m_iCachedMpos_x, m_iCachedMpos_y);
    switch (m_iDragMode)
    {
        case RDRAG_LIGHT:
        {
            VMatrix viewInv;
            MatrixInverseGeneral(__view, viewInv);
            matrix3x4_t rot_x, rot_y, rot_comb;
            Vector fwd, right, up;

            viewInv.GetBasisVectors(right, up, fwd);

            MatrixBuildRotationAboutAxis(right, mdelta_y, rot_x);
            MatrixBuildRotationAboutAxis(up, mdelta_x, rot_y);
            ConcatTransforms(rot_x, rot_y, rot_comb);

            Vector tmp, tmp2;
            AngleVectors(lightAng, &tmp);
            VectorRotate(tmp, rot_comb, tmp2);
            VectorAngles(tmp2, lightAng);
        }
        break;
        case RDRAG_ROTATE:
        {
            if (m_nAllowedRotateModes & ROTATE_PITCH)
            {
                m_flPitch -= mdelta_y * 0.5f;
                m_flPitch = clamp(m_flPitch, -89, 89);
            }

            if (m_nAllowedRotateModes & ROTATE_YAW)
            {
                m_flYaw += mdelta_x * 0.5f;

                if (m_flYaw > 180.0f)
                    m_flYaw -= 360.0f;
                if (m_flYaw < -180.0f)
                    m_flYaw += 360.0f;
            }
        }
        break;
        case RDRAG_POS:
        {
#define RENDER_DRAGPOS_MOVESCALE 0.2f
            Vector viewRight, viewUp;
            AngleVectors(render_ang, NULL, &viewRight, &viewUp);
            render_offset += mdelta_x * -viewRight * RENDER_DRAGPOS_MOVESCALE + mdelta_y * viewUp * RENDER_DRAGPOS_MOVESCALE;
        }
        break;
        default:
            break;
    }
}
void CRenderPanel::ResetView()
{
    m_flDist = m_flDefaultDist;
    m_flPitch = m_flDefaultPitch;
    m_flYaw = m_flDefaultYaw;
    lightAng = m_angLightDefault;
    m_nFOV = m_nDefaultFOV;

    GetModelCenter(render_offset_modelBase);

    render_offset.Init();

    UpdateRenderPosition();
    
    if (m_bSizeToParent)
        SizePanelToParent();
}

void CRenderPanel::SizePanelToParent()
{
    if (!m_bSizeToParent)
        return;

    int iInset_base = 5;
    int iInset_Top = 25;

    int parentSx, parentSy;
    if (GetParent())
    {
        GetParent()->GetSize(parentSx, parentSy);
        parentSx -= iInset_base * 2;
        parentSy -= iInset_base + iInset_Top;

        SetBounds(iInset_base, iInset_Top, parentSx, parentSy);
    }
}

bool CRenderPanel::IsModelReady()
{
    if (!m_pModelInstance)
        return false;

    MDLCACHE_CRITICAL_SECTION();

    return !modelinfo->IsDynamicModelLoading(m_pModelInstance->GetModelIndex());
}

void CRenderPanel::ReloadModel()
{
    if (!m_pModelInstance)
        return;

    char szModelPathCopy[MAX_PATH];
    Q_strncpy(szModelPathCopy, m_szModelPath, MAX_PATH);
    LoadModel(szModelPathCopy);
}

void CRenderPanel::ResetModel()
{
    if (!IsModelReady())
        return;

    m_pModelInstance->m_flAnimTime = gpGlobals->curtime;
    m_pModelInstance->m_flOldAnimTime = gpGlobals->curtime;
}

void CRenderPanel::GetModelCenter(Vector &vecInto)
{
    if (!IsModelReady())
        return;

    vecInto.Init();

    MDLCACHE_CRITICAL_SECTION();
    Vector mins, maxs;
    modelinfo->GetModelRenderBounds(m_pModelInstance->GetModel(), mins, maxs);
    VectorLerp(mins, maxs, 0.5f, vecInto);
}

void CRenderPanel::OnMouseWheeled(int delta)
{
    float amt = RemapVal(m_flDist, 0, 100, 2, 25);
    m_flDist -= delta * amt;
    m_flDist = clamp(m_flDist, 5, 16000);
}

void CRenderPanel::DestroyModel()
{
    if (m_pModelInstance)
    {
        modelinfo->ReleaseDynamicModel(m_pModelInstance->GetModelIndex());
        m_pModelInstance->Remove();
    }

    m_pModelInstance = nullptr;
    m_szModelPath[0] = '\0';
}

void CRenderPanel::SetCameraDefaults(int nFOV, float fPitch, float fYaw, float fDist)
{
    m_nDefaultFOV = nFOV;
    m_flDefaultPitch = fPitch;
    m_flDefaultYaw = fYaw;
    m_flDefaultDist = fDist;
}

bool CRenderPanel::LoadModel(const char *localPath)
{
    MDLCACHE_CRITICAL_SECTION();

    DestroyModel();

    const auto iMdlIndex = modelinfo->RegisterDynamicModel(localPath, true);
    if (iMdlIndex == -1)
        return false;

    Q_strcpy(m_szModelPath, localPath);

    auto pEnt = new C_ModelPanelModel;
    if (!pEnt->InitializeAsClientEntity(nullptr, RENDER_GROUP_OPAQUE_ENTITY))
    {
        pEnt->Remove();
        return false;
    }

    pEnt->RemoveFromClientSideAnimationList();
    pEnt->DontRecordInTools();
    pEnt->SetModelByIndex(iMdlIndex);
    pEnt->Spawn();

    pEnt->SetAbsAngles(vec3_angle);
    pEnt->SetAbsOrigin(vec3_origin);

    pEnt->AddEffects(EF_NODRAW | EF_NOINTERP);
    
    pEnt->m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;

    // leave it alone.
    pEnt->RemoveFromLeafSystem();
    pEnt->CollisionProp()->DestroyPartitionHandle();
    // Needs to be removed from the client entity list so that map start/end does not delete it ...
    cl_entitylist->RemoveEntity(pEnt->GetRefEHandle());
    // ... but this causes an unintended crash. When the player changes model detail, the game will crash
    // because this model does not invalidate its MdlCache (the client DLL engine interface calls it
    // for every entity it keeps track of in the entity list! *rolleyes*)
    // So we listen for the "invalid_mdl_cache" event (see constructor) to be able to manually call it,
    // while manually saving our entity from the perils of start/end map.

    if (m_bShouldAutoRotateModel)
    {
        pEnt->SetLocalAngularVelocity(m_angAutoRotation);
    }

    m_pModelInstance = pEnt;
    ResetView();

    return true;
}

void CRenderPanel::OnMousePressed(MouseCode code)
{
    BaseClass::OnMousePressed(code);

    if (m_iDragMode != RDRAG_NONE)
        return;

    if (code == MOUSE_LEFT && (m_nAllowedDragModes & RDRAG_ROTATE))
    {
        m_iDragMode = RDRAG_ROTATE;
    }
    else if (code == MOUSE_RIGHT && (m_nAllowedDragModes & RDRAG_POS))
    {
        m_iDragMode = RDRAG_POS; // Note: These were swapped
    }
    else if (code == MOUSE_MIDDLE && (m_nAllowedDragModes & RDRAG_LIGHT))
    {
        m_iDragMode = RDRAG_LIGHT;
    }

    if (m_iDragMode != RDRAG_NONE)
    {
        m_flLastDragTime = engine->Time();

        input()->GetCursorPosition(m_iCachedMpos_x, m_iCachedMpos_y);
        input()->SetCursorOveride(dc_none);
        input()->SetMouseCapture(GetVPanel());
    }
}

void CRenderPanel::OnMouseReleased(MouseCode code)
{
    BaseClass::OnMouseReleased(code);

    if (m_iDragMode == RDRAG_NONE)
        return;

    if (m_iDragMode != 1 << (code - MOUSE_FIRST))
        return;

    m_flLastDragTime = engine->Time();

    input()->SetCursorOveride(NULL);
    input()->SetMouseCapture(NULL);
    m_iDragMode = RDRAG_NONE;
}

void CRenderPanel::OnCursorMoved(int x, int y) {}

void CRenderPanel::SetupView(CViewSetup &setup)
{
    UpdateRenderPosition();

    int x = 0, y = 0, w, t;
    GetSize(w, t);
    LocalToScreen(x, y);
    if (x < 0)
        w += x;
    if (y < 0)
        t += y;
    x = max(0, x);
    y = max(0, y);

    setup.angles = render_ang;
    setup.origin = render_pos;

    setup.fov = m_nFOV;
    setup.m_bOrtho = false;

    setup.x = x;
    setup.y = y;

    setup.width = max(1, w);
    setup.height = max(1, t);

    setup.zNear = 1;
    setup.zFar = 10000;
}

void CRenderPanel::Paint()
{
    BaseClass::Paint();

    MDLCACHE_CRITICAL_SECTION();

    modelrender->SuppressEngineLighting(true);

    CViewSetup setup;
    SetupView(setup);

    render->Push3DView(setup, 0, nullptr, frustum);
    render->GetMatricesForView(setup, &__view, &__proj, &__ViewProj, &__ViewProjNDC);

    CMatRenderContextPtr pRenderContext(materials);
    pRenderContext->ClearColor4ub(0, 0, 0, 0);
    pRenderContext->ClearBuffers(false, true);

    pRenderContext->SetLightingOrigin(vec3_origin);
    g_pStudioRender->SetLocalLights(0, nullptr);

    LightDesc_t inf;
    Vector lightDir;
    AngleVectors(lightAng, &lightDir);

    inf.InitDirectional(lightDir, Vector(1, 1, 1));
    g_pStudioRender->SetLocalLights(1, &inf);

#define AMBIENT_ 0.05f // Valve has theirs at 0.4f
    static Vector white[6] = {
        Vector(AMBIENT_, AMBIENT_, AMBIENT_), Vector(AMBIENT_, AMBIENT_, AMBIENT_),
        Vector(AMBIENT_, AMBIENT_, AMBIENT_), Vector(AMBIENT_, AMBIENT_, AMBIENT_),
        Vector(AMBIENT_, AMBIENT_, AMBIENT_), Vector(AMBIENT_, AMBIENT_, AMBIENT_),
    };
    g_pStudioRender->SetAmbientLightColors(white);
    pRenderContext->SetAmbientLight(AMBIENT_, AMBIENT_, AMBIENT_);

    pRenderContext->SetLight(0, inf);

    MaterialFogMode_t oldFog = pRenderContext->GetFogMode();
    pRenderContext->FogMode(MATERIAL_FOG_NONE);

    pRenderContext->BindLocalCubemap(m_hDefaultCubemap);

    DrawModel();

    pRenderContext->FogMode(oldFog);

    render->PopView(frustum);

    modelrender->SuppressEngineLighting(false);

    pRenderContext->Flush();
}

void CRenderPanel::DrawModel()
{
    if (!IsModelReady())
        return;

    SetRenderColors(m_pModelInstance);
    m_pModelInstance->DrawModel(STUDIO_RENDER);
}

void CRenderPanel::SetRenderColors(C_BaseEntity* pEnt)
{
    const auto col = pEnt->GetRenderColor();
    const auto fAlpha = col.a / 255.0f;
    float color[4] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, fAlpha };
    render->SetColorModulation(color);
    render->SetBlend(fAlpha);
}

void CRenderPanel::FireGameEvent(IGameEvent* event)
{
    if (!m_pModelInstance)
        return;

    m_pModelInstance->InvalidateMdlCache();
}

void CRenderPanel::SetShouldAutoRotateModel(bool bRotate)
{
    m_bShouldAutoRotateModel = bRotate;

    if (m_pModelInstance)
    {
        m_pModelInstance->SetLocalAngularVelocity(m_bShouldAutoRotateModel ? m_angAutoRotation : vec3_angle);
    }
}

void CRenderPanel::SetAutoModelRotationSpeed(const QAngle &angle)
{
    m_angAutoRotation = angle;

    if (m_pModelInstance)
    {
        m_pModelInstance->SetLocalAngularVelocity(m_angAutoRotation);
    }
}