#include "cbase.h"

#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_base.h"
#include "ammodef.h"
#include "mom_player_shared.h"
#include "movevars_shared.h"
#include "util/mom_util.h"

#if defined( CLIENT_DLL )

#include "vgui/ISurface.h"
//#include "vgui_controls/controls.h"
#include "hud_crosshair.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"

extern IVModelInfoClient* modelinfo;

#else

#include "te_effect_dispatch.h"
#include "KeyValues.h"

extern IVModelInfo* modelinfo;

#endif


// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] =
{
    "none",		// WEAPON_NONE
    "momentum_pistol",	// WEAPON_PISTOL
    "momentum_rifle", //WEAPON_RIFLE
    "momentum_shotgun", //WEAPON_SHOTGUN
    "momentum_smg", //WEAPON_SMG
    "momentum_sniper", //WEAPON_SNIPER
    "momentum_lmg", //WEAPON_LMG
    "momentum_grenade", //WEAPON_GRENADE
    "knife",	// WEAPON_KNIFE
    "momentum_paintgun", // WEAPON_PAINTGUN
    "momentum_rocketlauncher", // WEAPON_ROCKETLAUNCHER
    nullptr,		// WEAPON_NONE
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID(const char *alias)
{
    if (alias)
    {
        for (int i = 0; s_WeaponAliasInfo[i] != nullptr; ++i)
            if (!Q_stricmp(s_WeaponAliasInfo[i], alias))
                return i;
    }

    return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias(int id)
{
    if (id >= WEAPON_MAX || id < 0)
        return nullptr;

    return s_WeaponAliasInfo[id];
}


// ----------------------------------------------------------------------------- //
// CWeaponBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponBase, DT_WeaponBase)

BEGIN_NETWORK_TABLE(CWeaponBase, DT_WeaponBase)
#ifdef CLIENT_DLL

#else
// world weapon models have no aminations
SendPropExclude("DT_AnimTimeMustBeFirst", "m_flAnimTime"),
SendPropExclude("DT_BaseAnimating", "m_nSequence"),
//	SendPropExclude( "DT_LocalActiveWeaponData", "m_flTimeWeaponIdle" ),
#endif


END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponBase)
DEFINE_PRED_FIELD(m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_cs_base, CWeaponBase);


#ifdef GAME_DLL

BEGIN_DATADESC(CWeaponBase)

//DEFINE_FUNCTION( DefaultTouch ),
DEFINE_FUNCTION(FallThink)

END_DATADESC()

#endif

// ----------------------------------------------------------------------------- //
// CWeaponBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponBase::CWeaponBase()
{
    SetPredictionEligible(true);
    m_bDelayFire = true;
    m_nextPrevOwnerTouchTime = 0.0;
    m_prevOwner = nullptr;
    AddSolidFlags(FSOLID_TRIGGER); // Nothing collides with these but it gets touches.

#ifdef CLIENT_DLL
    m_iCrosshairTextureID = 0;

    m_bInReloadAnimation = false;
#else
    m_iDefaultExtraAmmo = 0;
#endif

    m_flAccuracy = 1.0f;
    m_flDecreaseShotsFired = 0.0f;
    m_iExtraPrimaryAmmo = 0;
}


#ifndef CLIENT_DLL
bool CWeaponBase::KeyValue(const char *szKeyName, const char *szValue)
{
    if (!BaseClass::KeyValue(szKeyName, szValue))
    {
        if (FStrEq(szKeyName, "ammo"))
        {
            int bullets = atoi(szValue);
            if (bullets < 0)
                return false;

            m_iDefaultExtraAmmo = bullets;

            return true;
        }
    }

    return false;
}
#endif

bool CWeaponBase::PlayEmptySound()
{
    //MIKETODO: certain weapons should override this to make it empty:
    //	C4
    //	Flashbang
    //	HE Grenade
    //	Smoke grenade				

    CPASAttenuationFilter filter(this);
    filter.UsePredictionRules();

    if (IsPistol())
    {
        EmitSound(filter, entindex(), "Default.ClipEmpty_Pistol");
    }
    else
    {
        EmitSound(filter, entindex(), "Default.ClipEmpty_Rifle");
    }

    return false;
}

CMomentumPlayer* CWeaponBase::GetPlayerOwner() const
{
    return ToCMOMPlayer(GetOwner());
}

void CWeaponBase::ItemPostFrame()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (m_bInReload && pPlayer->m_flNextAttack <= gpGlobals->curtime)
    {
        // complete the reload. 
        int j = min(GetMaxClip1() - m_iClip1, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));

        // Add them to the clip
        m_iClip1 += j;
        pPlayer->RemoveAmmo(j, m_iPrimaryAmmoType);

        m_bInReload = false;
    }

    if (pPlayer->m_nButtons & IN_ATTACK2 && m_flNextSecondaryAttack <= gpGlobals->curtime)
    {
        if (m_iClip2 != -1 && !pPlayer->GetAmmoCount(GetSecondaryAmmoType()))
        {
            m_bFireOnEmpty = true;
        }

        SecondaryAttack();

        pPlayer->m_nButtons &= ~IN_ATTACK2;
    }
    else if (pPlayer->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack <= gpGlobals->curtime)
    {
        if (m_iClip1 == 0/* && pszAmmo1()*/ || GetMaxClip1() == -1 && !pPlayer->GetAmmoCount(GetPrimaryAmmoType()))
        {
            m_bFireOnEmpty = true;
        }

        PrimaryAttack();
        //---
    }
    else if (pPlayer->m_nButtons & IN_RELOAD && GetMaxClip1() != WEAPON_NOCLIP && !m_bInReload && m_flNextPrimaryAttack < gpGlobals->curtime)
    {
        // reload when reload is pressed, or if no buttons are down and weapon is empty.
         Reload();
    }
    else if (!(pPlayer->m_nButtons & (IN_ATTACK | IN_ATTACK2)))
    {
        // no fire buttons down

        // The following code prevents the player from tapping the firebutton repeatedly 
        // to simulate full auto and retaining the single shot accuracy of single fire
        if (m_bDelayFire)
        {
            m_bDelayFire = false;

            if (pPlayer->m_iShotsFired > 15)
                pPlayer->m_iShotsFired = 15;

            m_flDecreaseShotsFired = gpGlobals->curtime + 0.4f;
        }

        m_bFireOnEmpty = FALSE;

        // if it's a pistol then set the shots fired to 0 after the player releases a button
        if (IsPistol())
        {
            pPlayer->m_iShotsFired = 0;
        }
        else
        {
            if (pPlayer->m_iShotsFired > 0 && m_flDecreaseShotsFired < gpGlobals->curtime)
            {
                m_flDecreaseShotsFired = gpGlobals->curtime + 0.0225f;
                --pPlayer->m_iShotsFired;
            }
        }

        if (!IsUseable() && m_flNextPrimaryAttack < gpGlobals->curtime
#ifdef CLIENT_DLL
            || m_bInReloadAnimation
#endif
            )
        {
            // Intentionally blank -- used to switch weapons here
        }
        else
        {
            // weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
            if (m_iClip1 == 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime)
            {
                Reload();
                return;
            }
        }

        WeaponIdle();
    }
}


float CWeaponBase::GetMaxSpeed() const
{
    return sv_maxspeed.GetFloat();
}


const CWeaponInfo &CWeaponBase::GetMomWpnData() const
{
    const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
    const CWeaponInfo *pInfo;

#ifdef _DEBUG
    pInfo = dynamic_cast< const CWeaponInfo* >( pWeaponInfo );
    Assert( pInfo );
#else
    pInfo = static_cast<const CWeaponInfo*>(pWeaponInfo);
#endif

    return *pInfo;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CWeaponBase::GetViewModel(int /*viewmodelindex = 0 -- this is ignored in the base class here*/) const
{
    CMomentumPlayer *pOwner = GetPlayerOwner();

    if (pOwner == nullptr)
    {
        return BaseClass::GetViewModel();
    }

    return GetWpnData().szViewModel;
}

// Overridden for the CS gun overrides, since GetClassname returns the weapon_glock etc, instead
// of the weapon_momentum_* class. So we do a little workaround with the weapon ID.
void CWeaponBase::Precache(void)
{
    PrecacheScriptSound("Default.ClipEmpty_Pistol");
    PrecacheScriptSound("Default.ClipEmpty_Rifle");
    PrecacheScriptSound("Default.Zoom");

    const char *pWeaponAlias = WeaponIDToAlias(GetWeaponID());

    if (pWeaponAlias)
    {
        char wpnName[128];
        Q_snprintf(wpnName, sizeof(wpnName), "weapon_%s", pWeaponAlias);
        
        // Add this weapon to the weapon registry, and get our index into it
        // Get weapon data from script file
        if (ReadWeaponDataFromFileForSlot(filesystem, wpnName, &m_hWeaponFileInfo, GetEncryptionKey()))
        {
#if defined(CLIENT_DLL)
            gWR.LoadWeaponSprites(GetWeaponFileInfoHandle());
#endif
            // Precache models (preload to avoid hitch)
            m_iViewModelIndex = 0;
            m_iWorldModelIndex = 0;
            if (GetViewModel() && GetViewModel()[0])
            {
                m_iViewModelIndex = PrecacheModel(GetViewModel());
            }
            if (GetWorldModel() && GetWorldModel()[0])
            {
                m_iWorldModelIndex = PrecacheModel(GetWorldModel());
            }

            // Precache sounds, too
            for (int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i)
            {
                const char *shootsound = GetShootSound(i);
                if (shootsound && shootsound[0])
                {
                    PrecacheScriptSound(shootsound);
                }
            }
        }
        else
        {
            // Couldn't read data file, remove myself
            Warning("Error reading weapon data file for: %s\n", GetClassname());
            //	Remove( );	//don't remove, this gets released soon!
        }
    }
    else
    {
        Warning("Error reading weapon data file for weapon alias: %i \n", GetWeaponID());
    }
}

Activity CWeaponBase::GetDeployActivity(void)
{
    return ACT_VM_DRAW;
}

bool CWeaponBase::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt)
{
    // Msg( "deploy %s at %f\n", GetClassname(), gpGlobals->curtime );
    CMomentumPlayer *pOwner = GetPlayerOwner();
    if (!pOwner)
    {
        return false;
    }

    pOwner->SetAnimationExtension(szAnimExt);

    SetViewModel();
    SendWeaponAnim(GetDeployActivity());

    pOwner->SetNextAttack(gpGlobals->curtime + DeployTime());
    m_flNextPrimaryAttack = gpGlobals->curtime + DeployTime();
    m_flNextSecondaryAttack = gpGlobals->curtime + DeployTime();

    SetWeaponVisible(true);
    SetWeaponModelIndex(szWeaponModel);

    return true;
}

void CWeaponBase::SetWeaponModelIndex(const char *pName)
{
    m_iWorldModelIndex = modelinfo->GetModelIndex(pName);
}

bool CWeaponBase::CanBeSelected(void)
{
    if (!VisibleInWeaponSelection())
        return false;

    return true;
}

bool CWeaponBase::CanDeploy(void)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    return BaseClass::CanDeploy();
}

bool CWeaponBase::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

#ifndef CLIENT_DLL
    if (pPlayer)
        pPlayer->SetFOV(pPlayer, 0); // reset the default FOV.
#endif

    return BaseClass::Holster(pSwitchingTo);
}

bool CWeaponBase::Deploy()
{
#ifdef CLIENT_DLL
    m_iAlpha = 80;
#else

    m_flDecreaseShotsFired = gpGlobals->curtime;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer)
    {
        pPlayer->m_iShotsFired = 0;
        pPlayer->m_bResumeZoom = false;
        pPlayer->m_iLastZoomFOV = 0;
        pPlayer->SetFOV(pPlayer, 0);
    }
#endif

    return BaseClass::Deploy();
}

#ifndef CLIENT_DLL
bool CWeaponBase::IsRemoveable()
{
    if (BaseClass::IsRemoveable() == true)
    {
        if (m_nextPrevOwnerTouchTime > gpGlobals->curtime)
        {
            return false;
        }
    }

    return BaseClass::IsRemoveable();
}
#endif

void CWeaponBase::Drop(const Vector &vecVelocity)
{

#ifdef CLIENT_DLL
    BaseClass::Drop(vecVelocity);
    return;
#else

    // Once somebody drops a gun, it's fair game for removal when/if
    // a game_weapon_manager does a cleanup on surplus weapons in the
    // world.
    SetRemoveable(true);

    StopAnimation();
    StopFollowingEntity();
    SetMoveType(MOVETYPE_FLYGRAVITY);
    // clear follow stuff, setup for collision
    SetGravity(1.0);
    m_iState = WEAPON_NOT_CARRIED;
    RemoveEffects(EF_NODRAW);
    FallInit();
    SetGroundEntity(nullptr);

    m_bInReload = false; // stop reloading 

    SetThink(NULL);
    m_nextPrevOwnerTouchTime = gpGlobals->curtime + 0.8f;
    m_prevOwner = GetPlayerOwner();

    SetTouch(&CWeaponBase::DefaultTouch);

    IPhysicsObject *pObj = VPhysicsGetObject();
    if (pObj != nullptr)
    {
        AngularImpulse	angImp(200, 200, 200);
        pObj->AddVelocity(&vecVelocity, &angImp);
    }
    else
    {
        SetAbsVelocity(vecVelocity);
    }

    SetNextThink(gpGlobals->curtime);

    SetOwnerEntity(nullptr);
    SetOwner(nullptr);
#endif
}

// whats going on here is that if the player drops this weapon, they shouldn't take it back themselves
// for a little while.  But if they throw it at someone else, the other player should get it immediately.
void CWeaponBase::DefaultTouch(CBaseEntity *pOther)
{
    if (m_prevOwner && pOther == m_prevOwner && gpGlobals->curtime < m_nextPrevOwnerTouchTime)
    {
        return;
    }

    BaseClass::DefaultTouch(pOther);
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: Draw the weapon's crosshair
//-----------------------------------------------------------------------------
void CWeaponBase::DrawCrosshair()
{
    if (!crosshair.GetInt())
        return;

    CHudCrosshair *pCrosshair = GET_HUDELEMENT(CHudCrosshair);

    if (!pCrosshair)
        return;

    // clear crosshair
    pCrosshair->SetCrosshair(nullptr, Color(255, 255, 255, 255));
    
    // no crosshair for sniper rifles
    if (GetWeaponID() == WEAPON_SNIPER)
        return;

    pCrosshair->DrawCrosshair(this);
}


void CWeaponBase::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    if (GetPredictable() && !ShouldPredict())
        ShutdownPredictable();
}


bool CWeaponBase::ShouldPredict()
{
    if (GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
        return true;

    return BaseClass::ShouldPredict();
}

void CWeaponBase::ProcessMuzzleFlashEvent()
{
    // This is handled from the player's animstate, so it can match up to the beginning of the fire animation
}

bool CWeaponBase::OnFireEvent(C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options)
{
    if (event == 5001)
    {
        CMomentumPlayer *pPlayer = GetPlayerOwner();
        if (pPlayer && pPlayer->GetFOV() < pPlayer->GetDefaultFOV() && HideViewModelWhenZoomed())
            return true;
    }

    return BaseClass::OnFireEvent(pViewModel, origin, angles, event, options);
}

#else		

//-----------------------------------------------------------------------------
// Purpose: Get the accuracy derived from weapon and player, and return it
//-----------------------------------------------------------------------------
const Vector& CWeaponBase::GetBulletSpread()
{
    static Vector cone = VECTOR_CONE_8DEGREES;
    return cone;
}

//-----------------------------------------------------------------------------
// Purpose: Match the anim speed to the weapon speed while crouching
//-----------------------------------------------------------------------------
float CWeaponBase::GetDefaultAnimSpeed()
{
    return 1.0;
}

//-----------------------------------------------------------------------------
// Purpose: Draw the laser rifle effect
//-----------------------------------------------------------------------------
void CWeaponBase::BulletWasFired(const Vector &vecStart, const Vector &vecEnd)
{
}


bool CWeaponBase::ShouldRemoveOnRoundRestart()
{
    if (GetPlayerOwner())
        return false;
    else
        return true;
}


//=========================================================
// Materialize - make a CWeaponBase visible and tangible
//=========================================================
void CWeaponBase::Materialize()
{
    if (IsEffectActive(EF_NODRAW))
    {
        // changing from invisible state to visible.
        RemoveEffects(EF_NODRAW);
        DoMuzzleFlash();
    }

    AddSolidFlags(FSOLID_TRIGGER);

    //SetTouch( &CWeaponBase::DefaultTouch );

    SetThink(NULL);
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CWeaponBase::CheckRespawn()
{
    //GOOSEMAN : Do not respawn weapons!
    return;
}


//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CWeaponBase::Respawn()
{
    // make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
    // will decide when to make the weapon visible and touchable.
    CBaseEntity *pNewWeapon = Create(GetClassname(), g_pGameRules->VecWeaponRespawnSpot(this), GetAbsAngles(), GetOwner());

    if (pNewWeapon)
    {
        pNewWeapon->AddEffects(EF_NODRAW);// invisible for now
        pNewWeapon->SetTouch(NULL);// no touch
        pNewWeapon->SetThink(&BaseClass::AttemptToMaterialize);

        UTIL_DropToFloor(this, MASK_SOLID);

        // not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
        // but when it should respawn is based on conditions belonging to the weapon that was taken.
        pNewWeapon->SetNextThink(gpGlobals->curtime + g_pGameRules->FlWeaponRespawnTime(this));
    }
    else
    {
        Msg("Respawn failed to create %s!\n", GetClassname());
    }

    return pNewWeapon;
}

bool CWeaponBase::Reload()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    pPlayer->m_iShotsFired = 0;

    bool retval = BaseClass::Reload();

#ifdef CLIENT_DLL
    if (retval)
    {
        m_bInReloadAnimation = true;
    }
#endif

    return retval;
}

void CWeaponBase::Spawn()
{
    BaseClass::Spawn();

    // Override the bloat that our base class sets as it's a little bit bigger than we want.
    // If it's too big, you drop a weapon and its box is so big that you're still touching it
    // when it falls and you pick it up again right away.
    CollisionProp()->UseTriggerBounds(true, 30);

    // Set this here to allow players to shoot dropped weapons
    SetCollisionGroup(COLLISION_GROUP_WEAPON);

    SetExtraAmmoCount(m_iDefaultExtraAmmo);	//Start with no additional ammo

    m_nextPrevOwnerTouchTime = 0.0;
    m_prevOwner = nullptr;

#ifdef CLIENT_DLL
    m_bInReloadAnimation = false;
#endif
}

bool CWeaponBase::DefaultReload(int iClipSize1, int iClipSize2, int iActivity)
{
    if (BaseClass::DefaultReload(iClipSize1, iClipSize2, iActivity))
    {
        //SendReloadEvents(); MOM_TODO: will this be needed? (it sends it to other players)
        return true;
    }
    else
    {
        return false;
    }
}

void CWeaponBase::SendReloadEvents()
{
    //CMomentumPlayer *pPlayer = GetPlayerOwner();
    //if ( !pPlayer )
    //    return;

    // Make the player play his reload animation.
    //pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

#endif


bool CWeaponBase::DefaultPistolReload()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
        return true;

    if (!DefaultReload(GetMomWpnData().iDefaultClip1, 0, ACT_VM_RELOAD))
        return false;

    pPlayer->m_iShotsFired = 0;

#ifdef CLIENT_DLL
    m_bInReloadAnimation = true;
#endif

    return true;
}

bool CWeaponBase::IsUseable()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (Clip1() <= 0)
    {
        if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0 && GetMaxClip1() != -1)
        {
            // clip is empty (or nonexistant) and the player has no more ammo of this type. 
            return false;
        }
    }

    return true;
}


#if defined( CLIENT_DLL )

float	g_lateralBob;
float	g_verticalBob;

static ConVar	cl_bobcycle("cl_bobcycle", "0.8", FCVAR_CHEAT);
static ConVar	cl_bob("cl_bob", "0.002", FCVAR_CHEAT);
static ConVar	cl_bobup("cl_bobup", "0.5", FCVAR_CHEAT);

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CWeaponBase::CalcViewmodelBob(void)
{
    static	float bobtime;
    static	float lastbobtime;
    static  float lastspeed;
    float	cycle;

    CBasePlayer *player = ToBasePlayer(GetOwner());
    //Assert( player );

    //NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

    if (!gpGlobals->frametime ||
        player == nullptr ||
        cl_bobcycle.GetFloat() <= 0.0f ||
        cl_bobup.GetFloat() <= 0.0f ||
        cl_bobup.GetFloat() >= 1.0f)
    {
        //NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
        return 0.0f;// just use old value
    }

    //Find the speed of the player
    float speed = player->GetLocalVelocity().Length2D();
    float flmaxSpeedDelta = max(0, (gpGlobals->curtime - lastbobtime) * 320.0f);

    // don't allow too big speed changes
    speed = clamp(speed, lastspeed - flmaxSpeedDelta, lastspeed + flmaxSpeedDelta);
    speed = clamp(speed, -320, 320);

    lastspeed = speed;

    //FIXME: This maximum speed value must come from the server.
    //		 MaxSpeed() is not sufficient for dealing with sprinting - jdw



    float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);

    bobtime += (gpGlobals->curtime - lastbobtime) * bob_offset;
    lastbobtime = gpGlobals->curtime;

    //Calculate the vertical bob
    cycle = bobtime - (int) (bobtime / cl_bobcycle.GetFloat())*cl_bobcycle.GetFloat();
    cycle /= cl_bobcycle.GetFloat();

    if (cycle < cl_bobup.GetFloat())
    {
        cycle = M_PI * cycle / cl_bobup.GetFloat();
    }
    else
    {
        cycle = M_PI + M_PI*(cycle - cl_bobup.GetFloat()) / (1.0 - cl_bobup.GetFloat());
    }

    g_verticalBob = speed*0.005f;
    g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

    g_verticalBob = clamp(g_verticalBob, -7.0f, 4.0f);

    //Calculate the lateral bob
    cycle = bobtime - (int) (bobtime / cl_bobcycle.GetFloat() * 2)*cl_bobcycle.GetFloat() * 2;
    cycle /= cl_bobcycle.GetFloat() * 2;

    if (cycle < cl_bobup.GetFloat())
    {
        cycle = M_PI * cycle / cl_bobup.GetFloat();
    }
    else
    {
        cycle = M_PI + M_PI*(cycle - cl_bobup.GetFloat()) / (1.0 - cl_bobup.GetFloat());
    }

    g_lateralBob = speed*0.005f;
    g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
    g_lateralBob = clamp(g_lateralBob, -7.0f, 4.0f);

    //NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
    return 0.0f;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CWeaponBase::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
    Vector	forward, right;
    AngleVectors(angles, &forward, &right, nullptr);

    CalcViewmodelBob();

    // Apply bob, but scaled down to 40%
    VectorMA(origin, g_verticalBob * 0.4f, forward, origin);

    // Z bob a bit more
    origin[2] += g_verticalBob * 0.1f;

    // bob the angles
    angles[ROLL] += g_verticalBob * 0.5f;
    angles[PITCH] -= g_verticalBob * 0.4f;

    angles[YAW] -= g_lateralBob  * 0.3f;

    //	VectorMA( origin, g_lateralBob * 0.2f, right, origin );
}

#else

void CWeaponBase::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{

}

float CWeaponBase::CalcViewmodelBob(void)
{
    return 0.0f;
}

#endif

#ifndef CLIENT_DLL
bool CWeaponBase::PhysicsSplash(const Vector &centerPoint, const Vector &normal, float rawSpeed, float scaledSpeed)
{
    if (rawSpeed > 20)
    {

        float size = 4.0f;
        if (!IsPistol())
            size += 2.0f;

        // adjust splash size based on speed
        size += RemapValClamped(rawSpeed, 0, 400, 0, 3);

        CEffectData	data;
        data.m_vOrigin = centerPoint;
        data.m_vNormal = normal;
        data.m_flScale = random->RandomFloat(size, size + 1.0f);

        if (GetWaterType() & CONTENTS_SLIME)
        {
            data.m_fFlags |= FX_WATER_IN_SLIME;
        }

        DispatchEffect("gunshotsplash", data);

        return true;
    }

    return false;
}
#endif // !CLIENT_DLL
