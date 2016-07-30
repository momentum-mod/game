#include "cbase.h"
#include "weapon_csbasegun.h"
#include "mom_player_shared.h"
#include "fx_cs_shared.h"
#include "weapon_mom_rifle.h"

#ifdef CLIENT_DLL
#define CAK47 C_AK47
#define CWeaponAug C_WeaponAug
#define CWeaponAWP C_WeaponAWP
#define CWeaponFamas C_WeaponFamas
#define CWeaponG3SG1 C_WeaponG3SG1
#define CWeaponGalil C_WeaponGalil
#define CWeaponM4A1 C_WeaponM4A1
#define CWeaponScout C_WeaponScout
#define CWeaponSG550 C_WeaponSG550
#define CWeaponSG552 C_WeaponSG552
#else
#include "KeyValues.h"
#endif

#include "tier0/memdbgon.h"


/*****************
AK47
*****************/
class CAK47 : public CMomentumRifle
{
public:
    DECLARE_CLASS(CAK47, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CAK47() {}
};

BEGIN_PREDICTION_DATA(CAK47)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_ak47, CAK47);
PRECACHE_WEAPON_REGISTER(weapon_ak47);

/*
AUG
*/
class CWeaponAug : public CMomentumRifle
{
public:
    DECLARE_CLASS(CWeaponAug, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CWeaponAug() {}
};

BEGIN_PREDICTION_DATA(CWeaponAug)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_aug, CWeaponAug);
PRECACHE_WEAPON_REGISTER(weapon_aug);

/*
AWP
*/
#define SNIPER_ZOOM_CONTEXT		"SniperRifleThink"
class CWeaponAWP : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CWeaponAWP, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
    DECLARE_DATADESC();
#endif

    CWeaponAWP();

    virtual void Spawn();

    virtual void PrimaryAttack();
    virtual void SecondaryAttack();

    virtual void AWPFire(float flSpread);

    virtual float GetMaxSpeed() const;
    virtual bool IsAwp() const;

    virtual CSWeaponID GetWeaponID(void) const { return WEAPON_AWP; }

private:

#ifndef CLIENT_DLL
    void				UnzoomThink(void);
#endif

    CWeaponAWP(const CWeaponAWP &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponAWP, DT_WeaponAWP)

BEGIN_NETWORK_TABLE(CWeaponAWP, DT_WeaponAWP)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponAWP)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_awp, CWeaponAWP);
PRECACHE_WEAPON_REGISTER(weapon_awp);

#ifndef CLIENT_DLL

BEGIN_DATADESC(CWeaponAWP)
DEFINE_THINKFUNC(UnzoomThink),
END_DATADESC()

#endif

CWeaponAWP::CWeaponAWP()
{
}

void CWeaponAWP::Spawn()
{
    Precache();

    BaseClass::Spawn();
}


void CWeaponAWP::SecondaryAttack()
{
#ifndef CLIENT_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer == nullptr)
    {
        Assert(pPlayer != nullptr);
        return;
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        pPlayer->SetFOV(pPlayer, 40, 0.15f);
    }
    else if (pPlayer->GetFOV() == 40)
    {
        pPlayer->SetFOV(pPlayer, 10, 0.08f);
    }
    else
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.1f);
    }

    //pPlayer->ResetMaxSpeed();

#endif

#ifndef CLIENT_DLL
    // If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
    // Let the server play it since if only the client plays it, it's liable to get played twice cause of
    // a prediction error. joy.
    EmitSound("Default.Zoom");

    // let the bots hear the rifle zoom
    IGameEvent * event = gameeventmanager->CreateEvent("weapon_zoom");
    if (event)
    {
        event->SetInt("userid", pPlayer->GetUserID());
        gameeventmanager->FireEvent(event);
    }
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.15; // The worst zoom time from above.  

}


void CWeaponAWP::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        AWPFire(0.85);

    else if (pPlayer->GetAbsVelocity().Length2D() > 140)
        AWPFire(0.25);

    else if (pPlayer->GetAbsVelocity().Length2D() > 10)
        AWPFire(0.10);

    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        AWPFire(0.0);

    else
        AWPFire(0.001);
}

void CWeaponAWP::AWPFire(float flSpread)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer == nullptr)
    {
        Assert(pPlayer != nullptr);
        return;
    }

    // If we are not zoomed in, or we have very recently zoomed and are still transitioning, the bullet diverts more.
    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV() || (gpGlobals->curtime < m_zoomFullyActiveTime))
    {
        flSpread += 0.08;
    }

    if (pPlayer->GetFOV() != pPlayer->GetDefaultFOV())
    {
        pPlayer->m_iLastZoom = pPlayer->GetFOV();

#ifndef CLIENT_DLL
#ifdef AWP_UNZOOM
        SetContextThink(&CWeaponAWP::UnzoomThink, gpGlobals->curtime + sv_awpunzoomdelay.GetFloat(), SNIPER_ZOOM_CONTEXT);
#else
        pPlayer->m_bResumeZoom = true;
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.1f);
#endif
#endif
    }

    if (!CSBaseGunFire(flSpread, GetCSWpnData().m_flCycleTime, true))
        return;

    QAngle angle = pPlayer->GetPunchAngle();
    angle.x -= 2;
    pPlayer->SetPunchAngle(angle);
}

#ifndef CLIENT_DLL
void CWeaponAWP::UnzoomThink(void)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer == nullptr)
    {
        Assert(pPlayer != nullptr);
        return;
    }

    pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.1f);
}
#endif


float CWeaponAWP::GetMaxSpeed() const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer == nullptr)
    {
        Assert(pPlayer != nullptr);
        return BaseClass::GetMaxSpeed();
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        return BaseClass::GetMaxSpeed();
    }
    else
    {
        // Slower speed when zoomed in.
        return 150;
    }
}


bool CWeaponAWP::IsAwp() const
{
    return true;
}



/*
FAMAS
*/
class CWeaponFamas : public CMomentumRifle
{
public:
    DECLARE_CLASS(CWeaponFamas, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CWeaponFamas() {};
};

BEGIN_PREDICTION_DATA(CWeaponFamas)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_famas, CWeaponFamas);
PRECACHE_WEAPON_REGISTER(weapon_famas);


/*
G3SG1 - CT Autosniper
*/
class CWeaponG3SG1 : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CWeaponG3SG1, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponG3SG1();

    virtual void Spawn();
    virtual void SecondaryAttack();
    virtual void PrimaryAttack();
    virtual bool Reload();
    virtual bool Deploy();

    virtual float GetMaxSpeed();

    virtual CSWeaponID GetWeaponID(void) const { return WEAPON_G3SG1; }

private:

    CWeaponG3SG1(const CWeaponG3SG1 &);

    void G3SG1Fire(float flSpread);


    float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponG3SG1, DT_WeaponG3SG1)

BEGIN_NETWORK_TABLE(CWeaponG3SG1, DT_WeaponG3SG1)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponG3SG1)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_g3sg1, CWeaponG3SG1);
PRECACHE_WEAPON_REGISTER(weapon_g3sg1);



CWeaponG3SG1::CWeaponG3SG1()
{
    m_flLastFire = gpGlobals->curtime;
}

void CWeaponG3SG1::Spawn()
{
    BaseClass::Spawn();
    m_flAccuracy = 0.98;
}


void CWeaponG3SG1::SecondaryAttack()
{
#ifndef CLIENT_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        pPlayer->SetFOV(pPlayer, 40, 0.3f);
    }
    else if (pPlayer->GetFOV() == 40)
    {
        pPlayer->SetFOV(pPlayer, 15, 0.05);
    }
    else if (pPlayer->GetFOV() == 15)
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.1f);
    }

    //pPlayer->ResetMaxSpeed();
#endif

#ifndef CLIENT_DLL
    // If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
    // Let the server play it since if only the client plays it, it's liable to get played twice cause of
    // a prediction error. joy.
    EmitSound("Default.Zoom"); // zoom sound

    // let the bots hear the rifle zoom
    IGameEvent * event = gameeventmanager->CreateEvent("weapon_zoom");
    if (event)
    {
        event->SetInt("userid", pPlayer->GetUserID());
        gameeventmanager->FireEvent(event);
    }
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.3; // The worst zoom time from above.  
}

void CWeaponG3SG1::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        G3SG1Fire(0.45 * (1 - m_flAccuracy));
    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        G3SG1Fire(0.15);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        G3SG1Fire(0.035 * (1 - m_flAccuracy));
    else
        G3SG1Fire(0.055 * (1 - m_flAccuracy));
}

void CWeaponG3SG1::G3SG1Fire(float flSpread)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    // If we are not zoomed in, or we have very recently zoomed and are still transitioning, the bullet diverts more.
    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV() || (gpGlobals->curtime < m_zoomFullyActiveTime))
        flSpread += 0.025;

    // Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
    m_flAccuracy = 0.55 + (0.3) * (gpGlobals->curtime - m_flLastFire);

    if (m_flAccuracy > 0.98)
        m_flAccuracy = 0.98;

    m_flLastFire = gpGlobals->curtime;

    if (!CSBaseGunFire(flSpread, GetCSWpnData().m_flCycleTime, true))
        return;

    // Adjust the punch angle.
    QAngle angle = pPlayer->GetPunchAngle();
    angle.x -= SharedRandomFloat("G3SG1PunchAngleX", 0.75, 1.75) + (angle.x / 4);
    angle.y += SharedRandomFloat("G3SG1PunchAngleY", -0.75, 0.75);
    pPlayer->SetPunchAngle(angle);
}


bool CWeaponG3SG1::Reload()
{
    bool ret = BaseClass::Reload();

    m_flAccuracy = 0.98;

    return ret;
}

bool CWeaponG3SG1::Deploy()
{
    bool ret = BaseClass::Deploy();

    m_flAccuracy = 0.98;

    return ret;
}

float CWeaponG3SG1::GetMaxSpeed()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer && pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
        return BaseClass::GetMaxSpeed();
    else
        return 150; // zoomed in
}


/*
GALIL
*/
class CWeaponGalil : public CMomentumRifle
{
public:
    DECLARE_CLASS(CWeaponGalil, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CWeaponGalil() {}

};

BEGIN_PREDICTION_DATA(CWeaponGalil)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_galil, CWeaponGalil);
PRECACHE_WEAPON_REGISTER(weapon_galil);

/*
M4A1
*/
class CWeaponM4A1 : public CMomentumRifle
{
public:
    DECLARE_CLASS(CWeaponM4A1, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CWeaponM4A1() {}
};

BEGIN_PREDICTION_DATA(CWeaponM4A1)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m4a1, CWeaponM4A1);
PRECACHE_WEAPON_REGISTER(weapon_m4a1);


//SCOUT
class CWeaponScout : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CWeaponScout, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponScout();

    virtual void PrimaryAttack();
    virtual void SecondaryAttack();

    virtual float GetMaxSpeed() const;

    virtual CSWeaponID GetWeaponID(void) const { return WEAPON_SCOUT; }


private:

    CWeaponScout(const CWeaponScout &);

    void SCOUTFire(float flSpread);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponScout, DT_WeaponScout)

BEGIN_NETWORK_TABLE(CWeaponScout, DT_WeaponScout)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponScout)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_scout, CWeaponScout);
PRECACHE_WEAPON_REGISTER(weapon_scout);



CWeaponScout::CWeaponScout()
{
}

void CWeaponScout::SecondaryAttack()
{
#ifndef CLIENT_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer == NULL)
    {
        Assert(pPlayer != NULL);
        return;
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        pPlayer->SetFOV(pPlayer, 40, 0.15f);
    }
    else if (pPlayer->GetFOV() == 40)
    {
        pPlayer->SetFOV(pPlayer, 15, 0.05);
    }
    else if (pPlayer->GetFOV() == 15)
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.05f);
    }

    //pPlayer->ResetMaxSpeed();
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.15; // The worst zoom time from above.  

#ifndef CLIENT_DLL
    // If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
    // Let the server play it since if only the client plays it, it's liable to get played twice cause of
    // a prediction error. joy.
    EmitSound("Default.Zoom"); // zoom sound

    // let the bots hear the rifle zoom
    IGameEvent * event = gameeventmanager->CreateEvent("weapon_zoom");
    if (event)
    {
        event->SetInt("userid", pPlayer->GetUserID());
        gameeventmanager->FireEvent(event);
    }
#endif
}

void CWeaponScout::PrimaryAttack(void)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer == NULL)
    {
        Assert(pPlayer != NULL);
        return;
    }

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        SCOUTFire(0.2);
    else if (pPlayer->GetAbsVelocity().Length2D() > 170)
        SCOUTFire(0.075);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        SCOUTFire(0.0);
    else
        SCOUTFire(0.007);
}

void CWeaponScout::SCOUTFire(float flSpread)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer == NULL)
    {
        Assert(pPlayer != NULL);
        return;
    }

    // If we are not zoomed in, or we have very recently zoomed and are still transitioning, the bullet diverts more.
    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV() || (gpGlobals->curtime < m_zoomFullyActiveTime))
    {
        flSpread += 0.025;
    }

    if (pPlayer->GetFOV() != pPlayer->GetDefaultFOV())
    {
        pPlayer->m_bResumeZoom = true;
        pPlayer->m_iLastZoom = pPlayer->GetFOV();

#ifndef CLIENT_DLL
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.05f);
#endif
    }

    if (!CSBaseGunFire(flSpread, GetCSWpnData().m_flCycleTime, true))
        return;

    QAngle angle = pPlayer->GetPunchAngle();
    angle.x -= 2;
    pPlayer->SetPunchAngle(angle);
}

//MOM_TODO: Consider LJ gametype
float CWeaponScout::GetMaxSpeed() const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer == NULL)
    {
        Assert(pPlayer != NULL);
        return BaseClass::GetMaxSpeed();
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
        return BaseClass::GetMaxSpeed();
    else
        return 220;	// zoomed in.
}



/*
SG550 - Autosniper
*/
class CWeaponSG550 : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CWeaponSG550, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponSG550();

    virtual void Spawn();
    virtual void SecondaryAttack();
    virtual void PrimaryAttack();
    virtual bool Reload();
    virtual bool Deploy();

    virtual float GetMaxSpeed() const;

    virtual CSWeaponID GetWeaponID(void) const { return WEAPON_SG550; }


private:

    CWeaponSG550(const CWeaponSG550 &);

    void SG550Fire(float flSpread);

    float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSG550, DT_WeaponSG550)

BEGIN_NETWORK_TABLE(CWeaponSG550, DT_WeaponSG550)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSG550)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_sg550, CWeaponSG550);
PRECACHE_WEAPON_REGISTER(weapon_sg550);



CWeaponSG550::CWeaponSG550()
{
    m_flLastFire = gpGlobals->curtime;
}

void CWeaponSG550::Spawn()
{
    BaseClass::Spawn();
    m_flAccuracy = 0.98;
}


void CWeaponSG550::SecondaryAttack()
{
#ifndef CLIENT_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        pPlayer->SetFOV(pPlayer, 40, 0.3f);
    }
    else if (pPlayer->GetFOV() == 40)
    {
        pPlayer->SetFOV(pPlayer, 15, 0.05f);
    }
    else if (pPlayer->GetFOV() == 15)
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.1f);
    }

    //pPlayer->ResetMaxSpeed();
#endif

#ifndef CLIENT_DLL
    // If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
    // Let the server play it since if only the client plays it, it's liable to get played twice cause of
    // a prediction error. joy.
    EmitSound("Default.Zoom"); // zoom sound.

    // let the bots hear the rifle zoom
    IGameEvent * event = gameeventmanager->CreateEvent("weapon_zoom");
    if (event)
    {
        event->SetInt("userid", pPlayer->GetUserID());
        gameeventmanager->FireEvent(event);
    }
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.3; // The worst zoom time from above.  
}

void CWeaponSG550::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        SG550Fire(0.45f * (1 - m_flAccuracy));
    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        SG550Fire(0.15f);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        SG550Fire(0.04f * (1 - m_flAccuracy));
    else
        SG550Fire(0.05f * (1 - m_flAccuracy));
}

void CWeaponSG550::SG550Fire(float flSpread)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    // If we are not zoomed in, or we have very recently zoomed and are still transitioning, the bullet diverts more.
    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV() || (gpGlobals->curtime < m_zoomFullyActiveTime))
        flSpread += 0.025;

    // Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
    m_flAccuracy = 0.65 + (0.35) * (gpGlobals->curtime - m_flLastFire);

    if (m_flAccuracy > 0.98)
        m_flAccuracy = 0.98;

    m_flLastFire = gpGlobals->curtime;

    if (!CSBaseGunFire(flSpread, GetCSWpnData().m_flCycleTime, true))
        return;

    QAngle angle = pPlayer->GetPunchAngle();
    angle.x -= SharedRandomFloat("SG550PunchAngleX", 0.75, 1.25) + (angle.x / 4);
    angle.y += SharedRandomFloat("SG550PunchAngleY", -0.75, 0.75);
    pPlayer->SetPunchAngle(angle);
}

bool CWeaponSG550::Reload()
{
    bool ret = BaseClass::Reload();

    m_flAccuracy = 0.98;

    return ret;
}

bool CWeaponSG550::Deploy()
{
    bool ret = BaseClass::Deploy();

    m_flAccuracy = 0.98;

    return ret;
}

float CWeaponSG550::GetMaxSpeed() const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer || pPlayer->GetFOV() == 90)
        return BaseClass::GetMaxSpeed();
    else
        return 150; // zoomed in
}


/*
SG552 - T-sided AUG
*/
class CWeaponSG552 : public CMomentumRifle
{
public:
    DECLARE_CLASS(CWeaponSG552, CMomentumRifle);
    DECLARE_PREDICTABLE();

    CWeaponSG552() {}
};

BEGIN_PREDICTION_DATA(CWeaponSG552)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_sg552, CWeaponSG552);
PRECACHE_WEAPON_REGISTER(weapon_sg552);