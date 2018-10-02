#ifndef _C_MOM_BUTTONS_H_
#define _C_MOM_BUTTONS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_mom_basetoggle.h"

#define SF_BUTTON_DONTMOVE				1
#define SF_ROTBUTTON_NOTSOLID			1
#define	SF_BUTTON_TOGGLE				32		// button stays pushed until reactivated
#define SF_BUTTON_TOUCH_ACTIVATES		256		// Button fires when touched.
#define SF_BUTTON_DAMAGE_ACTIVATES		512		// Button fires when damaged.
#define SF_BUTTON_USE_ACTIVATES			1024	// Button fires when used.
#define SF_BUTTON_LOCKED				2048	// Whether the button is initially locked.
#define	SF_BUTTON_SPARK_IF_OFF			4096	// button sparks in OFF state
#define	SF_BUTTON_JIGGLE_ON_USE_LOCKED	8192	// whether to jiggle if someone uses us when we're locked

class C_BaseButton : public C_BaseToggle
{
	DECLARE_CLASS(C_BaseButton, C_BaseToggle);
	DECLARE_CLIENTCLASS();

public:
	C_BaseButton() : m_bIsBhopBlock(false) {};

	bool m_bIsBhopBlock;
};

#endif