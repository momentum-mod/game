//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements two types of doors: linear and rotating.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mom_basedoor.h"
#include "entitylist.h"
#include "physics.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include "physics_npc_solver.h"
#include "mom_basebutton.h"

#ifdef HL1_DLL
#include "mom_basefilter.h"
#endif

#ifdef CSTRIKE_DLL
#include "KeyValues.h"
#endif

#ifdef TF_DLL
#include "tf_gamerules.h"
#endif // TF_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
