#include "cbase.h"
#include "c_mom_online_ghost.h"
#include "steam/steam_api.h"
#include "GhostEntityPanel.h"
#include "flashlighteffect.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumOnlineGhostEntity, DT_MOM_OnlineGhost, CMomentumOnlineGhostEntity)
    RecvPropBool(RECVINFO(m_bSpectating)),
    RecvPropFloat(RECVINFO(m_vecViewOffset[0])),
    RecvPropFloat(RECVINFO(m_vecViewOffset[1])),
    RecvPropFloat(RECVINFO(m_vecViewOffset[2])),
END_RECV_TABLE();

C_MomentumOnlineGhostEntity::C_MomentumOnlineGhostEntity(): m_bSpectating(false),
                                                            m_pFlashlight(nullptr), m_pEntityPanel(nullptr)
{
}

C_MomentumOnlineGhostEntity::~C_MomentumOnlineGhostEntity()
{
    if (m_pEntityPanel)
        m_pEntityPanel->DeletePanel();

    m_pEntityPanel = nullptr;

    delete m_pFlashlight;
}

void C_MomentumOnlineGhostEntity::Spawn()
{
    Precache();
    SetNextClientThink(CLIENT_THINK_ALWAYS);

    m_pEntityPanel = new CGhostEntityPanel();
    m_pEntityPanel->Init(this);
}
void C_MomentumOnlineGhostEntity::ClientThink()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);

    m_pEntityPanel->SetVisible(!(m_bSpectating || m_bSpectated));
}

void C_MomentumOnlineGhostEntity::Simulate()
{
    BaseClass::Simulate();

    // The dim light is the flashlight.
    if (IsEffectActive(EF_DIMLIGHT))
    {
        if (!m_pFlashlight)
        {
            // Turned on the headlight; create it.
            m_pFlashlight = new CFlashlightEffect(m_index);

            if (!m_pFlashlight)
                return;

            m_pFlashlight->TurnOn();
        }

        Vector vecForward, vecRight, vecUp;
        AngleVectors(EyeAngles(), &vecForward, &vecRight, &vecUp);

        // Update the light with the new position and direction.		
        m_pFlashlight->UpdateLight(EyePosition(), vecForward, vecRight, vecUp, 1000);
    }
    else if (m_pFlashlight)
    {
        // Turned off the flashlight; delete it.
        delete m_pFlashlight;
        m_pFlashlight = nullptr;
    }
}

void C_MomentumOnlineGhostEntity::CreateLightEffects()
{
    // Stubbed so we don't get a light at our feet
}
