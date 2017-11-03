//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Includes all the headers/declarations necessary to access the
//			engine interface
//
// $NoKeywords: $
//=============================================================================//

#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

// these stupid set of includes are required to use the cdll_int interface
#include "mathlib/vector.h"
//#include "wrect.h"
#define IN_BUTTONS_H

// engine interface
#include "cdll_int.h"
#include "eiface.h"
#include "icvar.h"
#include "tier2/tier2.h"

// engine interface singleton accessors
extern IVEngineClient *engine;
extern class IBik *bik;
extern class IEngineVGui *enginevguifuncs;
extern class IGameUIFuncs *gameuifuncs;
extern class IEngineSound *enginesound;
extern class IAchievementMgr *achievementmgr;

#endif // ENGINEINTERFACE_H
