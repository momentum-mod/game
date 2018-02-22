#pragma once

#include "cbase.h"
#include "weapon_csbase.h"

#ifdef CLIENT_DLL
#define CMomentumGrenade C_MomentumGrenade
#endif

class CMomentumGrenade : public CWeaponCSBase
{
public:
    DECLARE_CLASS( CMomentumGrenade , CWeaponCSBase );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumGrenade();

    virtual void	Precache();

    bool			Deploy();
    bool			Holster( CBaseCombatWeapon *pSwitchingTo );

    void			PrimaryAttack();
    void			SecondaryAttack();

    bool			Reload();

    virtual void	ItemPostFrame();

    void			DecrementAmmo( CBaseCombatCharacter *pOwner );
    virtual void	StartGrenadeThrow();
    virtual void	ThrowGrenade();
    virtual void	DropGrenade();

    bool IsPinPulled() const;
    bool IsBeingThrown() const
    {
        return m_fThrowTime > 0;
    }

#ifndef CLIENT_DLL
    DECLARE_DATADESC();

    virtual bool AllowsAutoSwitchFrom( void ) const;

    int		CapabilitiesGet();

    // Each derived grenade class implements this.
    virtual void EmitGrenade( const Vector &vecSrc , const QAngle &vecAngles , const Vector &vecVel , AngularImpulse angImpulse , CBasePlayer *pPlayer );
#endif

protected:
    CNetworkVar( bool , m_bRedraw );	// Draw the weapon again after throwing a grenade
    CNetworkVar( bool , m_bPinPulled );	// Set to true when the pin has been pulled but the grenade hasn't been thrown yet.
    CNetworkVar( float , m_fThrowTime ); // the time at which the grenade will be thrown.  If this value is 0 then the time hasn't been set yet.

public:

    CMomentumGrenade( const CMomentumGrenade & )
    {
    }

    CSWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_GRENADE; }
};



inline bool CMomentumGrenade::IsPinPulled() const
{
    return m_bPinPulled;
}