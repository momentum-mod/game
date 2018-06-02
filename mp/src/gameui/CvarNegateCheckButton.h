//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CVARNEGATECHECKBUTTON_H
#define CVARNEGATECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/CheckButton.h>

class CvarNegateCheckButton : public vgui::CheckButton
{
	DECLARE_CLASS_SIMPLE( CvarNegateCheckButton, vgui::CheckButton );

public:
	CvarNegateCheckButton( vgui::Panel *parent, const char *panelName, const char *text, 
		char const *cvarname );
	~CvarNegateCheckButton();

	virtual void	SetSelected( bool state );
	virtual void	Paint();

	void			Reset();
	void			ApplyChanges();
	bool			HasBeenModified();

private:
	MESSAGE_FUNC( OnButtonChecked, "CheckButtonChecked" );

    ConVarRef m_cvarRef;
	bool			m_bStartState;
};

#endif // CVARNEGATECHECKBUTTON_H
