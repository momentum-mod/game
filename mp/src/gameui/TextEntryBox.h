//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Matthew D. Campbell (matt@turtlerockstudios.com), 2003
#pragma once

#include <vgui_controls/QueryBox.h>

//--------------------------------------------------------------------------------------------------------------
/**
 *  Popup dialog with a text entry, extending the QueryBox, which extends the MessageBox
 */
class CTextEntryBox : public vgui::QueryBox
{
public:
    DECLARE_CLASS_SIMPLE(CTextEntryBox, vgui::QueryBox);

	CTextEntryBox(const char *title, const char *labelText, const char *entryText, bool isCvar, Panel *parent = NULL);

	virtual ~CTextEntryBox();
 
	virtual void PerformLayout();						///< Layout override to position the label and text entry
	virtual void ShowWindow(Frame *pFrameOver);	///< Show window override to give focus to text entry

protected:
    vgui::CvarTextEntry	*m_pCvarEntry;
	vgui::TextEntry	*m_pEntry;

	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	void OnCommand( const char *command);			///< Handle button presses
};