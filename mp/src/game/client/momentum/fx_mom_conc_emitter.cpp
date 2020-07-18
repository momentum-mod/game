#include "cbase.h"
#include "beam_flags.h"
#include "c_te_effect_dispatch.h"
#include "clienteffectprecachesystem.h"
#include "fx_mom_conc_emitter.h"
#include "fx_explosion.h"
#include "materialsystem/imaterialvar.h"
#include "particles_simple.h"
#include "tempent.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

#define CONC_EFFECT_MATERIAL "sprites/concrefract"
extern int g_iConcRingTexture;

static MAKE_TOGGLE_CONVAR(mom_conc_effects_enable, "1", FCVAR_ARCHIVE, "Toggle the conc grenade's explosion effects. 0 = OFF, 1 = ON.\n");
ConVar mom_conc_refract("mom_conc_refract", "0", FCVAR_ARCHIVE, "Toggles between conc effects; set to 1 for the refractive sphere or 0 for the flat rings.\n");

#define CONC_FRAMERATE 1.0f
#define CONC_WIDTH 50.0f
#define CONC_WIDTH2 0.0f
#define CONC_WIDTH3 10.0f
#define CONC_SPREAD 0.0f
#define CONC_AMPLITUDE 0.0f
#define CONC_LIFETIME 0.3f
#define CONC_R 255.0f
#define CONC_G 255.0f
#define CONC_B 225.0f
#define CONC_A 178.0f
#define CONC_RADIUS 600.0f
#define CONC_RADIUS2 520.0f

CLIENTEFFECT_REGISTER_BEGIN(PrecacheConcEmitter)
    CLIENTEFFECT_MATERIAL(CONC_EFFECT_MATERIAL)
CLIENTEFFECT_REGISTER_END()

PMaterialHandle CConcEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

CConcEmitter::CConcEmitter(const char *pDebugName) : CSimpleEmitter(pDebugName)
{
    m_pDebugName = pDebugName;
}

CConcEmitter::~CConcEmitter()
{
}

CSmartPtr<CConcEmitter> CConcEmitter::Create(const char *pDebugName)
{
    CConcEmitter *pRet = new CConcEmitter(pDebugName);

    pRet->SetDynamicallyAllocated();

    if (m_hMaterial == INVALID_MATERIAL_HANDLE)
        m_hMaterial = pRet->GetPMaterial(CONC_EFFECT_MATERIAL);

    return pRet;
}

void CConcEmitter::RenderParticles(CParticleRenderIterator *pIterator)
{
    if (!mom_conc_effects_enable.GetBool())
        return;

    const ConcParticle *pParticle = (const ConcParticle *) pIterator->GetFirst();

    float flLife, flDeath;
    bool bFound;

    while (pParticle)
    {
        Vector tPos;

        TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
        float sortKey = (int)tPos.z;

        flLife = pParticle->m_flLifetime - pParticle->m_flOffset;
        flDeath = pParticle->m_flDieTime - pParticle->m_flOffset;

        if (flLife > 0.0f)
        {
            IMaterial *pMat = pIterator->GetParticleDraw()->m_pSubTexture->m_pMaterial;

            if (pMat)
            {
                IMaterialVar *pVar = pMat->FindVar("$refractamount", &bFound, true);

                if (pVar)
                    pVar->SetFloatValue(-pParticle->m_flRefract +
                                        SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0.001f, pParticle->m_flRefract));

                pVar = pMat->FindVar("$bluramount", &bFound, true);

                if (pVar)
                    pVar->SetFloatValue(0.5f);
            }

            float flColor = 1.0f;

            Vector vColor = Vector(flColor, flColor, flColor);

            RenderParticle_ColorSizeAngle(pIterator->GetParticleDraw(), tPos, vColor,
                                          flColor, // Alpha
                                          SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0, 512.0f), // Size
                                          pParticle->m_flRoll);
        }

        pParticle = (const ConcParticle *) pIterator->GetNext(sortKey);
    }
}

ConcParticle *CConcEmitter::AddConcParticle()
{
    ConcParticle *pRet = (ConcParticle *) AddParticle(sizeof(ConcParticle), m_hMaterial, GetSortOrigin());

    if (pRet)
    {
        pRet->m_Pos = GetSortOrigin();
        pRet->m_vecVelocity.Init();
        pRet->m_flRoll = 0;
        pRet->m_flRollDelta = 0;
        pRet->m_flLifetime = 0;
        pRet->m_flDieTime = 0;
        pRet->m_uchColor[0] = pRet->m_uchColor[1] = pRet->m_uchColor[2] = 0;
        pRet->m_uchStartAlpha = pRet->m_uchEndAlpha = 255;
        pRet->m_uchStartSize = 0;
        pRet->m_iFlags = 0;
        pRet->m_flOffset = 0;
    }

    return pRet;
}

class C_ConcEffect : public C_BaseAnimating
{
    typedef C_BaseAnimating BaseClass;

  public:
    static C_ConcEffect *CreateClientsideEffect(const char *pszModelName, Vector vecOrigin);

    bool InitializeEffect(const char *pszModelName, Vector vecOrigin);
    void ClientThink();

  protected:
    IMaterial *m_pMaterial;
    float m_flStart;
};

//-----------------------------------------------------------------------------
// Purpose: Create the conc effect
//-----------------------------------------------------------------------------
C_ConcEffect *C_ConcEffect::CreateClientsideEffect(const char *pszModelName, Vector vecOrigin)
{
    C_ConcEffect *pEffect = new C_ConcEffect;

    if (!pEffect)
        return nullptr;

    if (!pEffect->InitializeEffect(pszModelName, vecOrigin))
        return nullptr;

    pEffect->m_pMaterial = materials->FindMaterial(CONC_EFFECT_MATERIAL, TEXTURE_GROUP_OTHER);
    pEffect->m_flStart = gpGlobals->curtime;

    return pEffect;
}

bool C_ConcEffect::InitializeEffect(const char *pszModelName, Vector vecOrigin)
{
    if (!InitializeAsClientEntity(pszModelName, RENDER_GROUP_OPAQUE_ENTITY))
    {
        Release();
        return false;
    }

    SetAbsOrigin(vecOrigin);

    SetNextClientThink(CLIENT_THINK_ALWAYS);

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Adjust the material proxies for the conc as time goes on
//-----------------------------------------------------------------------------
void C_ConcEffect::ClientThink()
{
    C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();

    // We need to keep the correct part of the shader oriented towards the player
    // The bit we want is on the top, so rotate around x axis by 90
    Vector vecDir = GetAbsOrigin() - pPlayer->EyePosition();

    QAngle angFace;
    VectorAngles(vecDir, angFace);
    angFace.x += 90;

    SetAbsAngles(angFace);

    float flLife = gpGlobals->curtime - m_flStart;
    float flStrength = 0.0f;

    // These are temp until the values are decided
    float flMidTime = 0.25f;
    float flEndTime = 0.25f + 0.35f;

    if (flLife <= /*0.4f*/ flMidTime)
    {
        flStrength = SimpleSplineRemapVal(flLife, 0.0f, /*0.4f*/ flMidTime, 0.0f, 1.0f);
    }
    else if (flLife <= /*1.2f*/ flEndTime)
    {
        flStrength = SimpleSplineRemapVal(flLife, /*0.4f*/ flMidTime, /*1.2f*/ flEndTime, 1.0f, 0.0f);
    }
    // Bit of time to settle before releasing
    else if (flLife > /*1.5f*/ flEndTime + 0.2f)
    {
        Release();
        return;
    }

    flStrength = clamp(flStrength, 0.0f, 1.0f);

    bool bFound;
    IMaterialVar *pVar = m_pMaterial->FindVar("$refractamount", &bFound, true);

    if (pVar)
        pVar->SetFloatValue(flStrength * 0.5f);

    pVar = m_pMaterial->FindVar("$bluramount", &bFound, true);

    if (pVar)
        pVar->SetFloatValue(flStrength * 0.5f);
}

void FF_FX_ConcussionEffect_Callback(const CEffectData &data)
{
#define TE_EXPLFLAG_NODLIGHTS 0x2 // do not render dynamic lights
#define TE_EXPLFLAG_NOSOUND 0x4   // do not play client explosion sound

    // If underwater, just do a bunch of gas
    if (UTIL_PointContents(data.m_vOrigin) & CONTENTS_WATER)
    {
        WaterExplosionEffect().Create(data.m_vOrigin, 180.0f, 10.0f, TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);

        if (mom_conc_refract.GetBool())
            return;
    }

    if (!mom_conc_refract.GetBool())
    {
        CBroadcastRecipientFilter filter;

        te->BeamRingPoint(filter,
                          0,                                // delay
                          data.m_vOrigin,                   // origin
                          1.0f,                             // start radius
                          CONC_RADIUS,                      // end radius
                          g_iConcRingTexture,               // texture index
                          0,                                // halo index
                          0,                                // start frame
                          CONC_FRAMERATE,                   // frame rate
                          CONC_LIFETIME,                    // life
                          CONC_WIDTH,                       // width
                          CONC_SPREAD,                      // spread (10x end width)
                          CONC_AMPLITUDE,                   // amplitude
                          CONC_R,                           // r
                          CONC_G,                           // g
                          CONC_B,                           // b
                          CONC_A,                           // a
                          0,                                // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE); // flags

        te->BeamRingPoint(filter,
                          0,                                            // delay
                          (data.m_vOrigin + Vector(0.0f, 0.0f, 32.0f)), // origin
                          1.0f,                                         // start radius
                          CONC_RADIUS2,                                 // end radius
                          g_iConcRingTexture,                           // texture index
                          0,                                            // halo index
                          0,                                            // start frame
                          CONC_FRAMERATE,                               // frame rate
                          CONC_LIFETIME,                                // life
                          CONC_WIDTH2,                                  // width
                          CONC_SPREAD,                                  // spread (10x end width)
                          CONC_AMPLITUDE,                               // amplitude
                          CONC_R,                                       // r
                          CONC_G,                                       // g
                          CONC_B,                                       // b
                          CONC_A,                                       // a
                          0,                                            // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE);             // flags

        te->BeamRingPoint(filter,
                          0,                                             // delay
                          (data.m_vOrigin + Vector(0.0f, 0.0f, -32.0f)), // origin
                          1.0f,                                          // start radius
                          CONC_RADIUS2,                                  // end radius
                          g_iConcRingTexture,                            // texture index
                          0,                                             // halo index
                          0,                                             // start frame
                          CONC_FRAMERATE,                                // frame rate
                          CONC_LIFETIME,                                 // life
                          CONC_WIDTH2,                                   // width
                          CONC_SPREAD,                                   // spread (10x end width)
                          CONC_AMPLITUDE,                                // amplitude
                          CONC_R,                                        // r
                          CONC_G,                                        // g
                          CONC_B,                                        // b
                          CONC_A,                                        // a
                          0,                                             // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE);              // flags

        // Outer bounding rings on thrown concs
        te->BeamRingPoint(filter,
                          0,                                // delay
                          data.m_vOrigin,                   // origin
                          CONC_RADIUS - 1,                  // start radius
                          CONC_RADIUS,                      // end radius
                          g_iConcRingTexture,               // texture index
                          0,                                // halo index
                          0,                                // start frame
                          CONC_FRAMERATE,                   // frame rate
                          CONC_LIFETIME,                    // life
                          CONC_WIDTH3,                      // width
                          CONC_SPREAD,                      // spread (10x end width)
                          CONC_AMPLITUDE,                   // amplitude
                          CONC_R,                           // r
                          CONC_G,                           // g
                          CONC_B,                           // b
                          CONC_A,                           // a
                          0,                                // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE); // flags

        // GaussExplosion adds some cool white ember explosion to the conc
        te->GaussExplosion(filter, 0, data.m_vOrigin, Vector(0, 0, 1), 1);
    }
    else
    {
        CSmartPtr<CConcEmitter> concEffect = CConcEmitter::Create("ConcussionEffect");

        float offset = 0;

        ConcParticle *c = concEffect->AddConcParticle();

        if (c)
        {
            c->m_flDieTime = 0.5f;
            c->m_Pos = data.m_vOrigin;
            c->m_flRefract = 0.5f;
            c->m_flOffset = offset;

            offset += 0.05f;
        }
    }
}

void FF_FX_ConcussionEffectHandheld_Callback(const CEffectData &data)
{
#define TE_EXPLFLAG_NODLIGHTS 0x2 // do not render dynamic lights
#define TE_EXPLFLAG_NOSOUND 0x4   // do not play client explosion sound

    // If underwater, just do a bunch of gas
    if (UTIL_PointContents(data.m_vOrigin) & CONTENTS_WATER)
    {
        WaterExplosionEffect().Create(data.m_vOrigin, 180.0f, 10.0f, TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
        if (mom_conc_refract.GetBool())
            return;
    }

    if (!mom_conc_refract.GetBool())
    {
        CBroadcastRecipientFilter filter;

        te->BeamRingPoint(filter,
                          0,                                // delay
                          data.m_vOrigin,                   // origin
                          1.0f,                             // start radius
                          CONC_RADIUS,                      // end radius
                          g_iConcRingTexture,               // texture index
                          0,                                // halo index
                          0,                                // start frame
                          CONC_FRAMERATE,                   // frame rate
                          CONC_LIFETIME,                    // life
                          CONC_WIDTH,                       // width
                          CONC_SPREAD,                      // spread (10x end width)
                          CONC_AMPLITUDE,                   // amplitude
                          CONC_R,                           // r
                          CONC_G,                           // g
                          CONC_B,                           // b
                          CONC_A,                           // a
                          0,                                // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE); // flags

        te->BeamRingPoint(filter,
                          0,                                            // delay
                          (data.m_vOrigin + Vector(0.0f, 0.0f, 32.0f)), // origin
                          1.0f,                                         // start radius
                          CONC_RADIUS2,                                 // end radius
                          g_iConcRingTexture,                           // texture index
                          0,                                            // halo index
                          0,                                            // start frame
                          CONC_FRAMERATE,                               // frame rate
                          CONC_LIFETIME,                                // life
                          CONC_WIDTH2,                                  // width
                          CONC_SPREAD,                                  // spread (10x end width)
                          CONC_AMPLITUDE,                               // amplitude
                          CONC_R,                                       // r
                          CONC_G,                                       // g
                          CONC_B,                                       // b
                          CONC_A,                                       // a
                          0,                                            // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE);             // flags

        te->BeamRingPoint(filter,
                          0,                                             // delay
                          data.m_vOrigin + Vector(0.0f, 0.0f, -32.0f), // origin
                          1.0f,                                          // start radius
                          CONC_RADIUS2,                                  // end radius
                          g_iConcRingTexture,                            // texture index
                          0,                                             // halo index
                          0,                                             // start frame
                          CONC_FRAMERATE,                                // frame rate
                          CONC_LIFETIME,                                 // life
                          CONC_WIDTH2,                                   // width
                          CONC_SPREAD,                                   // spread (10x end width)
                          CONC_AMPLITUDE,                                // amplitude
                          CONC_R,                                        // r
                          CONC_G,                                        // g
                          CONC_B,                                        // b
                          CONC_A,                                        // a
                          0,                                             // speed
                          FBEAM_FADEOUT | FBEAM_SINENOISE);        // flags

        te->GaussExplosion(filter, 0, data.m_vOrigin, Vector(0, 0, 1), 1);
    }
    else
    {
        CSmartPtr<CConcEmitter> concEffect = CConcEmitter::Create("ConcussionEffect");

        float offset = 0;

        ConcParticle *c = concEffect->AddConcParticle();

        if (c)
        {
            c->m_flDieTime = 0.5f;
            c->m_Pos = data.m_vOrigin;
            c->m_flRefract = 0.5f;
            c->m_flOffset = offset;

            offset += 0.05f;
        }
    }
}

DECLARE_CLIENT_EFFECT("MOM_ConcussionEffect", FF_FX_ConcussionEffect_Callback);
DECLARE_CLIENT_EFFECT("MOM_ConcussionEffectHandheld", FF_FX_ConcussionEffectHandheld_Callback);
