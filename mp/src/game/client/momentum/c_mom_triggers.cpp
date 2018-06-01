#include "cbase.h"
#include "c_mom_triggers.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

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

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

#undef CTriggerSlide

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
RecvPropBool(RECVINFO(m_bTouching)) END_RECV_TABLE();

//-----------------------------------------------------------------------------
// Box vertices
//-----------------------------------------------------------------------------
static int s_pBoxFaceIndices[6][4] = {
    {0, 4, 6, 2}, // -x
    {5, 1, 3, 7}, // +x
    {0, 1, 5, 4}, // -y
    {2, 6, 7, 3}, // +y
    {0, 2, 3, 1}, // -z
    {4, 5, 7, 6}  // +z
};

static int s_pBoxFaceIndicesInsideOut[6][4] = {
    {0, 2, 6, 4}, // -x
    {5, 7, 3, 1}, // +x
    {0, 4, 5, 1}, // -y
    {2, 3, 7, 6}, // +y
    {0, 1, 3, 2}, // -z
    {4, 6, 7, 5}  // +z
};

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
static bool s_bMaterialsInitialized = false;
static IMaterial *s_pOutlineColor;

//-----------------------------------------------------------------------------
// Initializes standard materials
//-----------------------------------------------------------------------------
void InitMaterial()
{
    if (s_bMaterialsInitialized)
        return;

    s_bMaterialsInitialized = true;

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
    s_pOutlineColor = g_pMaterialSystem->CreateMaterial("__utilOutlineColor", pVMTKeyValues);
    s_pOutlineColor->IncrementReferenceCount();
}

void FreeMaterial()
{
    if (!s_bMaterialsInitialized)
        return;

    s_bMaterialsInitialized = false;

    s_pOutlineColor->DecrementReferenceCount();
    s_pOutlineColor = NULL;
}

static void GenerateBoxVertices(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs,
                                Vector pVerts[8])
{
    // Build a rotation matrix from orientation
    matrix3x4_t fRotateMatrix;
    AngleMatrix(angles, fRotateMatrix);

    Vector vecPos;
    for (int i = 0; i < 8; ++i)
    {
        vecPos[0] = (i & 0x1) ? vMaxs[0] : vMins[0];
        vecPos[1] = (i & 0x2) ? vMaxs[1] : vMins[1];
        vecPos[2] = (i & 0x4) ? vMaxs[2] : vMins[2];

        VectorRotate(vecPos, fRotateMatrix, pVerts[i]);
        pVerts[i] += vOrigin;
    }
}

//-----------------------------------------------------------------------------
// Renders a outlined box relative to an origin
//-----------------------------------------------------------------------------
void OutlineBox(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, const Color &c)
{
    InitMaterial();

    CMatRenderContextPtr pRenderContext(materials);
    pRenderContext->Bind(s_pOutlineColor);

    Vector p[8];
    GenerateBoxVertices(vOrigin, angles, vMins, vMaxs, p);

    unsigned char chRed = c.r();
    unsigned char chGreen = c.g();
    unsigned char chBlue = c.b();
    unsigned char chAlpha = c.a();

    IMesh *pMesh = pRenderContext->GetDynamicMesh();
    CMeshBuilder meshBuilder;
    meshBuilder.Begin(pMesh, MATERIAL_LINES, 24);

    // Draw the box
    for (int i = 0; i < 6; i++)
    {
        int *pFaceIndex = s_pBoxFaceIndices[i];

        for (int j = 0; j < 4; ++j)
        {
            meshBuilder.Position3fv(p[pFaceIndex[j]].Base());
            meshBuilder.Color4ub(chRed, chGreen, chBlue, chAlpha);
            meshBuilder.AdvanceVertex();

            meshBuilder.Position3fv(p[pFaceIndex[(j == 3) ? 0 : j + 1]].Base());
            meshBuilder.Color4ub(chRed, chGreen, chBlue, chAlpha);
            meshBuilder.AdvanceVertex();
        }
    }

    meshBuilder.End();
    pMesh->Draw();
}

void C_BaseMomentumTrigger::DrawOutlineOBBs(const Color &color)
{
    OutlineBox(GetAbsOrigin(), GetAbsAngles(), GetCollideable()->OBBMins(), GetCollideable()->OBBMaxs(), color);
}

bool C_TriggerTimerStart::ShouldDraw() { return true; }

int C_TriggerTimerStart::DrawModel(int flags)
{
    if (mom_startzone_outline_enable.GetBool())
    {
        Color color_startzone;
        if (g_pMomentumUtil->GetColorFromHex(mom_startzone_color.GetString(), color_startzone))
        {
            DrawOutlineOBBs(color_startzone);

            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

bool C_TriggerTimerStop::ShouldDraw() { return true; }

int C_TriggerTimerStop::DrawModel(int flags)
{
    if (mom_endzone_outline_enable.GetBool())
    {
        Color color_endzone;
        if (g_pMomentumUtil->GetColorFromHex(mom_endzone_color.GetString(), color_endzone))
        {
            DrawOutlineOBBs(color_endzone);

            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

void C_TriggerSlide::PostDataUpdate(DataUpdateType_t updatetype)
{
    BaseClass::PostDataUpdate(updatetype);

    if (m_bTouching)
    {
        g_pMomentumGameMovement->GetSlideTrigger() = reinterpret_cast<C_TriggerSlide *>(this);
    }
    else
    {
        g_pMomentumGameMovement->GetSlideTrigger() = nullptr;
    }
}