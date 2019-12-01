#include "cbase.h"
#include "basetempentity.h"


#define NUM_BULLET_SEED_BITS 8


//-----------------------------------------------------------------------------
// Purpose: Display's a blood sprite
//-----------------------------------------------------------------------------
class CTEFireBullets : public CBaseTempEntity
{
public:
    DECLARE_CLASS(CTEFireBullets, CBaseTempEntity);
    DECLARE_SERVERCLASS();

    CTEFireBullets(const char *name);
    virtual			~CTEFireBullets(void);

public:
    CNetworkVar(int, m_iEntity);
    CNetworkVector(m_vecOrigin);
    CNetworkQAngle(m_vecAngles);
    CNetworkVar(int, m_iAmmoType);
    CNetworkVar(bool, m_bSecondaryMode);
    CNetworkVar(int, m_iSeed);
    CNetworkVar(float, m_flSpread);

};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
CTEFireBullets::CTEFireBullets(const char *name) :
CBaseTempEntity(name)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTEFireBullets::~CTEFireBullets(void)
{
}

IMPLEMENT_SERVERCLASS_ST_NOBASE(CTEFireBullets, DT_TEFireBullets)
SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD),
SendPropAngle(SENDINFO_VECTORELEM(m_vecAngles, 0), 13, 0),
SendPropAngle(SENDINFO_VECTORELEM(m_vecAngles, 1), 13, 0),
SendPropInt(SENDINFO(m_iAmmoType), 4, SPROP_UNSIGNED), // max 16 weapons
SendPropBool(SENDINFO(m_bSecondaryMode)),
SendPropInt(SENDINFO(m_iSeed), NUM_BULLET_SEED_BITS, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iEntity), MAX_EDICT_BITS, SPROP_UNSIGNED), // max 2048 ents, see MAX_EDICTS
SendPropFloat(SENDINFO(m_flSpread), 10, 0, 0, 1),
END_SEND_TABLE()


// Singleton
static CTEFireBullets g_TEFireBullets("Shotgun Shot");


void TE_FireBullets(
    int	iEntIndex,
    const Vector &vOrigin,
    const QAngle &vAngles,
    int	iAmmoType,
    bool	bSecondaryMode,
    int iSeed,
    float flSpread)
{
    CPASFilter filter(vOrigin);
    filter.UsePredictionRules();

    g_TEFireBullets.m_iEntity = iEntIndex;
    g_TEFireBullets.m_vecOrigin = vOrigin;
    g_TEFireBullets.m_vecAngles = vAngles;
    g_TEFireBullets.m_iSeed = iSeed;
    g_TEFireBullets.m_flSpread = flSpread;
    g_TEFireBullets.m_bSecondaryMode = bSecondaryMode;
    g_TEFireBullets.m_iAmmoType = iAmmoType;

    Assert(iSeed < (1 << NUM_BULLET_SEED_BITS));

    g_TEFireBullets.Create(filter, 0);
}