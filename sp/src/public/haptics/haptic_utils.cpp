#include "cbase.h"

#ifdef CLIENT_DLL
	#include "tier3/tier3.h"
	#include "iviewrender.h"
	#include "inputsystem/iinputsystem.h"
	#include "vgui/IInputInternal.h"
	#include "c_basecombatweapon.h"
	#include "c_baseplayer.h"
	#include "haptics/ihaptics.h"
	#include "hud_macros.h"
	#include "prediction.h"
	#include "activitylist.h"
#ifdef TERROR
	#include "ClientTerrorPlayer.h"
#endif
extern vgui::IInputInternal *g_InputInternal;
#else
	#include "usermessages.h"
#endif

#include "haptics/haptic_utils.h"
#include "haptics/haptic_msgs.h"

#ifndef TERROR
#ifndef FCVAR_RELEASE
#define FCVAR_RELEASE 0
#endif
#endif

#ifdef CLIENT_DLL 
ConVar hap_HasDevice	( "hap_HasDevice", "0", FCVAR_USERINFO/*|FCVAR_HIDDEN*/, "falcon is connected" );
#define HAP_DEFAULT_DAMAGE_SCALE_GAME "1.0"
ConVar hap_damagescale_game("hap_damagescale_game", HAP_DEFAULT_DAMAGE_SCALE_GAME);
#undef HAP_DEFAULT_DAMAGE_SCALE_GAME

#endif

void HapticSendWeaponAnim(CBaseCombatWeapon* weapon, int iActivity)
{
	//ignore idle
	if(iActivity == ACT_VM_IDLE)
		return;
	
	#if defined( CLIENT_DLL )
	//if(hap_PrintEvents.GetBool())
	//	Msg("Client Activity :%s %s %s\n",weapon->GetName(),"Activities",VarArgs("%i",iActivity));
	if ( weapon->IsPredicted() )
		haptics->ProcessHapticWeaponActivity(weapon->GetName(),iActivity);
	#else
	if( !weapon->IsPredicted() && weapon->GetOwner() && weapon->GetOwner()->IsPlayer())
	{
		HapticMsg_SendWeaponAnim( ToBasePlayer(weapon->GetOwner()), iActivity );
	}
	#endif
}


void HapticSetDrag(CBasePlayer* pPlayer, float drag)
{
#ifdef CLIENT_DLL
	haptics->SetDrag(drag);
#else
	HapticMsg_SetDrag( pPlayer, drag );
#endif
}

#ifdef CLIENT_DLL
void HapticsHandleMsg_HapSetDrag( float drag )
{
	haptics->SetDrag(drag);
}
#endif

void HapticSetConstantForce(CBasePlayer* pPlayer, Vector force)
{
#ifdef CLIENT_DLL
	haptics->SetConstantForce(force);
#else
	HapticMsg_SetConstantForce( pPlayer, force );
#endif
}

#ifdef CLIENT_DLL

#ifndef HAPTICS_TEST_PREFIX
#define HAPTICS_TEST_PREFIX
#endif
static CSysModule *pFalconModule =0;
void ConnectHaptics(CreateInterfaceFn appFactory)
{
	bool success = false;
	// NVNT load haptics module
	pFalconModule = Sys_LoadModule( HAPTICS_TEST_PREFIX HAPTICS_DLL_NAME );
	if(pFalconModule)
	{
		CreateInterfaceFn factory = Sys_GetFactory( pFalconModule );
		if(factory)
		{
			haptics = reinterpret_cast< IHaptics* >( factory( HAPTICS_INTERFACE_VERSION, NULL ) );
			if(haptics && 
				haptics->Initialize(engine,
					view,
					g_InputInternal,
					gpGlobals,
					appFactory,
					g_pVGuiInput->GetIMEWindow(),
					filesystem,
					enginevgui,
					ActivityList_IndexForName,
					ActivityList_NameForIndex))
			{
				success = true;
				hap_HasDevice.SetValue(1);
			}
		}
	}
	if(!success)
	{
		Sys_UnloadModule(pFalconModule);
		pFalconModule = 0;
		haptics = new CHapticsStubbed;
	}

	if(haptics->HasDevice())
	{
		Assert( (void*)haptics == inputsystem->GetHapticsInterfaceAddress() );
	}
	HookHapticMessages();
}

void DisconnectHaptics()
{
	haptics->ShutdownHaptics();
	if(pFalconModule)
	{
		Sys_UnloadModule(pFalconModule);
		pFalconModule = 0;
	}else{
		// delete the stub.
		delete haptics;
	}
	haptics = NULL;
}
//Might be able to handle this better...
void HapticsHandleMsg_HapSetConst( Vector const &constant )
{
	//Msg("__MsgFunc_HapSetConst: %f %f %f\n",constant.x,constant.y,constant.z);
	haptics->SetConstantForce(constant);
	//C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
}


//Might be able to handle this better...
void HapticsHandleMsg_SPHapWeapEvent( int iActivity )
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon* weap = NULL;
	if(pPlayer)
		weap = pPlayer->GetActiveWeapon();
	if(weap)
		haptics->ProcessHapticEvent(4,"Weapons",weap->GetName(),"Activities",VarArgs("%i",iActivity));
}

void HapticsHandleMsg_HapPunch( QAngle const &angle )
{
	haptics->HapticsPunch(1,angle);

}
#ifdef TERROR
ConVar hap_zombie_damage_scale("hap_zombie_damage_scale", "0.25", FCVAR_RELEASE|FCVAR_NEVER_AS_STRING);
#endif
void HapticsHandleMsg_HapDmg( float pitch, float yaw, float damage, int damageType )
{
	if(!haptics->HasDevice())
		return;

#ifdef TERROR
	C_TerrorPlayer *pPlayer = C_TerrorPlayer::GetLocalTerrorPlayer();
#else
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
#endif

	if(pPlayer)
	{
		Vector damageDirection;

		damageDirection.x = cos(pitch*M_PI/180.0)*sin(yaw*M_PI/180.0);
		damageDirection.y = -sin(pitch*M_PI/180.0);
		damageDirection.z = -(cos(pitch*M_PI/180.0)*cos(yaw*M_PI/180.0));
#ifdef TERROR
		if(pPlayer->GetTeamNumber()==TEAM_ZOMBIE)
		{
			damageDirection *= hap_zombie_damage_scale.GetFloat();
		}
#endif

		haptics->ApplyDamageEffect(damage, damageType, damageDirection);
	}
}

ConVar hap_melee_scale("hap_melee_scale", "0.8", FCVAR_RELEASE|FCVAR_NEVER_AS_STRING);
void HapticsHandleMsg_HapMeleeContact()
{
	haptics->HapticsPunch(hap_melee_scale.GetFloat(), QAngle(0,0,0));
}
ConVar hap_noclip_avatar_scale("hap_noclip_avatar_scale", "0.10f", FCVAR_RELEASE|FCVAR_NEVER_AS_STRING);
void UpdateAvatarEffect(void)
{
	if(!haptics->HasDevice())
		return;

	Vector vel;
	Vector vvel;
	Vector evel;
	QAngle eye;
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return;

	eye = pPlayer->GetAbsAngles();

	vel = pPlayer->GetAbsVelocity();

	Vector PlayerVel = pPlayer->GetAbsVelocity();

	//Choreo vehicles use player avatar and don't produce their own velocity
	if( abs(vvel.Length()) == 0 )
	{
		vel = PlayerVel;
	}
	else
		vel = vvel;


	
	VectorYawRotate(vel, -90 -eye[YAW], vel );

	vel.y = -vel.y;
	vel.z = -vel.z;
	
	switch(pPlayer->GetMoveType()) {
		case MOVETYPE_NOCLIP:
			vel *= hap_noclip_avatar_scale.GetFloat();
			break;
		default:
			break;
	}

	haptics->UpdateAvatarVelocity(vel);
}

#endif


#ifndef CLIENT_DLL
void HapticsDamage(CBasePlayer* pPlayer, const CTakeDamageInfo &info)
{
#if !defined(CSTRIKE_DLL)
	if(!pPlayer->HasHaptics())
		return;// do not send to non haptic users.

	Vector DamageDirection(0,0,0);
	CBaseEntity *eInflictor = info.GetInflictor();
	// Pat: nuero toxix crash fix
	if(!eInflictor) {
		return;
	}
	// Player Data
	Vector playerPosition = pPlayer->GetLocalOrigin();
	Vector inflictorPosition = eInflictor->GetLocalOrigin();

	Vector posWithDir = playerPosition + (playerPosition - inflictorPosition);
	pPlayer->WorldToEntitySpace(posWithDir, &DamageDirection);
	QAngle dir(0,0,0);
	VectorAngles(DamageDirection, dir);
	float yawAngle = dir[YAW];
	float pitchAngle = dir[PITCH];

	int bitDamageType = info.GetDamageType();

	if(bitDamageType & DMG_FALL)
	{
		pitchAngle = ((float)-90.0); // coming from beneath
	}
	else if(bitDamageType & DMG_BURN && (bitDamageType & ~DMG_BURN)==0)
	{
		// just burn, use the z axis here.
		pitchAngle = 0.0;
	}
#ifdef TERROR
	else if(
		(bitDamageType & ( DMG_PARALYZE | DMG_NERVEGAS | DMG_POISON | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN ) ) && 
		(bitDamageType & ~( DMG_PARALYZE | DMG_NERVEGAS | DMG_POISON | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN ) )==0 )
	{
		// it is time based. and should not really do a punch.
		return;
	}
#endif
	
	float sendDamage = info.GetDamage();

	if(sendDamage>0.0f)
	{
		HapticMsg_HapDmg( pPlayer, pitchAngle, -yawAngle, sendDamage, bitDamageType );
	}
#endif
}

void HapticPunch(CBasePlayer* pPlayer, float x, float y, float z)
{
	HapticMsg_Punch( pPlayer, x, y, z );
}

void HapticMeleeContact(CBasePlayer* pPlayer)
{
	HapticMsg_MeleeContact( pPlayer );
}



#endif // NOT CLIENT_DLL

void HapticProcessSound(const char* soundname, int entIndex)
{
#ifdef CLIENT_DLL
	if (prediction->InPrediction() && prediction->IsFirstTimePredicted())
	{
		bool local = false;
		C_BaseEntity *ent = C_BaseEntity::Instance( entIndex );
		if(ent)
			local = (entIndex == -1  || ent == C_BasePlayer::GetLocalPlayer() || ent == C_BasePlayer::GetLocalPlayer()->GetActiveWeapon());

		haptics->ProcessHapticEvent(2,"Sounds",soundname);
	}
#endif
}
