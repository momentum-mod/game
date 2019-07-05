//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#ifndef VIEWDEBUG_H
#define VIEWDEBUG_H

#if defined( _WIN32 )
#pragma once
#endif

class CViewSetup;

//-----------------------------------------------------------------------------
// Purpose: Implements the debugging elements of view rendering
//-----------------------------------------------------------------------------
class CDebugViewRender
{
	DECLARE_CLASS_NOBASE( CDebugViewRender );
public:
	// Draws all the debugging info
	static void	Draw3DDebuggingInfo( const CViewSetup &viewSetup );
	static void	Draw2DDebuggingInfo( const CViewSetup &viewSetup );
	static void GenerateOverdrawForTesting();
};

#endif // VIEWDEBUG_H
