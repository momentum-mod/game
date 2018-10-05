//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Filters are outboard entities that hold a set of rules that other
//			entities can use to determine behaviors.
//			
//			For example, triggers can use an activator filter to determine who
//			activates them. NPCs and breakables can use a damage filter to
//			determine what can damage them.
//
//			Current filter criteria are:
//
//				Activator name
//				Activator class
//				Activator team
//				Damage type (for damage filters only)
//
//			More than one filter can be combined to create a more complex boolean
//			expression by using filter_multi.
//
//=============================================================================//

#ifndef FILTERS_H
#define FILTERS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basefilter.h"
#include "mom_entityoutput.h"

#endif // FILTERS_H
