// CREDITS: Valve.

#include "cbase.h"
#include "filesystem.h"
#include "mom_enginedraw.h"
#include "util/mom_util.h"

// Reference: search for "permanent" string inside IDA.

/*
int __cdecl r_cleardecals_Command(int a1)
{
  char *v1; // eax
  int v2; // eax
  char v3; // cl
  bool v4; // zf
  char v6; // [esp+0h] [ebp-4h]

  v1 = dword_10639BC8;
  if ( dword_10639BC8 )
  {
    v6 = 0;
    if ( *(_DWORD *)a1 == 2 )
    {
      v2 = sub_10249260(*(char **)(a1 + 1036), "permanent");
      v3 = 0;
      v4 = v2 == 0;
      v1 = dword_10639BC8;
      if ( v4 )
        v3 = 1;
      v6 = v3;
    }
    sub_1012F5F0(*((_DWORD *)v1 + 14), v6);
  }
  return sub_1010D510();
}

CON_COMMAND_F( r_cleardecals, "Usage r_cleardecals <permanent>.", FCVAR_CLIENTCMD_CAN_EXECUTE  )
{
    if ( host_state.worldmodel  )
    {
        bool bPermanent = false;
        if ( args.ArgC() == 2 )
        {
            if ( !Q_stricmp( args[1], "permanent" ) )
            {
                bPermanent = true;
            }
        }

        R_DecalTerm( host_state.worldmodel->brush.pShared, bPermanent );
    }

    R_RemoveAllDecalsFromAllModels();
}
*/

// Since engine will probably not change, we can use direct offset and not pattern scanning. (faster)
#if defined(_WIN32)
static uintptr_t OffsetHostStatePtr = 0x639BC8;
#elif defined(OSX)
static uintptr_t OffsetHostStatePtr = 0x7DC118;
#error
#elif defined(LINUX)
static uintptr_t OffsetHostStatePtr = 0xC79570;
#error
#else
#error
#endif

CCommonHostState *host_state =
    reinterpret_cast<CCommonHostState *>(reinterpret_cast<uintptr_t>(Sys_LoadModule("engine")) + OffsetHostStatePtr);

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

void MOM_DrawBrushOutlineModel(CBaseEntity *baseentity, const Color &color)
{
    auto model = baseentity->GetModel();

    if (model == nullptr)
        return;

    Vector vecAbsOrigin = baseentity->GetAbsOrigin();

    CUtlVector<msurface2_t *> surfaceList;
    SurfaceHandle_t surfID = SurfaceHandleFromIndex(model->brush.firstmodelsurface, model->brush.pShared);

    for (int i = 0; i < model->brush.nummodelsurfaces; i++, surfID++)
    {
        surfaceList.AddToTail(surfID);
    }

    InitMaterial();

    int nLineCount = 0;

    for (int i = 0; i < surfaceList.Count(); i++)
    {
        int nCount = MSurf_VertCount(surfaceList[i]);

        if (nCount >= 3)
        {
            nLineCount += nCount;
        }
    }

    if (nLineCount == 0)
        return;

    CMatRenderContextPtr pRenderContext(materials);

    pRenderContext->Bind(s_pOutlineColor);
    IMesh *pMesh = pRenderContext->GetDynamicMesh();
    CMeshBuilder meshBuilder;
    meshBuilder.Begin(pMesh, MATERIAL_LINES, nLineCount);

    for (int i = 0; i < surfaceList.Count(); i++)
    {
        SurfaceHandle_t surfID = surfaceList[i];

        // Compute the centroid of the surface
        int nCount = MSurf_VertCount(surfID);
        if (nCount >= 3)
        {
            int nFirstVertIndex = MSurf_FirstVertIndex(surfID);
            int nVertIndex = host_state->worldbrush->vertindices[nFirstVertIndex + nCount - 1];
            Vector vecPrevPos = host_state->worldbrush->vertexes[nVertIndex].position;
            vecPrevPos += vecAbsOrigin;

            for (int v = 0; v < nCount; ++v)
            {
                // world-space vertex
                nVertIndex = host_state->worldbrush->vertindices[nFirstVertIndex + v];
                Vector vec = host_state->worldbrush->vertexes[nVertIndex].position;
                vec += vecAbsOrigin;

                // output to mesh
                meshBuilder.Position3fv(vecPrevPos.Base());
                meshBuilder.Color4ub(color.r(), color.g(), color.b(), color.a());
                meshBuilder.AdvanceVertex();
                meshBuilder.Position3fv(vec.Base());
                meshBuilder.Color4ub(color.r(), color.g(), color.b(), color.a());
                meshBuilder.AdvanceVertex();

                vecPrevPos = vec;
            }
        }
    }

    meshBuilder.End();
    pMesh->Draw();
}
