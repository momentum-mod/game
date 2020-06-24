//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#define PROTECTED_THINGS_DISABLE

#include "vgui/Cursor.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/IPanel.h"
#include "KeyValues.h"

#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/MenuItem.h"
#include "vgui_controls/TextImage.h"

#include <ctype.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

namespace vgui
{
ComboBoxButton::ComboBoxButton(ComboBox *parent, const char *panelName, const char *text) : Button(parent, panelName, text, parent, "ButtonClicked")
{
	SetButtonActivationType(ACTIVATE_ONPRESSED);
}

void ComboBoxButton::ApplySchemeSettings(IScheme *pScheme)
{
	Button::ApplySchemeSettings(pScheme);
	
	SetFont(GetSchemeFont(pScheme, nullptr, "ComboBoxButton.Font", "Marlett"));
	SetContentAlignment(a_center);
	SetDefaultBorder(pScheme->GetBorder("ScrollBarButtonBorder"));
	
	// arrow changes color but the background doesnt.
	const auto cBgColor = GetSchemeColor("ComboBoxButton.BgColor", pScheme);
	SetDefaultColor(GetSchemeColor("ComboBoxButton.ArrowColor", pScheme), cBgColor);
	SetArmedColor(GetSchemeColor("ComboBoxButton.ArmedArrowColor", pScheme), cBgColor);
	SetDepressedColor(GetSchemeColor("ComboBoxButton.ArmedArrowColor", pScheme), cBgColor);
	m_DisabledBgColor = GetSchemeColor("ComboBoxButton.DisabledBgColor", pScheme);
}

IBorder * ComboBoxButton::GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
{
	return NULL;
	//		return Button::GetBorder(depressed, armed, selected, keyfocus);
}

//-----------------------------------------------------------------------------
// Purpose: Dim the arrow on the button when exiting the box
//			only if the menu is closed, so let the parent handle this.
//-----------------------------------------------------------------------------
void ComboBoxButton::OnCursorExited()
{
	// want the arrow to go grey when we exit the box if the menu is not open
	CallParentFunction(new KeyValues("CursorExited"));
}

} // namespace vgui

vgui::Panel *ComboBox_Factory()
{
	return new ComboBox( NULL, NULL, 5, true );
}
DECLARE_BUILD_FACTORY_CUSTOM( ComboBox, ComboBox_Factory );

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  : parent - parent class
//			panelName
//			numLines - number of lines in dropdown menu
//			allowEdit - whether combobox is editable or not
//-----------------------------------------------------------------------------
ComboBox::ComboBox(Panel *parent, const char *panelName, int numLines, bool allowEdit ) : TextEntry(parent, panelName)
{
	SetEditable(allowEdit);
	SetHorizontalScrolling(false); // do not scroll, always Start at the beginning of the text.

	// create the drop-down menu
	m_pDropDown = new Menu(this, NULL);
	m_pDropDown->AddActionSignalTarget(this);
	m_pDropDown->SetTypeAheadMode( Menu::TYPE_AHEAD_MODE );

	// button to Activate menu
	m_pButton = new ComboBoxButton(this, "Button", "u");

	SetNumberOfEditLines(numLines);

	m_bHighlight = false;
	m_iDirection = Menu::DOWN;
	m_iOpenOffsetY = 0;
	m_bPreventTextChangeMessage = false;
	m_szBorderOverride[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
ComboBox::~ComboBox()
{
}

//-----------------------------------------------------------------------------
// Purpose: Set the number of items in the dropdown menu.
// Input  : numLines -  number of items in dropdown menu
//-----------------------------------------------------------------------------
void ComboBox::SetNumberOfEditLines( int numLines )
{
	m_pDropDown->SetNumberOfVisibleItems( numLines );
}

//-----------------------------------------------------------------------------
// Purpose: Add an item to the drop down
// Input  : char *itemText - name of dropdown menu item
//-----------------------------------------------------------------------------
int ComboBox::AddItem(const char *itemText, const KeyValues *userData)
{
	// when the menu item is selected it will send the custom message "SetText"
	return m_pDropDown->AddMenuItem( itemText, new KeyValues("SetText", "text", itemText), this, userData );
}


//-----------------------------------------------------------------------------
// Purpose: Add an item to the drop down
// Input  : char *itemText - name of dropdown menu item
//-----------------------------------------------------------------------------
int ComboBox::AddItem(const wchar_t *itemText, const KeyValues *userData)
{
	// add the element to the menu
	// when the menu item is selected it will send the custom message "SetText"
	KeyValues *kv = new KeyValues("SetText");
	kv->SetWString("text", itemText);
	// get an ansi version for the menuitem name
	char ansi[128];
	g_pVGuiLocalize->ConvertUnicodeToANSI(itemText, ansi, sizeof(ansi));
	return m_pDropDown->AddMenuItem(ansi, kv, this, userData);
}


//-----------------------------------------------------------------------------
// Removes a single item
//-----------------------------------------------------------------------------
void ComboBox::DeleteItem( int itemID )
{
	if ( !m_pDropDown->IsValidMenuID(itemID))
		return;

	m_pDropDown->DeleteItem( itemID );
}


//-----------------------------------------------------------------------------
// Purpose: Updates a current item to the drop down
// Input  : char *itemText - name of dropdown menu item
//-----------------------------------------------------------------------------
bool ComboBox::UpdateItem(int itemID, const char *itemText, const KeyValues *userData)
{
	if ( !m_pDropDown->IsValidMenuID(itemID))
		return false;

	// when the menu item is selected it will send the custom message "SetText"
	m_pDropDown->UpdateMenuItem(itemID, itemText, new KeyValues("SetText", "text", itemText), userData);
	InvalidateLayout();
	return true;
}
//-----------------------------------------------------------------------------
// Purpose: Updates a current item to the drop down
// Input  : wchar_t *itemText - name of dropdown menu item
//-----------------------------------------------------------------------------
bool ComboBox::UpdateItem(int itemID, const wchar_t *itemText, const KeyValues *userData)
{
	if ( !m_pDropDown->IsValidMenuID(itemID))
		return false;

	// when the menu item is selected it will send the custom message "SetText"
	KeyValues *kv = new KeyValues("SetText");
	kv->SetWString("text", itemText);
	m_pDropDown->UpdateMenuItem(itemID, itemText, kv, userData);
	InvalidateLayout();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Updates a current item to the drop down
// Input  : wchar_t *itemText - name of dropdown menu item
//-----------------------------------------------------------------------------
bool ComboBox::IsItemIDValid( int itemID )
{
	return m_pDropDown->IsValidMenuID(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::SetItemEnabled(const char *itemText, bool state)
{
	m_pDropDown->SetItemEnabled(itemText, state);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::SetItemEnabled(int itemID, bool state)
{
	m_pDropDown->SetItemEnabled(itemID, state);
}

//-----------------------------------------------------------------------------
// Purpose: Remove all items from the drop down menu
//-----------------------------------------------------------------------------
void ComboBox::RemoveAll()
{
	m_pDropDown->DeleteAllItems();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int ComboBox::GetItemCount() const
{
	return m_pDropDown->GetItemCount();
}

int ComboBox::GetItemIDFromRow( int row )
{
	// valid from [0, GetItemCount)
	return m_pDropDown->GetMenuID( row );
}

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, as if that menu item had been selected by the user
// Input  : itemID - itemID from AddItem in list of dropdown items
//-----------------------------------------------------------------------------
void ComboBox::ActivateItem(int itemID)
{
	m_pDropDown->ActivateItem(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, as if that menu item had been selected by the user
// Input  : itemID - itemID from AddItem in list of dropdown items
//-----------------------------------------------------------------------------
void ComboBox::ActivateItemByRow(int row)
{
	m_pDropDown->ActivateItemByRow(row);
}

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, without sending a TextChanged message
// Input  : row - row to activate
//-----------------------------------------------------------------------------
void ComboBox::SilentActivateItemByRow(int row)
{
	int itemID = GetItemIDFromRow( row );
	if ( itemID >= 0 )
	{
		SilentActivateItem( itemID );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, without sending a TextChanged message
// Input  : itemID - itemID from AddItem in list of dropdown items
//-----------------------------------------------------------------------------
void ComboBox::SilentActivateItem(int itemID)
{
	m_pDropDown->SilentActivateItem(itemID);

	// Now manually call our set text, with a wrapper to ensure we don't send the Text Changed message
	wchar_t name[ 256 ];
	GetItemText( itemID, name, sizeof( name ) );

	m_bPreventTextChangeMessage = true;
	OnSetText( name );
	m_bPreventTextChangeMessage = false;
}

//-----------------------------------------------------------------------------
// Purpose: Allows a custom menu to be used with the combo box
//-----------------------------------------------------------------------------
void ComboBox::SetMenu( Menu *menu )
{
	if ( m_pDropDown )
	{
		m_pDropDown->MarkForDeletion();
	}

	m_pDropDown = menu;
	if ( m_pDropDown )
	{
		m_pDropDown->SetParent( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Layout the format of the combo box for drawing on screen
//-----------------------------------------------------------------------------
void ComboBox::PerformLayout()
{
	int wide, tall;
	GetPaintSize(wide, tall);

	BaseClass::PerformLayout();

	m_pButton->SetBounds(wide - tall, 0, tall, tall);

	if ( IsEditable() )
	{
		SetCursor(dc_ibeam);
	}
	else
	{
		SetCursor(dc_arrow);
	}

	m_pButton->SetEnabled(IsEnabled());

	DoMenuLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::DoMenuLayout()
{
	m_pDropDown->PositionRelativeToPanel( this, m_iDirection, m_iOpenOffsetY );

	// reset the width of the drop down menu to be the width of the combo box
	m_pDropDown->SetFixedWidth(GetWide());
	m_pDropDown->ForceCalculateWidth();

}

//-----------------------------------------------------------------------------
// Purpose: Sorts the items in the list
//-----------------------------------------------------------------------------
void ComboBox::SortItems( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: return the index of the last selected item
//-----------------------------------------------------------------------------
int ComboBox::GetActiveItem()
{
	return m_pDropDown->GetActiveItem();
}

//-----------------------------------------------------------------------------
// Purpose: return the index of the current item. Returns the highlighted item unless
// no item is highlighted, in which case returns the active item.
//-----------------------------------------------------------------------------
int ComboBox::GetCurrentItem()
{
    int iHLItem = m_pDropDown->GetCurrentlyHighlightedItem();
    return iHLItem >= 0 ? iHLItem : m_pDropDown->GetActiveItem();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
KeyValues *ComboBox::GetActiveItemUserData()
{
	return m_pDropDown->GetItemUserData(GetActiveItem());
}	


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
KeyValues *ComboBox::GetItemUserData(int itemID)
{
	return m_pDropDown->GetItemUserData(itemID);
}	


//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void ComboBox::GetItemText( int itemID, wchar_t *text, int bufLenInBytes )
{
	m_pDropDown->GetItemText( itemID, text, bufLenInBytes );
}

void ComboBox::GetItemText( int itemID, char *text, int bufLenInBytes )
{
	m_pDropDown->GetItemText( itemID, text, bufLenInBytes );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ComboBox::IsDropdownVisible()
{
	return m_pDropDown->IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *inResourceData - 
//-----------------------------------------------------------------------------
void ComboBox::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder( pScheme->GetBorder( m_szBorderOverride[0] ? m_szBorderOverride : "ComboBoxBorder" ) );
}

void ComboBox::ApplySettings( KeyValues *pInResourceData )
{
	BaseClass::ApplySettings( pInResourceData );

	const char *pBorderOverride = pInResourceData->GetString( "border_override", NULL );
	if ( pBorderOverride )
	{
		V_strncpy( m_szBorderOverride, pBorderOverride, sizeof( m_szBorderOverride ) );
	}

	KeyValues *pKVButton = pInResourceData->FindKey( "Button" );
	if ( pKVButton && m_pButton )
	{
		m_pButton->ApplySettings( pKVButton );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the visiblity of the drop down menu button.
//-----------------------------------------------------------------------------
void ComboBox::SetDropdownButtonVisible(bool state)
{
	m_pButton->SetVisible(state);
}

//-----------------------------------------------------------------------------
// Purpose: overloads TextEntry MousePressed
//-----------------------------------------------------------------------------
void ComboBox::OnMousePressed(MouseCode code)
{
	if ( !m_pDropDown )
		return;

	if ( !IsEnabled() )
		return;

	// make sure it's getting pressed over us (it may not be due to mouse capture)
	if ( !IsCursorOver() )
	{
		HideMenu();
		return;
	}

	if ( IsEditable() )
	{
		BaseClass::OnMousePressed(code);
		HideMenu();
	}
	else
	{
		// clicking on a non-editable text box just activates the drop down menu
		RequestFocus();
		DoClick();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Double-click acts the same as a single-click
//-----------------------------------------------------------------------------
void ComboBox::OnMouseDoublePressed(MouseCode code)
{
    if (IsEditable())
    {
        BaseClass::OnMouseDoublePressed(code);
    }
    else
    {
	    OnMousePressed(code);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Called when a command is received from the menu
//			Changes the label text to be that of the command
// Input  : char *command - 
//-----------------------------------------------------------------------------
void ComboBox::OnCommand( const char *command )
{
	if (!stricmp(command, "ButtonClicked"))
	{
		// hide / show the menu underneath
		DoClick();
	}

	Panel::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::OnSetText(const wchar_t *newtext)
{
	// see if the combobox text has changed, and if so, post a message detailing the new text
	const wchar_t *text = newtext;

	// check if the new text is a localized string, if so undo it
	if (*text == '#')
	{
		char cbuf[255];
		g_pVGuiLocalize->ConvertUnicodeToANSI(text, cbuf, 255);

		// try lookup in localization tables
		StringIndex_t unlocalizedTextSymbol = g_pVGuiLocalize->FindIndex(cbuf + 1);
		
		if (unlocalizedTextSymbol != INVALID_LOCALIZE_STRING_INDEX)
		{
			// we have a new text value
			text = g_pVGuiLocalize->GetValueByIndex(unlocalizedTextSymbol);
		}
	}

	wchar_t wbuf[255];
	GetText(wbuf, 254);
	
	if ( wcscmp(wbuf, text) )
	{
		// text has changed
		SetText(text);

		// fire off that things have changed
		if ( !m_bPreventTextChangeMessage )
		{
			PostActionSignal(new KeyValues("TextChanged", "text", text));
		}
		Repaint();
	}

	// close the box
	HideMenu();
}

//-----------------------------------------------------------------------------
// Purpose: hides the menu
//-----------------------------------------------------------------------------
void ComboBox::HideMenu(void)
{
	if ( !m_pDropDown )
		return;

	// hide the menu
	m_pDropDown->SetVisible(false);
	Repaint();
	OnHideMenu(m_pDropDown);
}

//-----------------------------------------------------------------------------
// Purpose: shows the menu
//-----------------------------------------------------------------------------
void ComboBox::ShowMenu(void)
{
	if ( !m_pDropDown )
		return;

	// hide the menu
	m_pDropDown->SetVisible(false);
	DoClick();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the window loses focus; hides the menu
//-----------------------------------------------------------------------------
void ComboBox::OnKillFocus()
{
	SelectNoText();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the menu is closed
//-----------------------------------------------------------------------------
void ComboBox::OnMenuClose()
{
	HideMenu();

	if ( HasFocus() )
	{
		SelectAllText(false);
	}
	else if ( m_bHighlight )
	{
		m_bHighlight = false;
        // we want the text to be highlighted when we request the focus
//		SelectAllOnFirstFocus(true);
        RequestFocus();
	}
	// if cursor is in this box or the arrow box
	else if ( IsCursorOver() )// make sure it's getting pressed over us (it may not be due to mouse capture)
	{
		SelectAllText(false);
		OnCursorEntered();
		// Get focus so the box will unhighlight if we click somewhere else.
		RequestFocus();
	}
	else
	{
		m_pButton->SetArmed(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles hotkey accesses
// FIXME: make this open different directions as necessary see menubutton.
//-----------------------------------------------------------------------------
void ComboBox::DoClick()
{
	// menu is already visible, hide the menu
	if ( m_pDropDown->IsVisible() )
	{
		HideMenu();
		return;
	}

	// do nothing if menu is not enabled
	if ( !m_pDropDown->IsEnabled() )
	{
		return;
	}
	// force the menu to Think
	m_pDropDown->PerformLayout();

	// make sure we're at the top of the draw order (and therefore our children as well)
	// RequestFocus();
	
	// We want the item that is shown in the combo box to show as selected
	int itemToSelect = -1;
	int i;
	wchar_t comboBoxContents[255];
	GetText(comboBoxContents, 255);
	for ( i = 0 ; i < m_pDropDown->GetItemCount() ; i++ )
	{
		wchar_t menuItemName[255];
		int menuID = m_pDropDown->GetMenuID(i);
		m_pDropDown->GetMenuItem(menuID)->GetText(menuItemName, 255);
		if (!wcscmp(menuItemName, comboBoxContents))
		{
			itemToSelect = i;
			break;
		}
	}
	// if we found a match, highlight it on opening the menu
	if ( itemToSelect >= 0 )
	{
		m_pDropDown->SetCurrentlyHighlightedItem( m_pDropDown->GetMenuID(itemToSelect) );
	}

	// reset the dropdown's position
	DoMenuLayout();


	// make sure we're at the top of the draw order (and therefore our children as well)
	// this important to make sure the menu will be drawn in the foreground
	MoveToFront();

	// !KLUDGE! Force alpha to solid.  Otherwise,
	// we run into weird VGUI problems with pops
	// and the stencil test
	Color c = m_pDropDown->GetBgColor();
	c[3] = 255;
	m_pDropDown->SetBgColor( c );

	// notify
	OnShowMenu(m_pDropDown);

	// show the menu
	m_pDropDown->SetVisible(true);

	// bring to focus
	m_pDropDown->RequestFocus();

	// no text is highlighted when the menu is opened
	SelectNoText();

	// highlight the arrow while menu is open
	m_pButton->SetArmed(true);

	Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: Brighten the arrow on the button when entering the box
//-----------------------------------------------------------------------------
void ComboBox::OnCursorEntered()
{
	// want the arrow to go white when we enter the box 
	m_pButton->OnCursorEntered();
	TextEntry::OnCursorEntered();
}

//-----------------------------------------------------------------------------
// Purpose: Dim the arrow on the button when exiting the box
//-----------------------------------------------------------------------------
void ComboBox::OnCursorExited()
{
	// want the arrow to go grey when we exit the box if the menu is not open
	if ( !m_pDropDown->IsVisible() )
	{
		m_pButton->SetArmed(false);
		TextEntry::OnCursorExited();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::OnMenuItemSelected()
{
	m_bHighlight = true;
	// For editable cbs, fill in the text field from whatever is chosen from the dropdown...
	//if ( m_bAllowEdit )
	{
		int idx = GetActiveItem();
		if ( idx >= 0 )
		{
			wchar_t name[ 256 ];
			GetItemText( idx, name, sizeof( name ) );

			OnSetText( name );
		}
	}

	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged( wide, tall);

	// set the drawwidth.
	int bwide, btall;
	PerformLayout();
	m_pButton->GetSize( bwide, btall);
	SetDrawWidth( wide - bwide );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::OnSetFocus()
{
    BaseClass::OnSetFocus();

	GotoTextEnd();
	SelectAllText(false);
}

//-----------------------------------------------------------------------------
// Purpose: Handles up/down arrows
//-----------------------------------------------------------------------------
void ComboBox::OnKeyCodeTyped(KeyCode code)
{
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));

	if (alt)
	{
		switch (code)
		{
		case KEY_UP:
		case KEY_DOWN:
			{
				DoClick();
				break;
			}
		default:
			{				
				BaseClass::OnKeyCodeTyped(code);
				break;
			}
		}
	}
	else
	{
		switch (code)
		{
		case KEY_HOME:
		case KEY_END:
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
		case KEY_UP:
		case KEY_DOWN:
			{
				int itemSelected = m_pDropDown->GetCurrentlyHighlightedItem();
				m_pDropDown->OnKeyCodeTyped(code);				
				int itemToSelect = m_pDropDown->GetCurrentlyHighlightedItem();

				if ( itemToSelect != itemSelected )
				{
					SelectMenuItem(itemToSelect);
				}
				break;
			}

		case KEY_ENTER:
			{
				int itemToSelect = m_pDropDown->GetCurrentlyHighlightedItem();
				m_pDropDown->ActivateItem(itemToSelect);
				break;
			}

		default:
			{
				BaseClass::OnKeyCodeTyped(code);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: handles key input
//-----------------------------------------------------------------------------
void ComboBox::OnKeyTyped(wchar_t unichar)
{
    // don't play with key presses in edit mode
    // x1b = escape key. This is needed as escape conflicts with hiding settings in game
    // (combobox was being reset to first item when settings are hidden)
	if ( IsEditable() || unichar == '\t' || unichar == '\x1b') 
	{
		BaseClass::OnKeyTyped( unichar );
		return;
	}

	int itemSelected = m_pDropDown->GetCurrentlyHighlightedItem();
	m_pDropDown->OnKeyTyped(unichar);
	int itemToSelect = m_pDropDown->GetCurrentlyHighlightedItem();

	if ( itemToSelect != itemSelected )
	{
		SelectMenuItem(itemToSelect);
	}
	else
	{
		BaseClass::OnKeyTyped( unichar );
	}
}

void ComboBox::SelectMenuItem(int itemToSelect)
{
	// if we found this item, then we scroll up or down
	if ( itemToSelect >= 0 && itemToSelect < m_pDropDown->GetItemCount() )
	{
		wchar_t menuItemName[255];

		int menuID = m_pDropDown->GetMenuID(itemToSelect);
		m_pDropDown->GetMenuItem(menuID)->GetText(menuItemName, 254);
		OnSetText(menuItemName);
		SelectAllText(false);		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ComboBox::MoveAlongMenuItemList(int direction)
{
	// We want the item that is shown in the combo box to show as selected
	int itemToSelect = -1;
    wchar_t menuItemName[255];
	int i;

	wchar_t comboBoxContents[255];
	GetText(comboBoxContents, 254);
	for ( i = 0 ; i < m_pDropDown->GetItemCount() ; i++ )
	{
		int menuID = m_pDropDown->GetMenuID(i);
		m_pDropDown->GetMenuItem(menuID)->GetText(menuItemName, 254);

		if ( !wcscmp(menuItemName, comboBoxContents) )
		{
			itemToSelect = i;
			break;
		}
	}

	if ( itemToSelect >= 0 )
	{
		int newItem = itemToSelect + direction;
		if ( newItem < 0 )
		{
			newItem = 0;
		}
		else if ( newItem >= m_pDropDown->GetItemCount() )
		{
			newItem = m_pDropDown->GetItemCount() - 1;
		}
		SelectMenuItem(newItem);
	}

}

void ComboBox::MoveToFirstMenuItem()
{
	SelectMenuItem(0);
}

void ComboBox::MoveToLastMenuItem()
{
	SelectMenuItem(m_pDropDown->GetItemCount() - 1);
}


//-----------------------------------------------------------------------------
// Purpose: Sets the direction from the menu button the menu should open
//-----------------------------------------------------------------------------
void ComboBox::SetOpenDirection(Menu::MenuDirection_e direction)
{
	m_iDirection = direction;
}

void ComboBox::SetFont( HFont font )
{
	BaseClass::SetFont( font );

	m_pDropDown->SetFont( font );
}


void ComboBox::SetUseFallbackFont( bool bState, HFont hFallback )
{
	BaseClass::SetUseFallbackFont( bState, hFallback );
	m_pDropDown->SetUseFallbackFont( bState, hFallback );
}
