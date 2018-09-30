#include "cbase.h"
#include "c_mom_triggers.h"
#include "model_types.h"
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

inline int C_BaseMomentumTrigger::GetSpawnFlags(void) const { return m_iSpawnFlags; }
inline void C_BaseMomentumTrigger::AddSpawnFlags(int nFlags) { m_iSpawnFlags |= nFlags; }
inline void C_BaseMomentumTrigger::RemoveSpawnFlags(int nFlags) { m_iSpawnFlags &= ~nFlags; }
inline void C_BaseMomentumTrigger::ClearSpawnFlags(void) { m_iSpawnFlags = 0; }
inline bool C_BaseMomentumTrigger::HasSpawnFlags(int nFlags) const { return (m_iSpawnFlags & nFlags) != 0; }

void TriggerProxy_Model(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	C_BaseMomentumTrigger *entity = (C_BaseMomentumTrigger *)pStruct;
	entity->SetModelName(pData->m_Value.m_pString);

	if (entity->SetModel(pData->m_Value.m_pString))
	{
		entity->SetSolid(SOLID_BSP);
		entity->AddSolidFlags(FSOLID_TRIGGER);
		entity->SetMoveType(MOVETYPE_NONE);

		Q_strncpy((char *)entity->m_iszModel.Get(), pData->m_Value.m_pString, MAX_TRIGGER_NAME);
		entity->PhysicsTouchTriggers();
	}
}

IMPLEMENT_CLIENTCLASS_DT(C_BaseMomentumTrigger, DT_BaseTrigger, CBaseTrigger)
	RecvPropString(RECVINFO(m_iszModel), NULL, TriggerProxy_Model),
	RecvPropString(RECVINFO(m_iszTarget)),
	RecvPropInt(RECVINFO(m_iSpawnFlags)),
END_RECV_TABLE();

bool C_BaseMomentumTrigger::PointIsWithin(const Vector &vecPoint)
{
    Ray_t ray;
    trace_t tr;
    ICollideable *pCollide = CollisionProp();
    ray.Init(vecPoint, vecPoint);
    enginetrace->ClipRayToCollideable(ray, MASK_ALL, pCollide, &tr);
    return (tr.startsolid);
}

bool C_BaseMomentumTrigger::PassesTriggerFilters(CBaseEntity *pOther)
{
    // First test spawn flag filters
    if (HasSpawnFlags(SF_TRIGGER_ALLOW_ALL) ||
        (HasSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS) && (pOther->GetFlags() & FL_CLIENT)) ||
        (HasSpawnFlags(SF_TRIGGER_ALLOW_NPCS) && (pOther->GetFlags() & FL_NPC)) ||
        (HasSpawnFlags(SF_TRIGGER_ALLOW_PUSHABLES) && FClassnameIs(pOther, "func_pushable")) ||
        (HasSpawnFlags(SF_TRIGGER_ALLOW_PHYSICS) && pOther->GetMoveType() == MOVETYPE_VPHYSICS))
    {
        if (pOther->GetFlags() & FL_NPC)
        {
            // TODO Make npc stuff
            /*CAI_BaseNPC *pNPC = pOther->MyNPCPointer();

            if (HasSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS))
            {
                if (!pNPC || !pNPC->IsPlayerAlly())
                {
                    return false;
                }
            }

            if (HasSpawnFlags(SF_TRIGGER_ONLY_NPCS_IN_VEHICLES))
            {
                if (!pNPC || !pNPC->IsInAVehicle())
                    return false;
            }*/
            Msg("[Client] PassesTriggerFilters is NPC");
            return false;
        }

        bool bOtherIsPlayer = pOther->IsPlayer();

        if (bOtherIsPlayer)
        {
            CBasePlayer *pPlayer = (CBasePlayer *)pOther;
            if (!pPlayer->IsAlive())
                return false;

            // TODO Make vehicle stuff
            if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES))
            {
                if (!pPlayer->IsInAVehicle())
                {
                    Msg("[Client] PassesTriggerFilters is not inside vehicle");
                    return false;
                }

                // Make sure we're also not exiting the vehicle at the moment
                /*IServerVehicle *pVehicleServer = pPlayer->GetVehicle();
                if (pVehicleServer == NULL)
                    return false;

                if (pVehicleServer->IsPassengerExiting())
                    return false;*/
            }

            if (HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES))
            {
                if (pPlayer->IsInAVehicle())
                {
                    Msg("[Client] PassesTriggerFilters is inside vehicle");
                    return false;
                }
            }

            // TODO Make bots stuff
            /*if (HasSpawnFlags(SF_TRIGGER_DISALLOW_BOTS))
            {
                if (pPlayer->IsFakeClient())
                    return false;
            }*/
        }

        return true;

        // TODO Make filter stuff
        // CBaseFilter *pFilter = m_hFilter.Get();
        // return (!pFilter) ? true : pFilter->PassesFilter(this, pOther);
    }
    Msg("[Client] Wat: %08X, %08X, %08X\n", (pOther->GetFlags() & FL_CLIENT) == FL_CLIENT,
        HasSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS), m_iSpawnFlags);
    return false;
}

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

LINK_ENTITY_TO_CLASS(trigger_teleport, C_TriggerTeleport);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTeleport, DT_TriggerTeleport, CTriggerTeleport)
END_RECV_TABLE();

CBaseEntity *FindEntityByClassAndName(CBaseEntity *pEnt, const char *szName)
{
    CBaseEntity *pNext = cl_entitylist->NextBaseEntity(pEnt);

    while (pNext != nullptr)
    {
        if (Q_strcmp(pNext->GetName(), szName) == 0)
        {
            return pNext;
        }

        pNext = cl_entitylist->NextBaseEntity(pNext);
    }
    return nullptr;
}

void C_TriggerTeleport::StartTouch(CBaseEntity *pOther)
{
	CBaseEntity *pentTarget = NULL;

	if (!PassesTriggerFilters(pOther))
	{
		return;
	}

	// The activator and caller are the same
	pentTarget = FindEntityByClassAndName(pentTarget, m_iszTarget.Get());

	if (!pentTarget)
	{
		Msg("[Client] Could not find target!\n");
		return;
	}

	pOther->SetGroundEntity(NULL);

	QAngle tmp_angle = pentTarget->GetAbsAngles();
	Vector tmp = pentTarget->GetAbsOrigin();

	pOther->m_angNetworkAngles = tmp_angle;
	pOther->SetLocalAngles(tmp_angle);

	pOther->m_vecNetworkOrigin = tmp;
	pOther->SetLocalOrigin(tmp);
	pOther->SetAbsOrigin(tmp);

	C_BasePlayer* player = C_BasePlayer::GetLocalPlayer();
	if (player == pOther)
	{
		// We need to do it this way to set viewangles at the same frame as the new orign is viewed to the screen
		if (prediction->GetIsFirstTimePredicted())
		{
			player->m_bFixViewAngle = true;
			player->m_vecFixedViewAngles = tmp_angle;
		}

		prediction->SetLocalViewAngles(tmp_angle);
		prediction->SetViewOrigin(tmp);
	}
}

LINK_ENTITY_TO_CLASS(info_teleport_destination, C_PointEntity);

IMPLEMENT_CLIENTCLASS_DT(C_PointEntity, DT_PointEntity, CPointEntity)
	RecvPropString(RECVINFO(m_iszName)),
END_RECV_TABLE();

void C_PointEntity::Spawn()
{
    SetSolid(SOLID_NONE);
}