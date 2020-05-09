//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef OPTIONS_SUB_KEYBOARD_H
#define OPTIONS_SUB_KEYBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlvector.h"
#include "tier1/utlsymbol.h"

#include <vgui_controls/Panel.h>
#include "vgui_controls/Frame.h"
#include "vgui_controls/PropertyPage.h"
class VControlsListPanel;

//-----------------------------------------------------------------------------
// Purpose: Keyboard Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubKeyboard : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubKeyboard, vgui::PropertyPage );

public:
	COptionsSubKeyboard(vgui::Panel *parent);

	virtual void	OnResetData();
	virtual void	OnKeyCodePressed( vgui::KeyCode code );
	virtual void	OnThink();

	// Trap row selection message
	MESSAGE_FUNC_INT( ItemSelected, "ItemSelected", itemID );

	VControlsListPanel* GetControlsList( void ) { return m_pKeyBindList; }

private:
    void Finish( ButtonCode_t code );

    virtual void OnCommand( const char *command );

	// Get column 0 action descriptions for all keys
    void ParseActionDescriptions( void );

    void BindKey(const char *key, const char *binding);
    void UnbindKey(const char *key);
    void FillInCurrentBindings( void );
    void ClearBindItems( void );
    void FillInDefaultBindings( void );
    void AddBinding( KeyValues *item, const char *keyname );
    void RemoveKeyFromBindItems( KeyValues *org_item, const char *key );

    KeyValues *GetItemForBinding( const char *binding );

private:
	void OpenKeyboardAdvancedDialog();
	vgui::DHANDLE<class COptionsSubKeyboardAdvancedDlg> m_OptionsSubKeyboardAdvancedDlg;
	virtual void OnKeyCodeTyped(vgui::KeyCode code);

	VControlsListPanel	*m_pKeyBindList;

	vgui::Button *m_pSetBindingButton;
	vgui::Button *m_pClearBindingButton;
};


//-----------------------------------------------------------------------------
// Purpose: advanced keyboard settings dialog
//-----------------------------------------------------------------------------
class COptionsSubKeyboardAdvancedDlg : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(COptionsSubKeyboardAdvancedDlg, vgui::Frame);

  public:
    COptionsSubKeyboardAdvancedDlg(vgui::VPANEL hParent);

    virtual void Activate();
    virtual void OnApplyData();
    virtual void OnCommand(const char *command);
    void OnKeyCodeTyped(vgui::KeyCode code);

  private:
    ConVarRef m_cvarConEnable, m_cvarFastSwitch;
};


#endif // OPTIONS_SUB_KEYBOARD_H