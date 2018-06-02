//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "vgui_controls/consoledialog.h"

#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "KeyValues.h"

#include "vgui_controls/Button.h"
#include "vgui/KeyCode.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/RichText.h"
#include "tier1/convar.h"
#include "tier1/convar_serverbounded.h"
#include "icvar.h"
#include "filesystem.h"

#include <stdlib.h>

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Used by the autocompletion system
//-----------------------------------------------------------------------------
class CNonFocusableMenu : public Menu
{
	DECLARE_CLASS_SIMPLE( CNonFocusableMenu, Menu );

public:
	CNonFocusableMenu( Panel *parent, const char *panelName )
		: BaseClass( parent, panelName ),
		m_pFocus( nullptr )
	{
	}

	void SetFocusPanel( Panel *panel )
	{
		m_pFocus = panel;
	}

	VPANEL GetCurrentKeyFocus()
	{
		if ( !m_pFocus )
			return GetVPanel();

		return m_pFocus->GetVPanel();
	}

private:
	Panel		*m_pFocus;
};


//-----------------------------------------------------------------------------
// Purpose: forwards tab key presses up from the text entry so we can do autocomplete
//-----------------------------------------------------------------------------
class TabCatchingTextEntry : public TextEntry
{
public:
	TabCatchingTextEntry(Panel *parent, const char *name, VPANEL comp) : TextEntry(parent, name), m_pCompletionList( comp )
	{
		SetAllowNonAsciiCharacters( true );
		SetDragEnabled( true );
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		if (code == KEY_TAB)
		{
			GetParent()->OnKeyCodeTyped(code);
		}
		else if ( code == KEY_ENTER )
		{
			// submit is the default button whose click event will have been called already
		}
		else
		{
			TextEntry::OnKeyCodeTyped(code);
		}
	}

	virtual void OnKillFocus()
	{
		if ( input()->GetFocus() != m_pCompletionList ) // if its not the completion window trying to steal our focus
		{
			PostMessage(GetParent(), new KeyValues("CloseCompletionList"));
		}
	}

private:
	VPANEL m_pCompletionList;
};



// Things the user typed in and hit submit/return with
CHistoryItem::CHistoryItem( void )
{
	m_bHasExtra = false;
}

CHistoryItem::CHistoryItem( const char *text, const char *extra )
{
	Assert( text );
	m_bHasExtra = false;
	SetText( text , extra );
}

CHistoryItem::CHistoryItem( const CHistoryItem& src )
{
	m_bHasExtra = false;
	SetText( src.GetText(), src.GetExtra() );
}

CHistoryItem::~CHistoryItem( void )
{
    m_text.Purge();
    m_extraText.Purge();
}

const char *CHistoryItem::GetText() const
{
    return m_text.Get();
}

const char *CHistoryItem::GetExtra() const
{
    return m_extraText.IsEmpty() ? nullptr : m_extraText.Get();
}

void CHistoryItem::SetText( const char *text, const char *extra )
{
    m_text.Purge();
    m_text = text;

	if ( extra )
	{
		m_bHasExtra = true;
        m_extraText.Purge();
        m_extraText = extra;
	}
	else
	{
		m_bHasExtra = false;
	}
}


//-----------------------------------------------------------------------------
//
// Console page completion item starts here
//
//-----------------------------------------------------------------------------
CConsolePanel::CompletionItem::CompletionItem( void )
{
	m_bIsCommand = true;
	m_pCommand = nullptr;
	m_pText = nullptr;
}

CConsolePanel::CompletionItem::CompletionItem( const CompletionItem& src )
{
	m_bIsCommand = src.m_bIsCommand;
	m_pCommand = src.m_pCommand;
	if ( src.m_pText )
	{
		m_pText = new CHistoryItem( (const CHistoryItem& )src.m_pText );
	}
	else
	{
		m_pText = nullptr;
	}
}

CConsolePanel::CompletionItem& CConsolePanel::CompletionItem::operator =( const CompletionItem& src )
{
	if ( this == &src )
		return *this;

	m_bIsCommand = src.m_bIsCommand;
	m_pCommand = src.m_pCommand;
	if ( src.m_pText )
	{
		m_pText = new CHistoryItem( (const CHistoryItem& )*src.m_pText );
	}
	else
	{
		m_pText = nullptr;
	}

	return *this;
}

CConsolePanel::CompletionItem::~CompletionItem( void )
{
	if ( m_pText )
	{
		delete m_pText;
		m_pText = nullptr;
	}
}

const char *CConsolePanel::CompletionItem::GetName() const
{
	if ( m_bIsCommand )
		return m_pCommand->GetName();
	return m_pCommand ? m_pCommand->GetName() : GetCommand();
}

const char *CConsolePanel::CompletionItem::GetItemText( void )
{
	static char text[256];
	text[0] = 0;
	if ( m_pText )
	{
		if ( m_pText->HasExtra() )
		{
			Q_snprintf( text, sizeof( text ), "%s %s", m_pText->GetText(), m_pText->GetExtra() );
		}
		else
		{
			Q_strncpy( text, m_pText->GetText(), sizeof( text ) );
		}
	}
	return text;
}	

const char *CConsolePanel::CompletionItem::GetCommand( void ) const
{
	static char text[256];
	text[0] = 0;
	if ( m_pText )
	{
		Q_strncpy( text, m_pText->GetText(), sizeof( text ) );
	}
	return text;
}


//-----------------------------------------------------------------------------
//
// Console page starts here
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: Constructor, destuctor
//-----------------------------------------------------------------------------
CConsolePanel::CConsolePanel( vgui::Panel *pParent, const char *pName, bool bStatusVersion ) : 
	BaseClass( pParent, pName ), m_bStatusVersion( bStatusVersion )
{
	SetKeyBoardInputEnabled( true );

	if ( !m_bStatusVersion )
	{
		SetMinimumSize(100,100);
	}

	// create controls
	m_pHistory = new RichText(this, "ConsoleHistory");
	m_pHistory->SetAllowKeyBindingChainToParent( false );
	SETUP_PANEL( m_pHistory );
	m_pHistory->SetVerticalScrollbar( !m_bStatusVersion );
	if ( m_bStatusVersion )
	{
		m_pHistory->SetDrawOffsets( 3, 3 );
	}
	m_pHistory->GotoTextEnd();
	
	m_pSubmit = new Button(this, "ConsoleSubmit", "#Console_Submit");
	m_pSubmit->SetCommand("submit");
	m_pSubmit->SetVisible( !m_bStatusVersion );
    m_pSubmit->SetContentAlignment(Label::a_center);
    m_pSubmit->SetAutoWide(true);
    m_pSubmit->SetAutoTall(true);

	CNonFocusableMenu *pCompletionList = new CNonFocusableMenu( this, "CompletionList" );
	m_pCompletionList = pCompletionList;
	m_pCompletionList->SetVisible(false);

	m_pEntry = new TabCatchingTextEntry(this, "ConsoleEntry", m_pCompletionList->GetVPanel() );
	m_pEntry->AddActionSignalTarget(this);
	m_pEntry->SendNewLine(true);
	pCompletionList->SetFocusPanel( m_pEntry );

    m_pEntry->PinToSibling("ConsoleHistory", PIN_TOPLEFT, PIN_BOTTOMLEFT);
    m_pSubmit->PinToSibling("ConsoleEntry", PIN_TOPLEFT, PIN_TOPRIGHT);
    pCompletionList->PinToSibling("ConsoleEntry", PIN_TOPLEFT, PIN_BOTTOMLEFT);

	// need to set up default colors, since ApplySchemeSettings won't be called until later
	m_PrintColor = Color(216, 222, 211, 255);
	m_DPrintColor = Color(196, 181, 80, 255);

	m_pEntry->SetTabPosition(1);

	m_bAutoCompleteMode = false;
	m_szPartialText[0] = 0;
	m_szPreviousPartialText[0]=0;

	// Add to global console list
	g_pCVar->InstallConsoleDisplayFunc( this );
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CConsolePanel::~CConsolePanel()
{
	ClearCompletionList();
	m_CommandHistory.Purge();
	g_pCVar->RemoveConsoleDisplayFunc( this );
}


//-----------------------------------------------------------------------------
// Updates the completion list
//-----------------------------------------------------------------------------
void CConsolePanel::OnThink()
{
	BaseClass::OnThink();

	if ( !IsVisible() )
		return;

	if ( !m_pCompletionList->IsVisible() )
		return;

	UpdateCompletionListPosition();
}


//-----------------------------------------------------------------------------
// Purpose: Clears the console
//-----------------------------------------------------------------------------
void CConsolePanel::Clear()
{
	m_pHistory->SetText("");
	m_pHistory->GotoTextEnd();
}


//-----------------------------------------------------------------------------
// Purpose: color text print
//-----------------------------------------------------------------------------
void CConsolePanel::ColorPrint( const Color& clr, const char *msg )
{
	if ( m_bStatusVersion )
	{
		Clear();
	}

	m_pHistory->InsertColorChange( clr );
	m_pHistory->InsertString( msg );
}


//-----------------------------------------------------------------------------
// Purpose: normal text print
//-----------------------------------------------------------------------------
void CConsolePanel::Print(const char *msg)
{
	ColorPrint( m_PrintColor, msg );
}


//-----------------------------------------------------------------------------
// Purpose: debug text print
//-----------------------------------------------------------------------------
void CConsolePanel::DPrint( const char *msg )
{
	ColorPrint( m_DPrintColor, msg );
}


void CConsolePanel::ClearCompletionList()
{
	int c = m_CompletionList.Count();
	int i;
	for ( i = c - 1; i >= 0; i-- )
	{
		delete m_CompletionList[ i ];
	}
	m_CompletionList.Purge();
}


static ConCommand *FindAutoCompleteCommmandFromPartial( const char *partial )
{
	char command[ 256 ];
	Q_strncpy( command, partial, sizeof( command ) );

	char *space = Q_strstr( command, " " );
	if ( space )
	{
		*space = 0;
	}

	ConCommand *cmd = g_pCVar->FindCommand( command );
	if ( !cmd )
		return nullptr;

	if ( !cmd->CanAutoComplete() )
		return nullptr;

	return cmd;
}


//-----------------------------------------------------------------------------
// Purpose: rebuilds the list of possible completions from the current entered text
//-----------------------------------------------------------------------------
void CConsolePanel::RebuildCompletionList(const char *text)
{
	ClearCompletionList();

	// we need the length of the text for the partial string compares
	int len = Q_strlen(text);
	if ( len < 1 )
	{
		// Fill the completion list with history instead
		for ( int i = 0 ; i < m_CommandHistory.Count(); i++ )
		{
			CHistoryItem *item = &m_CommandHistory[ i ];
			CompletionItem *comp = new CompletionItem();
			m_CompletionList.AddToTail( comp );
			comp->m_bIsCommand = false;
			comp->m_pCommand = nullptr;
			comp->m_pText = new CHistoryItem( *item );
		}
		return;
	}

	bool bNormalBuild = true;

	// if there is a space in the text, and the command isn't of the type to know how to autocomplet, then command completion is over
	const char *space = strstr( text, " " );
	if ( space )
	{
		ConCommand *pCommand = FindAutoCompleteCommmandFromPartial( text );
		if ( !pCommand )
			return;

		bNormalBuild = false;

		CUtlVector< CUtlString > commands;
		int count = pCommand->AutoCompleteSuggest( text, commands );
		Assert( count <= COMMAND_COMPLETION_MAXITEMS );
		int i;

		for ( i = 0; i < count; i++ )
		{
			// match found, add to list
			CompletionItem *item = new CompletionItem();
			m_CompletionList.AddToTail( item );
			item->m_bIsCommand = false;
			item->m_pCommand = nullptr;
			item->m_pText = new CHistoryItem( commands[ i ].String() );
		}
	}
				 
	if ( bNormalBuild )
	{
		// look through the command list for all matches
		ConCommandBase const *cmd = (ConCommandBase const *)cvar->GetCommands();
		while (cmd)
		{
			if ( cmd->IsFlagSet( FCVAR_DEVELOPMENTONLY ) || cmd->IsFlagSet( FCVAR_HIDDEN ) )
			{
				cmd = cmd->GetNext();
				continue;
			}

			if ( !strnicmp(text, cmd->GetName(), len))
			{
				// match found, add to list
				CompletionItem *item = new CompletionItem();
				m_CompletionList.AddToTail( item );
				item->m_pCommand = (ConCommandBase *)cmd;
				const char *tst = cmd->GetName();
				if ( !cmd->IsCommand() )
				{
					item->m_bIsCommand = false;
					ConVar *var = ( ConVar * )cmd;
					ConVar_ServerBounded *pBounded = dynamic_cast<ConVar_ServerBounded*>( var );
					if ( pBounded || var->IsFlagSet( FCVAR_NEVER_AS_STRING ) )
					{
						char strValue[512];
						
						int intVal = pBounded ? pBounded->GetInt() : var->GetInt();
						float floatVal = pBounded ? pBounded->GetFloat() : var->GetFloat();
						
						if ( floatVal == intVal )
							Q_snprintf( strValue, sizeof( strValue ), "%d", intVal );
						else
							Q_snprintf( strValue, sizeof( strValue ), "%f", floatVal );

						item->m_pText = new CHistoryItem( var->GetName(), strValue );
					}
					else
					{
						item->m_pText = new CHistoryItem( var->GetName(), var->GetString() );
					}
				}
				else
				{
					item->m_bIsCommand = true;
					item->m_pText = new CHistoryItem( tst );
				}
			}

			cmd = cmd->GetNext();
		}

		// Now sort the list by command name
		if ( m_CompletionList.Count() >= 2 )
		{
			for ( int i = 0 ; i < m_CompletionList.Count(); i++ )
			{
				for ( int j = i + 1; j < m_CompletionList.Count(); j++ )
				{
					const CompletionItem *i1, *i2;
					i1 = m_CompletionList[ i ];
					i2 = m_CompletionList[ j ];

					if ( Q_stricmp( i1->GetName(), i2->GetName() ) > 0 )
					{
						CompletionItem *temp = m_CompletionList[ i ];
						m_CompletionList[ i ] = m_CompletionList[ j ];
						m_CompletionList[ j ] = temp;
					}
				}
			}
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: auto completes current text
//-----------------------------------------------------------------------------
void CConsolePanel::OnAutoComplete(bool reverse)
{
	if (!m_bAutoCompleteMode)
	{
		// we're not in auto-complete mode, Start
		m_iNextCompletion = 0;
		m_bAutoCompleteMode = true;
	}

	// if we're in reverse, move back to before the current
	if (reverse)
	{
		m_iNextCompletion -= 2;
		if (m_iNextCompletion < 0)
		{
			// loop around in reverse
			m_iNextCompletion = m_CompletionList.Size() - 1;
		}
	}

	// get the next completion
	if (!m_CompletionList.IsValidIndex(m_iNextCompletion))
	{
		// loop completion list
		m_iNextCompletion = 0;
	}

	// make sure everything is still valid
	if (!m_CompletionList.IsValidIndex(m_iNextCompletion))
		return;

	// match found, set text
	char completedText[256];
	CompletionItem *item = m_CompletionList[m_iNextCompletion];
	Assert( item );

	if ( !item->m_bIsCommand && item->m_pCommand )
	{
		Q_strncpy(completedText, item->GetCommand(), sizeof(completedText) - 2 );
	}
	else
	{
		Q_strncpy(completedText, item->GetItemText(), sizeof(completedText) - 2 );
	}

	if ( !Q_strstr( completedText, " " ) )
	{
		Q_strncat(completedText, " ", sizeof(completedText), COPY_ALL_CHARACTERS );
	}

	m_pEntry->SetText(completedText);
	m_pEntry->GotoTextEnd();
	m_pEntry->SelectNone();

	m_iNextCompletion++;
}


//-----------------------------------------------------------------------------
// Purpose: Called whenever the user types text
//-----------------------------------------------------------------------------
void CConsolePanel::OnTextChanged(Panel *panel)
{
	if (panel != m_pEntry)
		return;

	Q_strncpy( m_szPreviousPartialText, m_szPartialText, sizeof( m_szPreviousPartialText ) );

	// get the partial text the user type
	m_pEntry->GetText(m_szPartialText, sizeof(m_szPartialText));

	// see if they've hit the tilde key (which opens & closes the console)
	int len = Q_strlen(m_szPartialText);

	bool hitTilde = ( m_szPartialText[len - 1] == '~' || m_szPartialText[len - 1] == '`' ) ? true : false;

	bool altKeyDown = ( vgui::input()->IsKeyDown( KEY_LALT ) || vgui::input()->IsKeyDown( KEY_RALT ) ) ? true : false;
	bool ctrlKeyDown = ( vgui::input()->IsKeyDown( KEY_LCONTROL ) || vgui::input()->IsKeyDown( KEY_RCONTROL ) ) ? true : false;

	// Alt-Tilde toggles Japanese IME on/off!!!
	if ( ( len > 0 ) && hitTilde )
	{
		// Strip the last character (tilde)
		m_szPartialText[ len - 1 ] = L'\0';

		if( !altKeyDown && !ctrlKeyDown )
		{
			m_pEntry->SetText( "" );

			// close the console
			PostMessage( this, new KeyValues( "Close" ) );
			PostActionSignal( new KeyValues( "ClosedByHittingTilde" ) );
		}
		else
		{
			m_pEntry->SetText( m_szPartialText );
		}
		return;
	}

	// clear auto-complete state since the user has typed
	m_bAutoCompleteMode = false;

	RebuildCompletionList(m_szPartialText);

	// build the menu
	if ( m_CompletionList.Count() < 1 )
	{
		m_pCompletionList->SetVisible(false);
	}
	else
	{
		m_pCompletionList->SetVisible(true);
		m_pCompletionList->DeleteAllItems();
		const int MAX_MENU_ITEMS = 10;

		// add the first ten items to the list
		for (int i = 0; i < m_CompletionList.Count() && i < MAX_MENU_ITEMS; i++)
		{
			char text[256];
			text[0] = 0;
			if (i == MAX_MENU_ITEMS - 1)
			{
				Q_strncpy(text, "...", sizeof( text ) );
			}
			else
			{
				Assert( m_CompletionList[i] );
				Q_strncpy(text, m_CompletionList[i]->GetItemText(), sizeof( text ) );
			}
			KeyValues *kv = new KeyValues("CompletionCommand");
			kv->SetString("command",text);
			m_pCompletionList->AddMenuItem(text, kv, this);
		}

		UpdateCompletionListPosition();
	}
	
	RequestFocus();
	m_pEntry->RequestFocus();

}

//-----------------------------------------------------------------------------
// Purpose: generic vgui command handler
//-----------------------------------------------------------------------------
void CConsolePanel::OnCommand(const char *command)
{
	if ( !Q_stricmp( command, "Submit" ) )
	{
		// submit the entry as a console commmand
		char szCommand[256];
		m_pEntry->GetText(szCommand, sizeof(szCommand));
        CUtlString comm(szCommand);
        comm.Trim(); // Do whitespace trimming here so we're all good for later on
		PostActionSignal( new KeyValues( "CommandSubmitted", "command", comm.Get() ) );

		// add to the history
		Print("] ");
		Print(comm);
		Print("\n");

		// clear the field
		m_pEntry->SetText("");

		// clear the completion state
		OnTextChanged(m_pEntry);

		// always go the end of the buffer when the user has typed something
		m_pHistory->GotoTextEnd();

        // Extract the extra variables, if any
        CUtlString ex;
		const char *extra = strchr(comm, ' ');
		if ( extra )
		{
            int indx = extra - comm.Get();
            ex = comm.Slice(indx+1);
            comm.SetLength(indx);
		}

        // Add the command to the history
        if (!comm.IsEmpty())
            AddToHistory(comm, ex);

		m_pCompletionList->SetVisible(false);
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}


//-----------------------------------------------------------------------------
// Focus related methods
//-----------------------------------------------------------------------------
bool CConsolePanel::TextEntryHasFocus() const
{
	return ( input()->GetFocus() == m_pEntry->GetVPanel() );
}

void CConsolePanel::TextEntryRequestFocus()
{
	m_pEntry->RequestFocus();
}


//-----------------------------------------------------------------------------
// Purpose: swallows tab key pressed
//-----------------------------------------------------------------------------
void CConsolePanel::OnKeyCodeTyped(KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);

	// check for processing
	if ( TextEntryHasFocus() )
	{
		if (code == KEY_TAB)
		{
			bool reverse = false;
			if (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT))
			{
				reverse = true;
			}

			// attempt auto-completion
			OnAutoComplete(reverse);
			m_pEntry->RequestFocus();
		}
		else if (code == KEY_DOWN)
		{
			OnAutoComplete(false);
		//	UpdateCompletionListPosition();
		//	m_pCompletionList->SetVisible(true);

			m_pEntry->RequestFocus();
		}
		else if (code == KEY_UP)
		{
			OnAutoComplete(true);
			m_pEntry->RequestFocus();
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: lays out controls
//-----------------------------------------------------------------------------
void CConsolePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	// setup tab ordering
	GetFocusNavGroup().SetDefaultButton(m_pSubmit);

	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
	m_pEntry->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
	m_pHistory->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));

	// layout controls
	int wide, tall;
	GetSize(wide, tall);

	if ( !m_bStatusVersion )
	{
		const int inset = 8;
		const int entryHeight = surface()->GetFontTall(m_pEntry->GetFont());
		const int topHeight = 4;
		const int entryInset = 4;
		const int submitWide = m_pSubmit->GetWide();

		m_pHistory->SetPos(inset, inset + topHeight); 
		m_pHistory->SetSize(wide - (inset * 2), tall - (entryInset * 2 + inset * 2 + topHeight + entryHeight));
		m_pHistory->InvalidateLayout();

	     m_pEntry->SetSize( wide - submitWide - 2 * inset, entryHeight);
	}
	else
	{
		const int inset = 2;

		int entryWidth = wide / 2;
		if ( wide > 400 )
		{
			entryWidth = 200;
		}

		m_pEntry->SetBounds( inset, inset, entryWidth, tall - 2 * inset );

		m_pHistory->SetBounds( inset + entryWidth + inset, inset, ( wide - entryWidth ) - inset, tall - 2 * inset );
	}

	UpdateCompletionListPosition();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the position of the completion list popup
//-----------------------------------------------------------------------------
void CConsolePanel::UpdateCompletionListPosition()
{
	if ( m_pCompletionList->IsVisible() )
	{
		m_pEntry->RequestFocus();
		MoveToFront();
		m_pCompletionList->MoveToFront();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Closes the completion list
//-----------------------------------------------------------------------------
void CConsolePanel::CloseCompletionList()
{
	m_pCompletionList->SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: sets up colors
//-----------------------------------------------------------------------------
void CConsolePanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_PrintColor = GetSchemeColor("Console.TextColor", pScheme);
	m_DPrintColor = GetSchemeColor("Console.DevTextColor", pScheme);
	m_pHistory->SetFont( pScheme->GetFont( "ConsoleText", IsProportional() ) );
	m_pCompletionList->SetFont( pScheme->GetFont( "DefaultSmall", IsProportional() ) );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handles autocompletion menu input
//-----------------------------------------------------------------------------
void CConsolePanel::OnMenuItemSelected(const char *command)
{
	if ( strstr( command, "..." ) ) // stop the menu going away if you click on ...
	{
		m_pCompletionList->SetVisible( true );
	}
	else
	{
		m_pEntry->SetText(command);
		m_pEntry->GotoTextEnd();
		m_pEntry->InsertChar(' ');
		m_pEntry->GotoTextEnd();
	}
}

void CConsolePanel::Hide()
{
	OnClose();
	m_iNextCompletion = 0;
	RebuildCompletionList("");
}

void CConsolePanel::AddToHistory( CUtlString &command, CUtlString &extra )
{
	// Newest at end, oldest at head
	while ( m_CommandHistory.Count() >= MAX_HISTORY_ITEMS )
	{
		// Remove from head until size is reasonable
		m_CommandHistory.Remove( 0 );
	}

	// If it's already there, then remove since we'll add it to the end instead
	CHistoryItem *item = nullptr;
	for ( int i = m_CommandHistory.Count() - 1; i >= 0; i-- )
	{
		item = &m_CommandHistory[ i ];
		if ( !item )
			continue;

		if (!command.IsEqual_CaseInsensitive(item->GetText()))
			continue;

		if ( !extra.IsEmpty() || item->GetExtra() )
		{
			if ( extra.IsEmpty() || !item->GetExtra() )
				continue;

			// stricmp so two commands with the same starting text get added
			if (!extra.IsEqual_CaseInsensitive(item->GetExtra()))	
				continue;
		}
		m_CommandHistory.Remove( i );
	}

	item = &m_CommandHistory[ m_CommandHistory.AddToTail() ];
	Assert( item );
	item->SetText( command, extra );

	m_iNextCompletion = 0;
	RebuildCompletionList( m_szPartialText );
}

void CConsolePanel::GetConsoleText( char *pchText, size_t bufSize ) const
{
	wchar_t *temp = new wchar_t[ bufSize ];
	m_pHistory->GetText( 0, temp, bufSize * sizeof( wchar_t ) );
	g_pVGuiLocalize->ConvertUnicodeToANSI( temp, pchText, bufSize );
	delete[] temp;
}

//-----------------------------------------------------------------------------
// Purpose: writes out console to disk
//-----------------------------------------------------------------------------
void CConsolePanel::DumpConsoleTextToFile()
{
	const int CONDUMP_FILES_MAX_NUM = 1000;

	FileHandle_t handle;
	bool found = false;
	char szfile[ 512 ];

	// we don't want to overwrite other condump.txt files
	for ( int i = 0 ; i < CONDUMP_FILES_MAX_NUM ; ++i )
	{
		_snprintf( szfile, sizeof(szfile), "condump%03d.txt", i );
		if ( !g_pFullFileSystem->FileExists(szfile) )
		{
			found = true;
			break;
		}
	}

	if ( !found )
	{
		Print( "Can't condump! Too many existing condump output files in the gamedir!\n" );
		return;
	}

	handle = g_pFullFileSystem->Open( szfile, "wb" );
	if ( handle != FILESYSTEM_INVALID_HANDLE )
	{
		int pos = 0;
		while (1)
		{
			wchar_t buf[512];
			m_pHistory->GetText(pos, buf, sizeof(buf));
			pos += sizeof(buf) / sizeof(wchar_t);

			// don't continue if none left
			if (buf[0] == 0)
				break;

			// convert to ansi
			char ansi[512];
			g_pVGuiLocalize->ConvertUnicodeToANSI(buf, ansi, sizeof(ansi));

			// write to disk
			int len = strlen(ansi);
			for (int i = 0; i < len; i++)
			{
				// preceed newlines with a return
				if (ansi[i] == '\n')
				{
					char ret = '\r';
					g_pFullFileSystem->Write( &ret, 1, handle );
				}

				g_pFullFileSystem->Write( ansi + i, 1, handle );
			}
		}

		g_pFullFileSystem->Close( handle );

		Print( "console dumped to " );
		Print( szfile );
		Print( "\n" );
	}
	else
	{
		Print( "Unable to condump to " );
		Print( szfile );
		Print( "\n" );
	}
}


//-----------------------------------------------------------------------------
//
// Console dialog starts here
//
//-----------------------------------------------------------------------------
CConsoleDialog::CConsoleDialog( vgui::Panel *pParent, const char *pName, bool bStatusVersion ) : 
	BaseClass( pParent, pName )
{
	// initialize dialog
	SetVisible( false );
    SetProportional(!bStatusVersion);
	SetTitle( "#Console_Title", true );
	m_pConsolePanel = new CConsolePanel( this, "ConsolePage", bStatusVersion );
	m_pConsolePanel->AddActionSignalTarget( this );

    HScheme consoleScheme = scheme()->LoadSchemeFromFile("ConsoleScheme.res", "ConsoleScheme");
    if (consoleScheme)
    {
        SetScheme(consoleScheme);
    }
}

void CConsoleDialog::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	BaseClass::OnScreenSizeChanged( iOldWide, iOldTall );

	int sx, sy;
	surface()->GetScreenSize( sx, sy );
									 
	int w, h;
	GetSize( w, h );
	if ( w > sx || h > sy  )
	{
		if ( w > sx )
		{
			w = sx;
		}
		if ( h > sy )
		{
			h = sy;
		}

		// Try and lower the size to match the screen bounds
		SetSize( w, h );
	}
}


//-----------------------------------------------------------------------------
// Purpose: brings dialog to the fore
//-----------------------------------------------------------------------------
void CConsoleDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	int x, y, w, h;
	GetClientArea( x, y, w, h );
	m_pConsolePanel->SetBounds( x, y, w, h );

    SetMinimumSize(scheme()->GetProportionalScaledValue(200), scheme()->GetProportionalScaledValue(100));
}


//-----------------------------------------------------------------------------
// Purpose: brings dialog to the fore
//-----------------------------------------------------------------------------
void CConsoleDialog::Activate()
{
	BaseClass::Activate();
	m_pConsolePanel->m_pEntry->RequestFocus();
}


//-----------------------------------------------------------------------------
// Hides the dialog
//-----------------------------------------------------------------------------
void CConsoleDialog::Hide()
{
	OnClose();
	m_pConsolePanel->Hide();
}


//-----------------------------------------------------------------------------
// Close just hides the dialog
//-----------------------------------------------------------------------------
void CConsoleDialog::Close()
{
	Hide();
}


//-----------------------------------------------------------------------------
// Submits commands
//-----------------------------------------------------------------------------
void CConsoleDialog::OnCommandSubmitted( const char *pCommand )
{
	PostActionSignal( new KeyValues( "CommandSubmitted", "command", pCommand ) );
}


//-----------------------------------------------------------------------------
// Chain to the page
//-----------------------------------------------------------------------------
void CConsoleDialog::Print( const char *pMessage )
{
	m_pConsolePanel->Print( pMessage );
}

void CConsoleDialog::DPrint( const char *pMessage )
{
	m_pConsolePanel->DPrint( pMessage );
}

void CConsoleDialog::ColorPrint( const Color& clr, const char *msg )
{
	m_pConsolePanel->ColorPrint( clr, msg );
}

void CConsoleDialog::Clear()
{
	m_pConsolePanel->Clear( );
}

void CConsoleDialog::DumpConsoleTextToFile()
{
	m_pConsolePanel->DumpConsoleTextToFile( );
}


void CConsoleDialog::OnKeyCodePressed( vgui::KeyCode code )
{
	if ( code == KEY_XBUTTON_B )
	{
		Hide();
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}