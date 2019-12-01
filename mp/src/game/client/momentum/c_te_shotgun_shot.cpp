#include "cbase.h"
#include "c_basetempentity.h"
#include "fx_mom_shared.h"

#include "tier0/memdbgon.h"

class C_TEFireBullets : public C_BaseTempEntity
{
  public:
    DECLARE_CLASS(C_TEFireBullets, C_BaseTempEntity);
    DECLARE_CLIENTCLASS();

    virtual void PostDataUpdate(DataUpdateType_t updateType);

    int m_iEntity;
    Vector m_vecOrigin;
    QAngle m_vecAngles;
    int m_iAmmoType;
    bool m_bSecondaryMode;
    int m_iSeed;
    float m_flSpread;
};

void C_TEFireBullets::PostDataUpdate(DataUpdateType_t updateType)
{
    // Create the effect.
    m_vecAngles.z = 0;
    FX_FireBullets(m_iEntity, m_vecOrigin, m_vecAngles, m_iAmmoType, m_bSecondaryMode, m_iSeed, m_flSpread);
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TEFireBullets, DT_TEFireBullets, CTEFireBullets);

BEGIN_RECV_TABLE_NOBASE(C_TEFireBullets, DT_TEFireBullets)
RecvPropVector(RECVINFO(m_vecOrigin)),
RecvPropFloat(RECVINFO(m_vecAngles[0])),
RecvPropFloat(RECVINFO(m_vecAngles[1])),
RecvPropInt(RECVINFO(m_iAmmoType)),
RecvPropBool(RECVINFO(m_bSecondaryMode)),
RecvPropInt(RECVINFO(m_iSeed)),
RecvPropInt(RECVINFO(m_iEntity)),
RecvPropFloat(RECVINFO(m_flSpread)),
END_RECV_TABLE();