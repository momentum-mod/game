//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific explosion effects
//
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"
#include "tempent.h"
#include "mom_shareddefs.h"
#include "tier0/vprof.h"

#include "tier0/memdbgon.h"

class C_TETFParticleEffect : public C_BaseTempEntity
{
  public:
    DECLARE_CLASS(C_TETFParticleEffect, C_BaseTempEntity);
    DECLARE_CLIENTCLASS();

    C_TETFParticleEffect();

    ClientEntityHandle_t m_hEntity;

  private:
    virtual void PostDataUpdate(DataUpdateType_t updateType);

    Vector m_vecOrigin;
    Vector m_vecStart;
    QAngle m_vecAngles;

    int m_iParticleSystemIndex;

    int m_iAttachType;
    int m_iAttachmentPointIndex;

    bool m_bResetParticles;
};

C_TETFParticleEffect::C_TETFParticleEffect()
{
    m_vecOrigin.Init();
    m_vecStart.Init();
    m_vecAngles.Init();

    m_iParticleSystemIndex = -1;

    m_hEntity = INVALID_EHANDLE_INDEX;

    m_iAttachType = PATTACH_ABSORIGIN;
    m_iAttachmentPointIndex = 0;

    m_bResetParticles = false;
}

void C_TETFParticleEffect::PostDataUpdate(DataUpdateType_t updateType)
{
    VPROF("C_TETFParticleEffect::PostDataUpdate");

    CEffectData data;

    data.m_nHitBox = m_iParticleSystemIndex;

    data.m_vOrigin = m_vecOrigin;
    data.m_vStart = m_vecStart;
    data.m_vAngles = m_vecAngles;

    if (m_hEntity != INVALID_EHANDLE_INDEX)
    {
        data.m_hEntity = m_hEntity;
        data.m_fFlags |= PARTICLE_DISPATCH_FROM_ENTITY;
    }
    else
    {
        data.m_hEntity = NULL;
    }

    data.m_nDamageType = m_iAttachType;
    data.m_nAttachmentIndex = m_iAttachmentPointIndex;

    if (m_bResetParticles)
    {
        data.m_fFlags |= PARTICLE_DISPATCH_RESET_PARTICLES;
    }

    DispatchEffect("ParticleEffect", data);
}

static void RecvProxy_ParticleSystemEntIndex(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
    int nEntIndex = pData->m_Value.m_Int;
    ((C_TETFParticleEffect *)pStruct)->m_hEntity =
        (nEntIndex < 0) ? INVALID_EHANDLE_INDEX : ClientEntityList().EntIndexToHandle(nEntIndex);
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFParticleEffect, DT_TETFParticleEffect, CTETFParticleEffect)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecStart)),
    RecvPropQAngles(RECVINFO(m_vecAngles)),
    RecvPropInt(RECVINFO(m_iParticleSystemIndex)),
    RecvPropInt("entindex", 0, SIZEOF_IGNORE, 0, RecvProxy_ParticleSystemEntIndex),
    RecvPropInt(RECVINFO(m_iAttachType)),
    RecvPropInt(RECVINFO(m_iAttachmentPointIndex)),
    RecvPropInt(RECVINFO(m_bResetParticles)), 
END_RECV_TABLE()