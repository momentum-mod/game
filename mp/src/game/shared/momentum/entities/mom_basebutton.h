#ifndef _MOM_BASEBUTTON_H_
#define _MOM_BASEBUTTON_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basetoggle.h"
#include "mom_basedoor.h"
#include "mom_entityoutput.h"

#include "takedamageinfo.h"

#ifdef CLIENT_DLL
#define CBaseButton C_BaseButton
#else
#include "locksounds.h"
#include "spark.h"
#endif

#define SF_BUTTON_DONTMOVE				1
#define SF_ROTBUTTON_NOTSOLID			1
#define	SF_BUTTON_TOGGLE				32		// button stays pushed until reactivated
#define SF_BUTTON_TOUCH_ACTIVATES		256		// Button fires when touched.
#define SF_BUTTON_DAMAGE_ACTIVATES		512		// Button fires when damaged.
#define SF_BUTTON_USE_ACTIVATES			1024	// Button fires when used.
#define SF_BUTTON_LOCKED				2048	// Whether the button is initially locked.
#define	SF_BUTTON_SPARK_IF_OFF			4096	// button sparks in OFF state
#define	SF_BUTTON_JIGGLE_ON_USE_LOCKED	8192	// whether to jiggle if someone uses us when we're locked

enum BUTTON_CODE
{
	BUTTON_NOTHING,
	BUTTON_ACTIVATE,
	BUTTON_RETURN,
	BUTTON_PRESS
};

class CBaseButton : public CBaseToggle
{
public:
	DECLARE_CLASS(CBaseButton, CBaseToggle);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	CBaseButton() : m_bIsBhopBlock(false) {};

	virtual void Spawn(void);

	bool CreateVPhysics(void);

protected:
	void ButtonActivate(void);

	void ButtonTouch(::CBaseEntity *pOther);
	void TriggerAndWait(void);
	void ButtonReturn(void);
	void ButtonBackHome(void);
	void ButtonUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	bool OnUseLocked(CBaseEntity *pActivator);

	virtual void Lock(void);
	virtual void Unlock(void);
	virtual void Press(CBaseEntity *pActivator, BUTTON_CODE eCode);

	// Input handlers
	void InputLock(inputdata_t &inputdata);
	void InputUnlock(inputdata_t &inputdata);
	void InputPress(inputdata_t &inputdata);
	void InputPressIn(inputdata_t &inputdata);
	void InputPressOut(inputdata_t &inputdata);

	virtual int OnTakeDamage(const CTakeDamageInfo &info);

	BUTTON_CODE	ButtonResponseToTouch(void);

	Vector m_vecMoveDir;

	bool	m_fStayPushed;		// button stays pushed in until touched again?
	bool	m_fRotating;		// a rotating button?  default is a sliding button.

	byte	m_bLockedSound;		// ordinals from entity selection
	byte	m_bLockedSentence;
	byte	m_bUnlockedSound;
	byte	m_bUnlockedSentence;
	bool	m_bLocked;
	int		m_sounds;
	float	m_flUseLockedTime;		// Controls how often we fire the OnUseLocked output.
	int		m_nState;
	bool	m_bSolidBsp;

	string_t	m_sNoise;			// The actual WAV file name of the sound.

	COutputEvent m_OnDamaged;
	COutputEvent m_OnPressed;
	COutputEvent m_OnUseLocked;
	COutputEvent m_OnIn;
	COutputEvent m_OnOut;

public:
	bool	m_bIsBhopBlock;

#ifdef GAME_DLL // Server specific things
public:
	virtual int UpdateTransmitState();
	virtual void Precache(void);
	virtual bool KeyValue(const char *szKeyName, const char *szValue);
	virtual int	ObjectCaps(void);
	void ButtonSpark(void);
	int DrawDebugTextOverlays();

protected:
	locksound_t m_ls;			// door lock sounds
	DECLARE_DATADESC();
#endif
};

#ifdef GAME_DLL
extern string_t MakeButtonSound(int sound);
extern void PlayLockSounds(CBaseEntity *pEdict, locksound_t *pls, int flocked, int fbutton);
#endif
#endif