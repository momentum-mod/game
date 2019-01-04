#include "cbase.h"
#include "ModelPanel.h"

#include "model_types.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "view_shared.h"

using namespace vgui;

CRenderPanel::CRenderPanel(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
{
    render_ang.Init();
    render_pos.Init();
    render_offset.Init();
    render_offset_modelBase.Init();
    m_pModelInstance = nullptr;
    m_bSizeToParent = true;

    ResetView();

    m_iDragMode = RDRAG_NONE;
    m_iCachedMpos_x = 0;
    m_iCachedMpos_y = 0;

    m_nFOV = 70;

    __view.Identity();
    __proj.Identity();
    __ViewProj.Identity();
    __ViewProjNDC.Identity();

    ListenForGameEvent("invalid_mdl_cache");
}

CRenderPanel::~CRenderPanel()
{
}

void CRenderPanel::UpdateRenderPosition()
{
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

    if (m_iDragMode)
    {
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
            m_flPitch -= mdelta_y * 0.5f;
            m_flYaw += mdelta_x * 0.5f;

            m_flPitch = clamp(m_flPitch, -89, 89);
            if (m_flYaw > 180.0f)
                m_flYaw -= 360.0f;
            if (m_flYaw < -180.0f)
                m_flYaw += 360.0f;
        }
        break;
        case RDRAG_POS:
        {
#define RENDER_DRAGPOS_MOVESCALE 0.2f
            Vector viewRight, viewUp;
            AngleVectors(render_ang, NULL, &viewRight, &viewUp);
            render_offset +=
                mdelta_x * -viewRight * RENDER_DRAGPOS_MOVESCALE + mdelta_y * viewUp * RENDER_DRAGPOS_MOVESCALE;
        }
        break;
        default:
            break;
        }
    }
}
void CRenderPanel::ResetView()
{
    m_flDist = 100;
    m_flPitch = 45;
    m_flYaw = 180;
    lightAng.Init(45, -135, 0);
    m_nFOV = 70;

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
    bool bValid = !!m_pModelInstance->GetModel();

    if (!bValid && Q_strlen(m_szModelPath))
    {
        const model_t *pMdl = modelinfo ? engine->LoadModel(m_szModelPath) : nullptr;
        if (pMdl)
            m_pModelInstance->SetModelPointer(pMdl);
        bValid = !!pMdl;
    }

    if (!bValid)
        DestroyModel();

    return bValid;
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
    vecInto.Init();
    if (IsModelReady())
    {
        MDLCACHE_CRITICAL_SECTION();
        if (m_pModelInstance->GetModel())
        {
            Vector mins, maxs;
            modelinfo->GetModelRenderBounds(m_pModelInstance->GetModel(), mins, maxs);
            VectorLerp(mins, maxs, 0.5f, vecInto);
        }
    }
}

void CRenderPanel::OnMouseWheeled(int delta)
{
    BaseClass::OnMouseWheeled(delta);

    float amt = RemapVal(m_flDist, 0, 100, 2, 25);
    m_flDist -= delta * amt;
    m_flDist = clamp(m_flDist, 5, 16000);
}

void CRenderPanel::DestroyModel()
{
    if (m_pModelInstance)
        m_pModelInstance->Remove();

    m_pModelInstance = nullptr;
    m_szModelPath[0] = '\0';
}

bool CRenderPanel::LoadModel(const char *localPath)
{
    MDLCACHE_CRITICAL_SECTION();

    DestroyModel();

    const model_t *mdl = engine->LoadModel(localPath);
    if (!mdl)
        return false;

    Q_strcpy(m_szModelPath, localPath);

    CModelPanelModel *pEnt = new CModelPanelModel();
    if (!pEnt->InitializeAsClientEntity(nullptr, RENDER_GROUP_OPAQUE_ENTITY))
    {
        pEnt->Remove();
        return false;
    }

    pEnt->DontRecordInTools();
    pEnt->SetModelPointer(mdl);
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

    m_pModelInstance = pEnt;
    ResetView();

    return true;
}

void CRenderPanel::OnMousePressed(MouseCode code)
{
    BaseClass::OnMousePressed(code);
    if (m_iDragMode)
        return;

    if (code == MOUSE_LEFT)
        m_iDragMode = RDRAG_ROTATE;
    else if (code == MOUSE_RIGHT)
        m_iDragMode = RDRAG_POS; // Note: These were swapped
    else
        m_iDragMode = RDRAG_LIGHT;

    input()->GetCursorPosition(m_iCachedMpos_x, m_iCachedMpos_y);
    input()->SetCursorOveride(dc_none);
    input()->SetMouseCapture(GetVPanel());
}
void CRenderPanel::OnMouseReleased(MouseCode code)
{
    BaseClass::OnMouseReleased(code);
    if (!m_iDragMode)
        return;

    input()->SetCursorOveride(NULL);
    input()->SetMouseCapture(NULL);
    m_iDragMode = RDRAG_NONE;
}
void CRenderPanel::OnCursorMoved(int x, int y) {}

void CRenderPanel::SetupView(CViewSetup &setup)
{
    UpdateRenderPosition();

    int x, y, w, t;
    GetSize(w, t);
    x = y = 0;
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

    DrawModel();

    pRenderContext->FogMode(oldFog);

    render->PopView(frustum);

    modelrender->SuppressEngineLighting(false);
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
    color32 col = pEnt->GetRenderColor();
    float color[4] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f };
    float alphaBias = (GetParent() ? GetParent()->GetAlpha() : GetAlpha()) / 255.0f;
    render->SetColorModulation(color);
    render->SetBlend(alphaBias * color[3]);
}

void CRenderPanel::FireGameEvent(IGameEvent* event)
{
    if (m_pModelInstance)
        m_pModelInstance->InvalidateMdlCache();
}
