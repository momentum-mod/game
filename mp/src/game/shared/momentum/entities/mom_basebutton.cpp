#include "cbase.h"
#include "mom_basebutton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(func_button, CBaseButton);

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CBaseButton) // MOM_TODO: Add _NO_BASE stuff to predict here
END_PREDICTION_DATA();

#undef CBaseButton // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_BaseButton, DT_BaseButton, CBaseButton) // MOM_TODO: Add _NOBASE and get things working
	RecvPropVector(RECVINFO(m_vecPosition1)),
	RecvPropVector(RECVINFO(m_vecPosition2)),
END_RECV_TABLE();
#define CBaseButton C_BaseButton // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CBaseButton, DT_BaseButton) // MOM_TODO: Add _NOBASE and get things working
	SendPropVector(SENDINFO(m_vecPosition1)),
	SendPropVector(SENDINFO(m_vecPosition2)),
END_SEND_TABLE();

BEGIN_DATADESC(CBaseButton)
	DEFINE_KEYFIELD(m_vecMoveDir, FIELD_VECTOR, "movedir"),
	DEFINE_FIELD(m_fStayPushed, FIELD_BOOLEAN),
	DEFINE_FIELD(m_fRotating, FIELD_BOOLEAN),

	DEFINE_FIELD(m_bLockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(m_bUnlockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(m_bUnlockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(m_bLocked, FIELD_BOOLEAN),
	DEFINE_FIELD(m_sNoise, FIELD_SOUNDNAME),
	DEFINE_FIELD(m_flUseLockedTime, FIELD_TIME),
	DEFINE_FIELD(m_bSolidBsp, FIELD_BOOLEAN),

	DEFINE_KEYFIELD(m_sounds, FIELD_INTEGER, "sounds"),

	//	DEFINE_FIELD( m_ls, FIELD_SOUNDNAME ),   // This is restored in Precache()
	//  DEFINE_FIELD( m_nState, FIELD_INTEGER ),

	// Function Pointers
	DEFINE_FUNCTION(ButtonTouch),
	DEFINE_FUNCTION(ButtonSpark),
	DEFINE_FUNCTION(TriggerAndWait),
	DEFINE_FUNCTION(ButtonReturn),
	DEFINE_FUNCTION(ButtonBackHome),
	DEFINE_FUNCTION(ButtonUse),

	// Inputs
	DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock),
	DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock),
	DEFINE_INPUTFUNC(FIELD_VOID, "Press", InputPress),
	DEFINE_INPUTFUNC(FIELD_VOID, "PressIn", InputPressIn),
	DEFINE_INPUTFUNC(FIELD_VOID, "PressOut", InputPressOut),

	// Outputs
	DEFINE_OUTPUT(m_OnDamaged, "OnDamaged"),
	DEFINE_OUTPUT(m_OnPressed, "OnPressed"),
	DEFINE_OUTPUT(m_OnUseLocked, "OnUseLocked"),
	DEFINE_OUTPUT(m_OnIn, "OnIn"),
	DEFINE_OUTPUT(m_OnOut, "OnOut"),
END_DATADESC();
#endif

//-----------------------------------------------------------------------------
// Purpose: play door or button locked or unlocked sounds. 
//			NOTE: this routine is shared by doors and buttons
// Input  : pEdict - 
//			pls - 
//			flocked - if true, play 'door is locked' sound, otherwise play 'door
//				is unlocked' sound.
//			fbutton - 
//-----------------------------------------------------------------------------
void PlayLockSounds(CBaseEntity *pEdict, locksound_t *pls, int flocked, int fbutton)
{
	CBaseDoor *pDoor = dynamic_cast<CBaseDoor*>(pEdict);
	// Dynamic_cast is more expensive than a normal cast, so we should only do it if we need it.
	// As pDoor will be nullptr if it's not a Door, then it has to be a Button, and that is the only moment when
	// we need to dynamic_cast pEdict into button. This way we save a dynamic_cast most of the times (Most blocks are doors)
	CBaseButton *pButton = pDoor ? nullptr : dynamic_cast<CBaseButton*>(pEdict);
	bool isMomentumBlock = pDoor ? pDoor->m_bIsBhopBlock : (pButton ? pButton->m_bIsBhopBlock : false);
	bool shouldPlayBhopSound = ConVarRef("mom_bhop_playblocksound").GetBool();
	if (pEdict->HasSpawnFlags(SF_DOOR_SILENT) || (isMomentumBlock && !shouldPlayBhopSound))
	{
		return;
	}
	float flsoundwait = (fbutton) ? BUTTON_SOUNDWAIT : DOOR_SOUNDWAIT;

	if (flocked)
	{
		int		fplaysound = (pls->sLockedSound != NULL_STRING && gpGlobals->curtime > pls->flwaitSound);
		int		fplaysentence = (pls->sLockedSentence != NULL_STRING && !pls->bEOFLocked && gpGlobals->curtime > pls->flwaitSentence);
		float	fvol = (fplaysound && fplaysentence) ? 0.25f : 1.0f;

		// if there is a locked sound, and we've debounced, play sound
		if (fplaysound)
		{
			// play 'door locked' sound
			CPASAttenuationFilter filter(pEdict);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_ITEM;
			ep.m_pSoundName = (char*)STRING(pls->sLockedSound);
			ep.m_flVolume = fvol;
			ep.m_SoundLevel = SNDLVL_NORM;

			CBaseEntity::EmitSound(filter, pEdict->entindex(), ep);
			pls->flwaitSound = gpGlobals->curtime + flsoundwait;
		}

		// if there is a sentence, we've not played all in list, and we've debounced, play sound
		if (fplaysentence)
		{
			// play next 'door locked' sentence in group
			int iprev = pls->iLockedSentence;

			pls->iLockedSentence = SENTENCEG_PlaySequentialSz(pEdict->edict(),
				STRING(pls->sLockedSentence),
				0.85f,
				SNDLVL_NORM,
				0,
				100,
				pls->iLockedSentence,
				FALSE);
			pls->iUnlockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFLocked = (iprev == pls->iLockedSentence);

			pls->flwaitSentence = gpGlobals->curtime + DOOR_SENTENCEWAIT;
		}
	}
	else
	{
		// UNLOCKED SOUND

		int fplaysound = (pls->sUnlockedSound != NULL_STRING && gpGlobals->curtime > pls->flwaitSound);
		int fplaysentence = (pls->sUnlockedSentence != NULL_STRING && !pls->bEOFUnlocked && gpGlobals->curtime > pls->flwaitSentence);
		float fvol;

		// if playing both sentence and sound, lower sound volume so we hear sentence
		fvol = (fplaysound && fplaysentence) ? 0.25f : 1.0f;

		// play 'door unlocked' sound if set
		if (fplaysound)
		{
			CPASAttenuationFilter filter(pEdict);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_ITEM;
			ep.m_pSoundName = (char*)STRING(pls->sUnlockedSound);
			ep.m_flVolume = fvol;
			ep.m_SoundLevel = SNDLVL_NORM;

			CBaseEntity::EmitSound(filter, pEdict->entindex(), ep);
			pls->flwaitSound = gpGlobals->curtime + flsoundwait;
		}

		// play next 'door unlocked' sentence in group
		if (fplaysentence)
		{
			int iprev = pls->iUnlockedSentence;

			pls->iUnlockedSentence = SENTENCEG_PlaySequentialSz(pEdict->edict(), STRING(pls->sUnlockedSentence),
				0.85, SNDLVL_NORM, 0, 100, pls->iUnlockedSentence, FALSE);
			pls->iLockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFUnlocked = (iprev == pls->iUnlockedSentence);
			pls->flwaitSentence = gpGlobals->curtime + DOOR_SENTENCEWAIT;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Button sound table.
//			Also used by CBaseDoor to get 'touched' door lock/unlock sounds
// Input  : sound - index of sound to look up.
// Output : Returns a pointer to the corresponding sound file.
//-----------------------------------------------------------------------------
string_t MakeButtonSound(int sound)
{
	char tmp[1024];
	Q_snprintf(tmp, sizeof(tmp), "Buttons.snd%d", sound);
	return AllocPooledString(tmp);
}

void CBaseButton::Spawn(void)
{
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	if (m_sounds)
	{
		m_sNoise = MakeButtonSound(m_sounds);
		PrecacheScriptSound(m_sNoise.ToCStr());
	}
	else
	{
		m_sNoise = NULL_STRING;
	}

	Precache();

	if (HasSpawnFlags(SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		SetThink(&CBaseButton::ButtonSpark);
		SetNextThink(gpGlobals->curtime + 0.5f);// no hurry, make sure everything else spawns
	}

	// Convert movedir from angles to a vector
	QAngle angMoveDir = QAngle(m_vecMoveDir.x, m_vecMoveDir.y, m_vecMoveDir.z);
	AngleVectors(angMoveDir, &m_vecMoveDir);

	SetMoveType(MOVETYPE_PUSH);
	SetSolid(SOLID_BSP);
	SetModel(STRING(GetModelName()));

	if (m_flSpeed == 0)
	{
		m_flSpeed = 40;
	}

	m_takedamage = DAMAGE_YES;

	if (m_flWait == 0)
	{
		m_flWait = 1;
	}

	if (m_flLip == 0)
	{
		m_flLip = 4;
	}

	m_toggle_state = TS_AT_BOTTOM;
	m_vecPosition1 = GetLocalOrigin();

	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	Vector vecButtonOBB = CollisionProp()->OBBSize();
	vecButtonOBB -= Vector(2, 2, 2);
	m_vecPosition2 = m_vecPosition1 + (m_vecMoveDir * (DotProductAbs(m_vecMoveDir, vecButtonOBB) - m_flLip));

	// Is this a non-moving button?
	if (((m_vecPosition2 - m_vecPosition1).Length() < 1) || HasSpawnFlags(SF_BUTTON_DONTMOVE))
	{
		m_vecPosition2 = m_vecPosition1;
	}

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = FALSE;

	if (HasSpawnFlags(SF_BUTTON_LOCKED))
	{
		m_bLocked = true;
	}

	//
	// If using activates the button, set its use function.
	//
	if (HasSpawnFlags(SF_BUTTON_USE_ACTIVATES))
	{
		SetUse(&CBaseButton::ButtonUse);
	}
	else
	{
		SetUse(NULL);
	}

	//
	// If touching activates the button, set its touch function.
	//
	if (HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES))
	{
		SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		SetTouch(NULL);
	}

	CreateVPhysics();
}

void CBaseButton::Precache(void)
{
	// get door button sounds, for doors which require buttons to open
	if (m_bLockedSound)
	{
		m_ls.sLockedSound = MakeButtonSound((int)m_bLockedSound);
		PrecacheScriptSound(m_ls.sLockedSound.ToCStr());
	}

	if (m_bUnlockedSound)
	{
		m_ls.sUnlockedSound = MakeButtonSound((int)m_bUnlockedSound);
		PrecacheScriptSound(m_ls.sUnlockedSound.ToCStr());
	}

	// get sentence group names, for doors which are directly 'touched' to open

	switch (m_bLockedSentence)
	{
	case 1: m_ls.sLockedSentence = MAKE_STRING("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = MAKE_STRING("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = MAKE_STRING("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = MAKE_STRING("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = MAKE_STRING("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = MAKE_STRING("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = MAKE_STRING("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = MAKE_STRING("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = MAKE_STRING("NG"); break; // broken door

	default: m_ls.sLockedSentence = NULL_STRING; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = MAKE_STRING("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = MAKE_STRING("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = MAKE_STRING("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = MAKE_STRING("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = MAKE_STRING("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = MAKE_STRING("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = MAKE_STRING("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = MAKE_STRING("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = NULL_STRING; break;
	}

	if (m_sNoise != NULL_STRING)
	{
		PrecacheScriptSound(STRING(m_sNoise));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cache user-entity-field values until spawn is called.
// Input  : szKeyName - 
//			szValue - 
// Output : Returns true if handled, false if not.
//-----------------------------------------------------------------------------
bool CBaseButton::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "locked_sound"))
	{
		m_bLockedSound = atof(szValue);
	}
	else if (FStrEq(szKeyName, "locked_sentence"))
	{
		m_bLockedSentence = atof(szValue);
	}
	else if (FStrEq(szKeyName, "unlocked_sound"))
	{
		m_bUnlockedSound = atof(szValue);
	}
	else if (FStrEq(szKeyName, "unlocked_sentence"))
	{
		m_bUnlockedSentence = atof(szValue);
	}
	else
	{
		return BaseClass::KeyValue(szKeyName, szValue);
	}

	return true;
}

bool CBaseButton::CreateVPhysics(void)
{
	VPhysicsInitShadow(false, false);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Starts the button moving "in/up".
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBaseButton::ButtonActivate(void)
{
	if (m_sNoise != NULL_STRING)
	{
		CPASAttenuationFilter filter(this);

		EmitSound_t ep;
		ep.m_nChannel = CHAN_VOICE;
		ep.m_pSoundName = (char*)STRING(m_sNoise);
		ep.m_flVolume = 1;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator) || m_bLocked)
	{
		// button is locked, play locked sound
		PlayLockSounds(this, &m_ls, TRUE, TRUE);
		return;
	}
	else
	{
		// button is unlocked, play unlocked sound
		PlayLockSounds(this, &m_ls, FALSE, TRUE);
	}

	ASSERT(m_toggle_state == TS_AT_BOTTOM);
	m_toggle_state = TS_GOING_UP;

	SetMoveDone(&CBaseButton::TriggerAndWait);
	if (!m_fRotating)
		LinearMove(m_vecPosition2, m_flSpeed);
	else
		AngularMove(m_vecAngle2, m_flSpeed);
}

//-----------------------------------------------------------------------------
// Purpose: Touch function that activates the button if it responds to touch.
// Input  : pOther - The entity that touched us.
//-----------------------------------------------------------------------------
void CBaseButton::ButtonTouch(CBaseEntity *pOther)
{
	// Ignore touches by anything but players
	if (!pOther->IsPlayer())
		return;

	m_hActivator = pOther;

	BUTTON_CODE code = ButtonResponseToTouch();

	if (code == BUTTON_NOTHING)
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther) || m_bLocked)
	{
		// play button locked sound
		PlayLockSounds(this, &m_ls, TRUE, TRUE);
		return;
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch(NULL);

	if (code == BUTTON_RETURN)
	{
		if (m_sNoise != NULL_STRING)
		{
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_VOICE;
			ep.m_pSoundName = (char*)STRING(m_sNoise);
			ep.m_flVolume = 1;
			ep.m_SoundLevel = SNDLVL_NORM;

			EmitSound(filter, entindex(), ep);
		}

		m_OnPressed.FireOutput(m_hActivator, this);
		ButtonReturn();
	}
	else
	{
		// code == BUTTON_ACTIVATE
		m_OnPressed.FireOutput(m_hActivator, this);
		ButtonActivate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function that emits sparks at random intervals.
//-----------------------------------------------------------------------------
void CBaseButton::ButtonSpark(void)
{
	SetThink(&CBaseButton::ButtonSpark);
	SetNextThink(gpGlobals->curtime + 0.1 + random->RandomFloat(0, 1.5));// spark again at random interval

	DoSpark(this, WorldSpaceCenter(), 1, 1, true, vec3_origin);
}

//-----------------------------------------------------------------------------
// Purpose: Button has reached the "pressed/top" position. Fire its OnIn output,
//			and pause before returning to "unpressed/bottom".
//-----------------------------------------------------------------------------
void CBaseButton::TriggerAndWait(void)
{
	ASSERT(m_toggle_state == TS_GOING_UP);

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator) || m_bLocked)
	{
		return;
	}

	m_toggle_state = TS_AT_TOP;

	//
	// Re-instate touches if the button is of the toggle variety.
	//
	if (m_fStayPushed || HasSpawnFlags(SF_BUTTON_TOGGLE))
	{
		if (HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES))
		{
			SetTouch(&CBaseButton::ButtonTouch);
		}
		else
		{
			// BUGBUG: ALL buttons no longer respond to touch
			SetTouch(NULL);
		}
	}

	//
	// If button automatically comes back out, start it moving out.
	//
	else
	{
		SetNextThink(gpGlobals->curtime + m_flWait);
		SetThink(&CBaseButton::ButtonReturn);
	}

	m_nState = 1;			// use alternate textures

	m_OnIn.FireOutput(m_hActivator, this);
}


//-----------------------------------------------------------------------------
// Purpose: Starts the button moving "out/down".
//-----------------------------------------------------------------------------
void CBaseButton::ButtonReturn(void)
{
	Assert(m_toggle_state == TS_AT_TOP);
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseButton::ButtonBackHome);
	if (!m_fRotating)
		LinearMove(m_vecPosition1, m_flSpeed);
	else
		AngularMove(m_vecAngle1, m_flSpeed);

	m_nState = 0;			// use normal textures
}


//-----------------------------------------------------------------------------
// Purpose: Button has returned to the "unpressed/bottom" position. Fire its
//			OnOut output and stop moving.
//-----------------------------------------------------------------------------
void CBaseButton::ButtonBackHome(void)
{
	Assert(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;

	m_OnOut.FireOutput(m_hActivator, this);

	//
	// Re-instate touch method, movement cycle is complete.
	//
	if (HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES))
	{
		SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		// BUGBUG: ALL buttons no longer respond to touch
		SetTouch(NULL);
	}

	// reset think for a sparking button
	if (HasSpawnFlags(SF_BUTTON_SPARK_IF_OFF))
	{
		SetThink(&CBaseButton::ButtonSpark);
		SetNextThink(gpGlobals->curtime + 0.5f);// no hurry
	}
}

//-----------------------------------------------------------------------------
// Purpose: Use function that starts the button moving.
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CBaseButton::ButtonUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	// UNDONE: Should this use ButtonResponseToTouch() too?
	if (m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN)
		return;

	if (m_bLocked)
	{
		OnUseLocked(pActivator);
		return;
	}

	m_hActivator = pActivator;

	if (m_toggle_state == TS_AT_TOP)
	{
		//
		// If it's a toggle button it can return now. Otherwise, it will either
		// return on its own or will stay pressed indefinitely.
		//
		if (HasSpawnFlags(SF_BUTTON_TOGGLE))
		{
			if (m_sNoise != NULL_STRING)
			{
				CPASAttenuationFilter filter(this);

				EmitSound_t ep;
				ep.m_nChannel = CHAN_VOICE;
				ep.m_pSoundName = (char*)STRING(m_sNoise);
				ep.m_flVolume = 1;
				ep.m_SoundLevel = SNDLVL_NORM;

				EmitSound(filter, entindex(), ep);
			}

			m_OnPressed.FireOutput(m_hActivator, this);
			ButtonReturn();
		}
	}
	else
	{
		m_OnPressed.FireOutput(m_hActivator, this);
		ButtonActivate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when someone uses us whilst we are locked.
//-----------------------------------------------------------------------------
bool CBaseButton::OnUseLocked(CBaseEntity *pActivator)
{
	PlayLockSounds(this, &m_ls, TRUE, TRUE);

	if (gpGlobals->curtime > m_flUseLockedTime)
	{
		m_OnUseLocked.FireOutput(pActivator, this);
		m_flUseLockedTime = gpGlobals->curtime + 0.5;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Locks the button. If locked, the button will play the locked sound
//			when the player tries to use it.
//-----------------------------------------------------------------------------
void CBaseButton::Lock(void)
{
	m_bLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: Unlocks the button, making it able to be pressed again.
//-----------------------------------------------------------------------------
void CBaseButton::Unlock(void)
{
	m_bLocked = false;
}

//-----------------------------------------------------------------------------
// Presses or unpresses the button.
//-----------------------------------------------------------------------------
void CBaseButton::Press(CBaseEntity *pActivator, BUTTON_CODE eCode)
{
	if ((eCode == BUTTON_PRESS) && (m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN))
	{
		return;
	}

	if ((eCode == BUTTON_ACTIVATE) && (m_toggle_state == TS_GOING_UP || m_toggle_state == TS_AT_TOP))
	{
		return;
	}

	if ((eCode == BUTTON_RETURN) && (m_toggle_state == TS_GOING_DOWN || m_toggle_state == TS_AT_BOTTOM))
	{
		return;
	}

	// FIXME: consolidate all the button press code into one place!
	if (m_bLocked)
	{
		// play button locked sound
		PlayLockSounds(this, &m_ls, TRUE, TRUE);
		return;
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch(NULL);

	if (((eCode == BUTTON_PRESS) && (m_toggle_state == TS_AT_TOP)) ||
		((eCode == BUTTON_RETURN) && (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP)))
	{
		if (m_sNoise != NULL_STRING)
		{
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_VOICE;
			ep.m_pSoundName = (char*)STRING(m_sNoise);
			ep.m_flVolume = 1;
			ep.m_SoundLevel = SNDLVL_NORM;

			EmitSound(filter, entindex(), ep);
		}

		m_OnPressed.FireOutput(pActivator, this);
		ButtonReturn();
	}
	else if ((eCode == BUTTON_PRESS) ||
		((eCode == BUTTON_ACTIVATE) && (m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN)))
	{
		m_OnPressed.FireOutput(pActivator, this);
		ButtonActivate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Locks the button. If locked, the button will play the locked sound
//			when the player tries to use it.
//-----------------------------------------------------------------------------
void CBaseButton::InputLock( inputdata_t &inputdata )
{
	Lock();
}

//-----------------------------------------------------------------------------
// Purpose: Unlocks the button, making it able to be pressed again.
//-----------------------------------------------------------------------------
void CBaseButton::InputUnlock( inputdata_t &inputdata )
{
	Unlock();
}

//-----------------------------------------------------------------------------
// Presses the button.
//-----------------------------------------------------------------------------
void CBaseButton::InputPress(inputdata_t &inputdata)
{
	Press(inputdata.pActivator, BUTTON_PRESS);
}

//-----------------------------------------------------------------------------
// Presses the button, sending it to the top/pressed position.
//-----------------------------------------------------------------------------
void CBaseButton::InputPressIn(inputdata_t &inputdata)
{
	Press(inputdata.pActivator, BUTTON_ACTIVATE);
}

//-----------------------------------------------------------------------------
// Unpresses the button, sending it to the unpressed/bottom position.
//-----------------------------------------------------------------------------
void CBaseButton::InputPressOut(inputdata_t &inputdata)
{
	Press(inputdata.pActivator, BUTTON_RETURN);
}

//-----------------------------------------------------------------------------
// Purpose: We have been damaged. Possibly activate, depending on our flags.
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : 
//-----------------------------------------------------------------------------
int CBaseButton::OnTakeDamage(const CTakeDamageInfo &info)
{
	// dvsents2: remove obselete health keyvalue from func_button
	if (!HasSpawnFlags(SF_BUTTON_DAMAGE_ACTIVATES) && (m_iHealth == 0))
	{
		return(0);
	}

	BUTTON_CODE code = ButtonResponseToTouch();

	if (code == BUTTON_NOTHING)
		return 0;

	m_hActivator = info.GetAttacker();

	// dvsents2: why would activator be NULL here?
	if (m_hActivator == NULL)
		return 0;

	m_OnDamaged.FireOutput(m_hActivator, this);

	if (m_bLocked)
	{
		return(0);
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch(NULL);

	if (code == BUTTON_RETURN)
	{
		if (m_sNoise != NULL_STRING)
		{
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_VOICE;
			ep.m_pSoundName = (char*)STRING(m_sNoise);
			ep.m_flVolume = 1;
			ep.m_SoundLevel = SNDLVL_NORM;

			EmitSound(filter, entindex(), ep);
		}

		m_OnPressed.FireOutput(m_hActivator, this);
		ButtonReturn();
	}
	else
	{
		// code == BUTTON_ACTIVATE
		m_OnPressed.FireOutput(m_hActivator, this);
		ButtonActivate();
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a code indicating how the button should respond to being touched.
// Output : Returns one of the following:
//				BUTTON_NOTHING - do nothing
//				BUTTON_RETURN - 
//				BUTTON_ACTIVATE - act as if pressed
//-----------------------------------------------------------------------------
BUTTON_CODE CBaseButton::ButtonResponseToTouch( void )
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	if (m_toggle_state == TS_GOING_UP ||
		m_toggle_state == TS_GOING_DOWN ||
		(m_toggle_state == TS_AT_TOP && !m_fStayPushed && !HasSpawnFlags(SF_BUTTON_TOGGLE) ) )
		return BUTTON_NOTHING;

	if (m_toggle_state == TS_AT_TOP)
	{
		if ( HasSpawnFlags(SF_BUTTON_TOGGLE) && !m_fStayPushed)
		{
			return BUTTON_RETURN;
		}
	}
	else
		return BUTTON_ACTIVATE;

	return BUTTON_NOTHING;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Enables or disables the use capability based on our spawnflags.
//-----------------------------------------------------------------------------
int	CBaseButton::ObjectCaps(void)
{
	return((BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) |
		(HasSpawnFlags(SF_BUTTON_USE_ACTIVATES) ? (FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS) : 0));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CBaseButton::DrawDebugTextOverlays()
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		static const char *pszStates[] =
		{
			"Pressed",
			"Unpressed",
			"Pressing...",
			"Unpressing...",
			"<UNKNOWN STATE>",
		};

		char tempstr[255];

		int nState = m_toggle_state;
		if ((nState < 0) || (nState > 3))
		{
			nState = 4;
		}

		Q_snprintf(tempstr, sizeof(tempstr), "State: %s", pszStates[nState]);
		EntityText(text_offset, tempstr, 0);
		text_offset++;

		Q_snprintf(tempstr, sizeof(tempstr), "%s", m_bLocked ? "Locked" : "Unlocked");
		EntityText(text_offset, tempstr, 0);
		text_offset++;
	}
	return text_offset;
}

#endif