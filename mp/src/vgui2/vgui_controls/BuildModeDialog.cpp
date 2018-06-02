//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <ctype.h>
#include <stdio.h>
#include <utlvector.h>
#include <utlbuffer.h>

#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/BuildModeDialog.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/MenuButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/Divider.h>
#include <vgui_controls/PanelListPanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct PanelItem_t
{
	PanelItem_t() : m_EditLabel(nullptr), m_EditPanel(nullptr), m_pCombo(nullptr), m_EditButton(nullptr),
	                m_iType(0)
    {
        m_szName[0] = '\0';
    }

	Label *m_EditLabel;
	TextEntry *m_EditPanel;
	ComboBox *m_pCombo;
	Button *m_EditButton;
	char m_szName[64];
	int m_iType;
};

class CSmallTextEntry : public TextEntry
{
	DECLARE_CLASS_SIMPLE( CSmallTextEntry, TextEntry );
public:

	CSmallTextEntry( Panel *parent, char const *panelName ) :
		BaseClass( parent, panelName )
	{
	}

	virtual void ApplySchemeSettings( IScheme *scheme )
	{
		BaseClass::ApplySchemeSettings( scheme );

		SetFont( scheme->GetFont( "DefaultVerySmall", IsProportional() ) );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Holds a list of all the edit fields for the currently selected panel
//-----------------------------------------------------------------------------
class BuildModeDialog::PanelList
{
public:

	CUtlVector<PanelItem_t> m_PanelList;

	void AddItem( Label *label, TextEntry *edit, ComboBox *combo, Button *button, const char *name, int type )
	{
		PanelItem_t item;
		item.m_EditLabel = label;
		item.m_EditPanel = edit;
		Q_strncpy(item.m_szName, name, sizeof(item.m_szName));
		item.m_iType = type;
		item.m_pCombo = combo;
		item.m_EditButton = button;

		m_PanelList.AddToTail( item );
	}

	void RemoveAll( void )
	{
		for ( int i = 0; i < m_PanelList.Size(); i++ )
		{
			PanelItem_t *item = &m_PanelList[i];
			delete item->m_EditLabel;
			delete item->m_EditPanel;
			delete item->m_EditButton;
		}

		m_PanelList.RemoveAll();
		m_pControls->RemoveAll();
	}

    void SetFont(HFont font)
	{
	    FOR_EACH_VEC(m_PanelList, i)
	    {
            PanelItem_t t = m_PanelList[i];

            if (t.m_EditPanel)
                t.m_EditPanel->SetFont(font);
            if (t.m_EditButton)
                t.m_EditButton->SetFont(font);
            if (t.m_EditLabel)
                t.m_EditLabel->SetFont(font);
            if (t.m_pCombo)
                t.m_pCombo->SetFont(font);
	    }
	}

	KeyValues *m_pResourceData;
	PanelListPanel *m_pControls;
};

//-----------------------------------------------------------------------------
// Purpose: Dialog for adding localized strings
//-----------------------------------------------------------------------------
class BuildModeLocalizedStringEditDialog : public Frame
{
	DECLARE_CLASS_SIMPLE(BuildModeLocalizedStringEditDialog, Frame);

public:

#pragma warning( disable : 4355 )
	BuildModeLocalizedStringEditDialog() : Frame(this, nullptr)
	{
		m_pTokenEntry = new TextEntry(this, nullptr);
		m_pValueEntry = new TextEntry(this, nullptr);
		m_pFileCombo = new ComboBox(this, nullptr, 12, false);
		m_pOKButton = new Button(this, nullptr, "OK");
		m_pCancelButton = new Button(this, nullptr, "Cancel");

		m_pCancelButton->SetCommand("Close");
		m_pOKButton->SetCommand("OK");

		// add the files to the combo
		for (int i = 0; i < g_pVGuiLocalize->GetLocalizationFileCount(); i++)
		{
			m_pFileCombo->AddItem(g_pVGuiLocalize->GetLocalizationFileName(i), nullptr);
		}
	}
#pragma warning( default : 4355 )

	virtual void DoModal(const char *token)
	{
		input()->SetAppModalSurface(GetVPanel());

		// setup data
		m_pTokenEntry->SetText(token);

		// lookup the value
		StringIndex_t val = g_pVGuiLocalize->FindIndex(token);
		if (val != INVALID_LOCALIZE_STRING_INDEX)
		{
			m_pValueEntry->SetText(g_pVGuiLocalize->GetValueByIndex(val));

			// set the place in the file combo
			m_pFileCombo->SetText(g_pVGuiLocalize->GetFileNameByIndex(val));
		}
		else
		{
			m_pValueEntry->SetText("");
		}
	}

private:
	virtual void PerformLayout()
	{
	}

	virtual void OnClose()
	{
		input()->SetAppModalSurface(NULL);
		BaseClass::OnClose();
		//PostActionSignal(new KeyValues("Command"
	}

	virtual void OnCommand(const char *command)
	{
		if (!stricmp(command, "OK"))
		{
			//!! apply changes
		}
		else
		{
			BaseClass::OnCommand(command);
		}
	}

	vgui::TextEntry *m_pTokenEntry;
	vgui::TextEntry *m_pValueEntry;
	vgui::ComboBox *m_pFileCombo;
	vgui::Button *m_pOKButton;
	vgui::Button *m_pCancelButton;
};

class CBuildModeDialogMgr
{
public:
	
	void Add( BuildModeDialog *pDlg );
	void Remove( BuildModeDialog *pDlg );

	int Count() const;

private:
	CUtlVector< BuildModeDialog * > m_vecBuildDialogs;
};

static CBuildModeDialogMgr g_BuildModeDialogMgr;

void CBuildModeDialogMgr::Add( BuildModeDialog *pDlg )
{
	if ( m_vecBuildDialogs.Find( pDlg ) == m_vecBuildDialogs.InvalidIndex() )
	{
		m_vecBuildDialogs.AddToTail( pDlg );
	}
}

void CBuildModeDialogMgr::Remove( BuildModeDialog *pDlg )
{
	m_vecBuildDialogs.FindAndRemove( pDlg );
}

int CBuildModeDialogMgr::Count() const
{
	return m_vecBuildDialogs.Count();
}

int GetBuildModeDialogCount()
{
	return g_BuildModeDialogMgr.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
BuildModeDialog::BuildModeDialog(BuildGroup *buildGroup) : Frame(nullptr, "BuildModeDialog")
{
    SetProportional(true);
    SetMinimumSize(GetScaledVal(250), GetScaledVal(256));
	SetSize(300, 420);
	m_pCurrentPanel = nullptr;
	m_pEditableParents = nullptr;
	m_pEditableChildren = nullptr;
	m_pNextChild = nullptr;
	m_pPrevChild = nullptr;
	m_pBuildGroup = buildGroup;
	_undoSettings = nullptr;
	_copySettings = nullptr;
	_autoUpdate = false;
    m_Font = 0;
	SetTitle("VGUI Build Mode Editor", true);

	CreateControls();
	LoadUserConfig("BuildModeDialog");

	g_BuildModeDialogMgr.Add( this );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
BuildModeDialog::~BuildModeDialog()
{
	g_BuildModeDialogMgr.Remove( this );

	m_pPanelList->m_pResourceData->deleteThis();
	m_pPanelList->m_pControls->DeleteAllItems();
	if (_undoSettings)
		_undoSettings->deleteThis();
	if (_copySettings)
		_copySettings->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: makes sure build mode has been shut down properly
//-----------------------------------------------------------------------------
void BuildModeDialog::OnClose()
{
	if (m_pBuildGroup->IsEnabled())
	{
		m_pBuildGroup->SetEnabled(false);
	}
	else
	{
		BaseClass::OnClose();
		MarkForDeletion();
	}
}

class CBuildModeNavCombo : public ComboBox
{
	DECLARE_CLASS_SIMPLE( CBuildModeNavCombo, ComboBox );
public:

	CBuildModeNavCombo(Panel *parent, const char *panelName, int numLines, bool allowEdit, bool getParents, Panel *context ) : 
		BaseClass( parent, panelName, numLines, allowEdit ),
		m_bParents( getParents )
	{
		m_hContext = context;
	}
	
	virtual void OnShowMenu(Menu *menu)
	{
		menu->DeleteAllItems();
		if ( !m_hContext.Get() )
			return;

		if ( m_bParents )
		{
			Panel *p = m_hContext->GetParent();
			while ( p )
			{
				EditablePanel *ep = dynamic_cast < EditablePanel * >( p );
				if ( ep && ep->GetBuildGroup() )
				{
					KeyValues *kv = new KeyValues( "Panel" );
					kv->SetPtr( "ptr", p );
					char const *text = ep->GetName() ? ep->GetName() : "unnamed";
					menu->AddMenuItem( text, new KeyValues("SetText", "text", text), GetParent(), kv );
				}
				p = p->GetParent();
			}
		}
		else
		{
			int i;
			int c = m_hContext->GetChildCount();
			for ( i = 0; i < c; ++i )
			{
				EditablePanel *ep = dynamic_cast < EditablePanel * >( m_hContext->GetChild( i ) );
				if ( ep && ep->IsVisible() && ep->GetBuildGroup() )
				{
					KeyValues *kv = new KeyValues( "Panel" );
					kv->SetPtr( "ptr", ep );
					char const *text = ep->GetName() ? ep->GetName() : "unnamed";
					menu->AddMenuItem( text, new KeyValues("SetText", "text", text), GetParent(), kv );
				}
			}
		}
	}
private:
	bool	m_bParents;
	vgui::PHandle m_hContext;
};

//-----------------------------------------------------------------------------
// Purpose: Creates the build mode editing controls
//-----------------------------------------------------------------------------
void BuildModeDialog::CreateControls()
{
	int i;
	m_pPanelList = new PanelList;
	m_pPanelList->m_pResourceData = new KeyValues( "BuildDialog" );
	m_pPanelList->m_pControls = new PanelListPanel(this, "BuildModeControls");

	// file to edit combo box is first
	m_pFileSelectionCombo = new ComboBox(this, "FileSelectionCombo", 10, false);
	for ( i = 0; i < m_pBuildGroup->GetRegisteredControlSettingsFileCount(); i++)
	{
		m_pFileSelectionCombo->AddItem(m_pBuildGroup->GetRegisteredControlSettingsFileByIndex(i), nullptr);
	}
	if (m_pFileSelectionCombo->GetItemCount() < 2)
	{
		m_pFileSelectionCombo->SetEnabled(false);
	}

	int buttonH = GetScaledVal(18);

	// status info at top of dialog
	m_pStatusLabel = new Label(this, "StatusLabel", "[nothing currently selected]");
	m_pStatusLabel->SetTextColorState(Label::CS_DULL);
	m_pStatusLabel->SetTall( buttonH );
	m_pDivider = new Divider(this, "Divider");
	// drop-down combo box for adding new controls
	m_pAddNewControlCombo = new ComboBox(this, nullptr, 30, false);
	m_pAddNewControlCombo->SetSize(GetScaledVal(116), buttonH);
	m_pAddNewControlCombo->SetOpenDirection(Menu::DOWN);

	m_pEditableParents = new CBuildModeNavCombo( this, nullptr, 15, false, true, m_pBuildGroup->GetContextPanel() );
	m_pEditableParents->SetSize(GetScaledVal(116), buttonH);
	m_pEditableParents->SetOpenDirection(Menu::DOWN);

	m_pEditableChildren = new CBuildModeNavCombo( this, nullptr, 15, false, false, m_pBuildGroup->GetContextPanel() );
	m_pEditableChildren->SetSize(GetScaledVal(116), buttonH);
	m_pEditableChildren->SetOpenDirection(Menu::DOWN);

	m_pNextChild = new Button( this, "NextChild", "Next", this );
	m_pNextChild->SetCommand( new KeyValues( "OnChangeChild", "direction", 1 ) );
    m_pNextChild->SetAutoWide(true);
    m_pNextChild->SetContentAlignment(Label::a_center);

	m_pPrevChild = new Button( this, "PrevChild", "Prev", this );
	m_pPrevChild->SetCommand( new KeyValues( "OnChangeChild", "direction", -1 ) );
    m_pPrevChild->SetAutoWide(true);
    m_pPrevChild->SetContentAlignment(Label::a_center);

	// controls that can be added
	// this list comes from controls EditablePanel can create by name.
	int defaultItem = m_pAddNewControlCombo->AddItem("None", nullptr);

	CUtlVector< char const * >	names;
	CBuildFactoryHelper::GetFactoryNames( names );
	// Sort the names
	CUtlRBTree< char const *, int > sorted( 0, 0, StringLessThan );

	for ( i = 0; i < names.Count(); ++i )
	{
		sorted.Insert( names[ i ] );
	}

	for ( i = sorted.FirstInorder(); i != sorted.InvalidIndex(); i = sorted.NextInorder( i ) )
	{
		m_pAddNewControlCombo->AddItem( sorted[ i ], nullptr );
	}

	m_pAddNewControlCombo->ActivateItem(defaultItem);

	m_pExitButton = new Button(this, "ExitButton", "&Exit");
	m_pExitButton->SetSize(GetScaledVal(64), buttonH);
    m_pExitButton->SetAutoWide(true);
    m_pExitButton->SetContentAlignment(Label::a_center);

	m_pSaveButton = new Button(this, "SaveButton", "&Save");
	m_pSaveButton->SetSize(GetScaledVal(64), buttonH);
    m_pSaveButton->SetAutoWide(true);
    m_pSaveButton->SetContentAlignment(Label::a_center);
	
	m_pApplyButton = new Button(this, "ApplyButton", "&Apply");
	m_pApplyButton->SetSize(GetScaledVal(64), buttonH);
    m_pApplyButton->SetAutoWide(true);
    m_pApplyButton->SetContentAlignment(Label::a_center);

	m_pReloadLocalization = new Button( this, "Localization", "&Reload Localization" );
	m_pReloadLocalization->SetSize(GetScaledVal(100), buttonH );
    m_pReloadLocalization->SetAutoWide(true);
    m_pReloadLocalization->SetContentAlignment(Label::a_center);

	m_pExitButton->SetCommand("Exit");
	m_pSaveButton->SetCommand("Save");
	m_pApplyButton->SetCommand("Apply");
	m_pReloadLocalization->SetCommand( new KeyValues( "ReloadLocalization" ) );

	m_pDeleteButton = new Button(this, "DeletePanelButton", "Delete");
	m_pDeleteButton->SetSize(GetScaledVal(64), buttonH);
	m_pDeleteButton->SetCommand("DeletePanel");
    m_pDeleteButton->SetAutoWide(true);
    m_pDeleteButton->SetContentAlignment(Label::a_center);

	m_pVarsButton = new MenuButton(this, "VarsButton", "Variables");
	m_pVarsButton->SetSize(GetScaledVal(72), buttonH);
	m_pVarsButton->SetOpenDirection(Menu::UP);
    m_pVarsButton->SetAutoWide(true);
    m_pVarsButton->SetContentAlignment(Label::a_center);
	
	// iterate the vars
	KeyValues *vars = m_pBuildGroup->GetDialogVariables();
	if (vars && vars->GetFirstSubKey())
	{
		// create the menu
		m_pVarsButton->SetEnabled(true);
		Menu *menu = new Menu(m_pVarsButton, "VarsMenu");

		// set all the variables to be copied to the clipboard when selected
		for (KeyValues *kv = vars->GetFirstSubKey(); kv != nullptr; kv = kv->GetNextKey())
		{
			char buf[32];
			_snprintf(buf, sizeof(buf), "%%%s%%", kv->GetName());
			menu->AddMenuItem(kv->GetName(), new KeyValues("SetClipboardText", "text", buf), this);
		}

		m_pVarsButton->SetMenu(menu);
	}
	else
	{
		// no variables
		m_pVarsButton->SetEnabled(false);
	}

	m_pApplyButton->SetTabPosition(1);
	m_pPanelList->m_pControls->SetTabPosition(2);
	m_pVarsButton->SetTabPosition(3);
	m_pDeleteButton->SetTabPosition(4);
	m_pAddNewControlCombo->SetTabPosition(5);
	m_pSaveButton->SetTabPosition(6);
	m_pExitButton->SetTabPosition(7);

	m_pEditableParents->SetTabPosition( 8 );
	m_pEditableChildren->SetTabPosition( 9 );

	m_pPrevChild->SetTabPosition( 10 );
	m_pNextChild->SetTabPosition( 11 );

	m_pReloadLocalization->SetTabPosition( 12 );
}

void BuildModeDialog::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_Font = pScheme->GetFont( "DefaultVerySmall", IsProportional() );
    m_pPanelList->SetFont(m_Font);
	m_pStatusLabel->SetFont(m_Font);
	m_pReloadLocalization->SetFont(m_Font);
	m_pExitButton->SetFont(m_Font);
	m_pSaveButton->SetFont(m_Font);
	m_pApplyButton->SetFont(m_Font);
	m_pAddNewControlCombo->SetFont(m_Font);
	m_pEditableParents->SetFont(m_Font);
	m_pEditableChildren->SetFont(m_Font);
	m_pDeleteButton->SetFont(m_Font);
	m_pVarsButton->SetFont(m_Font);
	m_pPrevChild->SetFont(m_Font);
	m_pNextChild->SetFont(m_Font);
}

//-----------------------------------------------------------------------------
// Purpose: lays out controls
//-----------------------------------------------------------------------------
void BuildModeDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	// layout parameters
	const int BORDER_GAP = GetScaledVal(16), 
    YGAP_SMALL = GetScaledVal(4), 
    YGAP_LARGE = GetScaledVal(8), 
    TITLE_HEIGHT = GetScaledVal(24), 
    BOTTOM_CONTROLS_HEIGHT = GetScaledVal(145), 
    XGAP = GetScaledVal(6);

	int wide, tall;
	GetSize(wide, tall);
	
	int xpos = BORDER_GAP;
	int ypos = BORDER_GAP + TITLE_HEIGHT;

	// controls from top down
	// selection combo
	m_pFileSelectionCombo->SetBounds(xpos, ypos, wide - (BORDER_GAP * 2), m_pStatusLabel->GetTall());
	ypos += (m_pStatusLabel->GetTall() + YGAP_SMALL);

	// status
	m_pStatusLabel->SetBounds(xpos, ypos, wide - (BORDER_GAP * 2), m_pStatusLabel->GetTall());
	ypos += (m_pStatusLabel->GetTall() + YGAP_SMALL);

	// center control
	m_pPanelList->m_pControls->SetPos(xpos, ypos);
	m_pPanelList->m_pControls->SetSize(wide - (BORDER_GAP * 2), tall - (ypos + BOTTOM_CONTROLS_HEIGHT));

	// controls from bottom-right
	ypos = tall - BORDER_GAP;
	xpos = BORDER_GAP + m_pVarsButton->GetWide() + m_pDeleteButton->GetWide() + m_pAddNewControlCombo->GetWide() + (XGAP * 2);

	// bottom row of buttons
	ypos -= m_pApplyButton->GetTall();
	xpos -= m_pApplyButton->GetWide();
	m_pApplyButton->SetPos(xpos, ypos);

	xpos -= m_pExitButton->GetWide();
	xpos -= XGAP;
	m_pExitButton->SetPos(xpos, ypos);

	xpos -= m_pSaveButton->GetWide();
	xpos -= XGAP;
	m_pSaveButton->SetPos(xpos, ypos);

	// divider
	xpos = BORDER_GAP;
	ypos -= (YGAP_LARGE + m_pDivider->GetTall());
    m_pDivider->SetBounds(xpos, ypos, wide - (xpos + BORDER_GAP), 2);

	ypos -= (YGAP_LARGE  + m_pVarsButton->GetTall());

	xpos = BORDER_GAP;
	m_pEditableParents->SetPos( xpos, ypos );
	m_pEditableChildren->SetPos( xpos + GetScaledVal(150), ypos );

	ypos -= (YGAP_LARGE + 18 );
	xpos = BORDER_GAP;
	m_pReloadLocalization->SetPos( xpos, ypos );

	xpos += ( XGAP ) + m_pReloadLocalization->GetWide();
	
	m_pPrevChild->SetPos( xpos, ypos );
	m_pPrevChild->SetSize( GetScaledVal(64), m_pReloadLocalization->GetTall() );
	xpos += ( XGAP ) + m_pPrevChild->GetWide();

	m_pNextChild->SetPos( xpos, ypos );
	m_pNextChild->SetSize( GetScaledVal(64), m_pReloadLocalization->GetTall() );

	ypos -= (YGAP_LARGE  + m_pVarsButton->GetTall());
	xpos = BORDER_GAP;

	// edit buttons
	m_pVarsButton->SetPos(xpos, ypos);
	xpos += (XGAP + m_pVarsButton->GetWide());
	m_pDeleteButton->SetPos(xpos, ypos);
	xpos += (XGAP + m_pDeleteButton->GetWide());
	m_pAddNewControlCombo->SetPos(xpos, ypos);
}


//-----------------------------------------------------------------------------
// Purpose: Deletes all the controls from the panel
//-----------------------------------------------------------------------------
void BuildModeDialog::RemoveAllControls( void )
{
	// free the array
	m_pPanelList->RemoveAll();
}

void BuildModeDialog::OnTextKillFocus()
{
	if ( !m_pCurrentPanel )
		return;

	ApplyDataToControls();
}


//-----------------------------------------------------------------------------
// Purpose: sets up the current control to edit
//-----------------------------------------------------------------------------
void BuildModeDialog::SetActiveControl(Panel *controlToEdit)
{	
	if (m_pCurrentPanel == controlToEdit)
	{
		// it's already set, so just update the property data and quit
		if (m_pCurrentPanel)
		{
			UpdateControlData(m_pCurrentPanel);
		}
		return;
	}

	// reset the data
	m_pCurrentPanel = controlToEdit;
	RemoveAllControls();
	m_pPanelList->m_pControls->MoveScrollBarToTop();

	if (!m_pCurrentPanel)
	{
		m_pStatusLabel->SetText("[nothing currently selected]");
		m_pStatusLabel->SetTextColorState(Label::CS_DULL);
		RemoveAllControls();
		return;
	}

	// get the control's settings and create controls based on the type
    CUtlVector<PanelSetting_t> *vecSettings = &m_pCurrentPanel->GetPanelSettings();
    int tabPosition = 1;
    FOR_EACH_VEC(*vecSettings, i)
    {
        PanelSetting_t setting = vecSettings->Element(i);

        int itemHeight = GetScaledVal(18);

        // build a control & label
        Label *label = new Label(this, nullptr, setting.name);
        label->SetSize(GetScaledVal(96), itemHeight);
        label->SetContentAlignment(Label::a_east);

        TextEntry *edit = nullptr;
        ComboBox *editCombo = nullptr;
        Button *editButton = nullptr;
        if (setting.type == TYPE_ALIGNMENT)
        {
            // drop-down combo box
            editCombo = new ComboBox(this, nullptr, 9, false);
            editCombo->AddItem("north-west", nullptr);
            editCombo->AddItem("north", nullptr);
            editCombo->AddItem("north-east", nullptr);
            editCombo->AddItem("west", nullptr);
            editCombo->AddItem("center", nullptr);
            editCombo->AddItem("east", nullptr);
            editCombo->AddItem("south-west", nullptr);
            editCombo->AddItem("south", nullptr);
            editCombo->AddItem("south-east", nullptr);

            edit = editCombo;
        }
        else if (setting.type == TYPE_AUTORESIZE)
        {
            // drop-down combo box
            editCombo = new ComboBox(this, nullptr, 4, false);
            editCombo->AddItem("0 - no auto-resize", nullptr);
            editCombo->AddItem("1 - resize right", nullptr);
            editCombo->AddItem("2 - resize down", nullptr);
            editCombo->AddItem("3 - down & right", nullptr);

            edit = editCombo;
        }
        else if (setting.type == TYPE_CORNER)
        {
            // drop-down combo box
            editCombo = new ComboBox(this, nullptr, 8, false);
            editCombo->AddItem("0 - top-left", nullptr);
            editCombo->AddItem("1 - top-right", nullptr);
            editCombo->AddItem("2 - bottom-left", nullptr);
            editCombo->AddItem("3 - bottom-right", nullptr);
            editCombo->AddItem("4 - center-top", nullptr);
            editCombo->AddItem("5 - center-right", nullptr);
            editCombo->AddItem("6 - center-bottom", nullptr);
            editCombo->AddItem("7 - center-left", nullptr);

            edit = editCombo;
        }
        else if (setting.type == TYPE_LOCALIZEDSTRING)
        {
            editButton = new Button(this, nullptr, "...");
            editButton->SetParent(this);
            editButton->AddActionSignalTarget(this);
            editButton->SetTabPosition(tabPosition++);
            editButton->SetTall(itemHeight);
            label->SetAssociatedControl(editButton);
        }
        else
        {
            // normal string edit
            edit = new CSmallTextEntry(this, nullptr);
        }

        if (edit)
        {
            edit->SetTall(itemHeight);
            edit->SetParent(this);
            edit->AddActionSignalTarget(this);
            edit->SetTabPosition(tabPosition++);
            edit->SetAllowNumericInputOnly(setting.type == TYPE_INTEGER || setting.type == TYPE_FLOAT || setting.type == TYPE_BOOL);
            label->SetAssociatedControl(edit);
        }

        // add to our control list
        m_pPanelList->AddItem(label, edit, editCombo, editButton, setting.name, setting.type);

        if (edit)
        {
            m_pPanelList->m_pControls->AddItem(label, edit);
        }
        else
        {
            m_pPanelList->m_pControls->AddItem(label, editButton);
        }
    }

	// check and see if the current panel is a Label
	// iterate through the class hierarchy 
    m_pDeleteButton->SetEnabled(controlToEdit->IsBuildModeDeletable());

	// update the property data in the dialog
	UpdateControlData(m_pCurrentPanel);
	
	// set our title
	if ( m_pBuildGroup->GetResourceName() )
	{
		m_pFileSelectionCombo->SetText(m_pBuildGroup->GetResourceName());
	}
	else
	{
		m_pFileSelectionCombo->SetText("[ no resource file associated with dialog ]");
	}

	m_pApplyButton->SetEnabled(false);
	InvalidateLayout(false, true);
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the edit fields with information about the control
//-----------------------------------------------------------------------------
void BuildModeDialog::UpdateControlData(Panel *control)
{
	KeyValues *dat = m_pPanelList->m_pResourceData->FindKey( control->GetName(), true );
	control->GetSettings( dat );

	// apply the settings to the edit panels
	for ( int i = 0; i < m_pPanelList->m_PanelList.Size(); i++ )
	{
		const char *name = m_pPanelList->m_PanelList[i].m_szName;
		const char *datstring = dat->GetString( name, "" );

		UpdateEditControl(m_pPanelList->m_PanelList[i], datstring);
	}

	char statusText[512];
	Q_snprintf(statusText, sizeof(statusText), "%s: \'%s\'", control->GetClassName(), control->GetName());
	m_pStatusLabel->SetText(statusText);
	m_pStatusLabel->SetTextColorState(Label::CS_NORMAL);
}

//-----------------------------------------------------------------------------
// Purpose: Updates the data in a single edit control
//-----------------------------------------------------------------------------
void BuildModeDialog::UpdateEditControl(PanelItem_t &panelItem, const char *datstring)
{
	switch (panelItem.m_iType)
	{
		case TYPE_AUTORESIZE:
		case TYPE_CORNER:
			{
				int dat = atoi(datstring);
				panelItem.m_pCombo->ActivateItemByRow(dat);
			}
			break;

		case TYPE_LOCALIZEDSTRING:
			{
				panelItem.m_EditButton->SetText(datstring);
			}
			break;

		default:
			{
				wchar_t unicode[512];
				g_pVGuiLocalize->ConvertANSIToUnicode(datstring, unicode, sizeof(unicode));
				panelItem.m_EditPanel->SetText(unicode);
			}
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when one of the buttons is pressed
//-----------------------------------------------------------------------------
void BuildModeDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Save"))
	{
		// apply the current data and save it to disk
		ApplyDataToControls();
		m_pBuildGroup->SaveControlSettings();
        // disable save button until another change has been made
        m_pSaveButton->SetEnabled(false);
	}
	else if (!stricmp(command, "Exit"))
	{
		// exit build mode
		ExitBuildMode();
	}
	else if (!stricmp(command, "Apply"))
	{
		// apply data to controls
		ApplyDataToControls();
	}
	else if (!stricmp(command, "DeletePanel"))
	{
		OnDeletePanel();
	}
	else if (!stricmp(command, "RevertToSaved"))
	{
		RevertToSaved();
	}
	else if (!stricmp(command, "ShowHelp"))
	{
		ShowHelp();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Deletes a panel from the buildgroup
//-----------------------------------------------------------------------------
void BuildModeDialog::OnDeletePanel()
{
	if (!m_pCurrentPanel->IsBuildModeEditable())
	{
		return; 
	}

	m_pBuildGroup->RemoveSettings();
	SetActiveControl(m_pBuildGroup->GetCurrentPanel());

	_undoSettings->deleteThis();
	_undoSettings = nullptr;
	m_pSaveButton->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Applies the current settings to the build controls
//-----------------------------------------------------------------------------
void BuildModeDialog::ApplyDataToControls()
{
	// don't apply if the panel is not editable
	if ( !m_pCurrentPanel->IsBuildModeEditable())
	{
		UpdateControlData( m_pCurrentPanel );
		return; // return success, since we are behaving as expected.
	}

	char fieldName[512];
	if (m_pPanelList->m_PanelList[0].m_EditPanel)
	{
		m_pPanelList->m_PanelList[0].m_EditPanel->GetText(fieldName, sizeof(fieldName));
	}
	else
	{
		m_pPanelList->m_PanelList[0].m_EditButton->GetText(fieldName, sizeof(fieldName));
	}

	// check to see if any buildgroup panels have this name
	Panel *panel = m_pBuildGroup->FieldNameTaken(fieldName);
	if (panel)
	{
		if (panel != m_pCurrentPanel)// make sure name is taken by some other panel not this one
		{
			char messageString[255];
			Q_snprintf(messageString, sizeof( messageString ), "Fieldname is not unique: %s\nRename it and try again.", fieldName);
			MessageBox *errorBox = new MessageBox("Cannot Apply", messageString);
			errorBox->DoModal();
			UpdateControlData(m_pCurrentPanel);
			m_pApplyButton->SetEnabled(false);
			return;
		}
	}

	// create a section to store settings
    KeyValues *dat = new KeyValues(m_pCurrentPanel->GetName());
    StoreSettings(dat);

	// dat is built, hand it back to the control
	m_pCurrentPanel->ApplySettings( dat );
    dat->deleteThis();

	if ( m_pBuildGroup->GetContextPanel() )
	{
		m_pBuildGroup->GetContextPanel()->Repaint();
	}

	m_pApplyButton->SetEnabled(false);
	m_pSaveButton->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Store the settings of the current panel in a KeyValues
//-----------------------------------------------------------------------------
void BuildModeDialog::StoreUndoSettings()
{
	// don't save if the planel is not editable
	if ( !m_pCurrentPanel->IsBuildModeEditable())
	{
		if (_undoSettings)
			_undoSettings->deleteThis();
		_undoSettings = nullptr;
		return; 
	}

	if (_undoSettings)
	{
		_undoSettings->deleteThis();
		_undoSettings = nullptr;
	}

    _undoSettings = new KeyValues(m_pCurrentPanel->GetName());
	StoreSettings(_undoSettings);
}


//-----------------------------------------------------------------------------
// Purpose: Revert to the stored the settings of the current panel in a keyValues
//-----------------------------------------------------------------------------
void BuildModeDialog::DoUndo()
{
	if ( _undoSettings )
	{		
		m_pCurrentPanel->ApplySettings( _undoSettings );
		UpdateControlData(m_pCurrentPanel);
		_undoSettings->deleteThis();
		_undoSettings = nullptr;
	}

	m_pSaveButton->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Copy the settings of the current panel into a keyValues
//-----------------------------------------------------------------------------
void BuildModeDialog::DoCopy()
{
	if (_copySettings)
	{
		_copySettings->deleteThis();
		_copySettings = nullptr;
	}

    _copySettings = new KeyValues(m_pCurrentPanel->GetName());
	StoreSettings(_copySettings);
	Q_strncpy (_copyClassName, m_pCurrentPanel->GetClassName(), sizeof( _copyClassName ) );
}

//-----------------------------------------------------------------------------
// Purpose: Create a new Panel with the _copySettings applied
//-----------------------------------------------------------------------------
void BuildModeDialog::DoPaste()
{
	// Make a new control located where you had the mouse
	int x, y;
	input()->GetCursorPos(x, y);
	m_pBuildGroup->GetContextPanel()->ScreenToLocal(x,y);

	Panel *newPanel = OnNewControl(_copyClassName, x, y);
	if (newPanel)
	{
		newPanel->ApplySettings(_copySettings);
		newPanel->SetPos(x, y);
		char name[255];
		m_pBuildGroup->GetNewFieldName(name, sizeof(name), newPanel);
		newPanel->SetName(name);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Store the settings of the current panel in a keyValues
//-----------------------------------------------------------------------------
void BuildModeDialog::StoreSettings(KeyValues *storedSettings)
{
	// loop through the textedit filling in settings
	for ( int i = 0; i < m_pPanelList->m_PanelList.Size(); i++ )
	{
        PanelItem_t *item = &m_pPanelList->m_PanelList[i];

		const char *name = item->m_szName;

        switch (item->m_iType)
        {
        case TYPE_CORNER:
        case TYPE_AUTORESIZE:
            if (item->m_pCombo)
                storedSettings->SetInt(name, item->m_pCombo->GetActiveItem());
            break;

        default:
            {
                char buf[512];
                if (item->m_EditPanel)
                {
                    item->m_EditPanel->GetText(buf, sizeof(buf));
                }
                else if (item->m_EditButton)
                {
                    item->m_EditButton->GetText(buf, sizeof(buf));
                }

                storedSettings->SetString(name, buf);
            }
            break;
        }
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BuildModeDialog::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ENTER) // if someone hits return apply the changes
	{
		ApplyDataToControls();
	}
    else if (code == KEY_R && (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL)))
    {
        if (m_pBuildGroup)
            m_pBuildGroup->ToggleRulerDisplay();
    }
	else
	{
		Frame::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if any text has changed
//-----------------------------------------------------------------------------
void BuildModeDialog::OnTextChanged( Panel *panel )
{
	if (panel == m_pFileSelectionCombo)
	{
		// reload file if it's changed
		char newFile[512];
		m_pFileSelectionCombo->GetText(newFile, sizeof(newFile));

		if (stricmp(newFile, m_pBuildGroup->GetResourceName()) != 0)
		{
			// file has changed, reload
			SetActiveControl(nullptr);
			m_pBuildGroup->ChangeControlSettingsFile(newFile);
		}
		return;
	}

	if (panel == m_pAddNewControlCombo)
	{
		char buf[40];
		m_pAddNewControlCombo->GetText(buf, 40);
		if (stricmp(buf, "None") != 0)
		{	
			OnNewControl(buf);
			// reset box back to None
			m_pAddNewControlCombo->ActivateItemByRow( 0 );
		}
	}

	if ( panel == m_pEditableChildren )
	{
		KeyValues *kv = m_pEditableChildren->GetActiveItemUserData();
		if ( kv )
		{
			EditablePanel *ep = reinterpret_cast< EditablePanel * >( kv->GetPtr( "ptr" ) );
			if ( ep )
			{
				ep->ActivateBuildMode();
			}
		}
	}

	if ( panel == m_pEditableParents )
	{
		KeyValues *kv = m_pEditableParents->GetActiveItemUserData();
		if ( kv )
		{
			EditablePanel *ep = reinterpret_cast< EditablePanel * >( kv->GetPtr( "ptr" ) );
			if ( ep )
			{
				ep->ActivateBuildMode();
			}
		}
	}

	if (m_pCurrentPanel && m_pCurrentPanel->IsBuildModeEditable())
	{
		m_pApplyButton->SetEnabled(true);
	}
	
	if (_autoUpdate) 
	{
		ApplyDataToControls();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BuildModeDialog::ExitBuildMode( void )
{
	// make sure rulers are off
	if (m_pBuildGroup->HasRulersOn())
	{
		m_pBuildGroup->ToggleRulerDisplay();
	}
	m_pBuildGroup->SetEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new control in the context panel
//-----------------------------------------------------------------------------
Panel *BuildModeDialog::OnNewControl( const char *name, int x, int y)
{
	// returns NULL on failure
	Panel *newPanel = m_pBuildGroup->NewControl(name, x, y);
	if (newPanel)
	{
	   // call mouse commands to simulate selecting the new
		// panel. This will set everything up correctly in the buildGroup.
		m_pBuildGroup->MousePressed(MOUSE_LEFT, newPanel);
		m_pBuildGroup->MouseReleased(MOUSE_LEFT, newPanel);
	}

	m_pSaveButton->SetEnabled(true);

	return newPanel;
}

//-----------------------------------------------------------------------------
// Purpose: enable the save button, useful when buildgroup needs to Activate it.
//-----------------------------------------------------------------------------
void BuildModeDialog::EnableSaveButton()
{
	m_pSaveButton->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Revert to the saved settings in the .res file
//-----------------------------------------------------------------------------
void BuildModeDialog::RevertToSaved()
{
	// hide the dialog as reloading will destroy it
	surface()->SetPanelVisible(this->GetVPanel(), false);
	m_pBuildGroup->ReloadControlSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Display some information about the editor
//-----------------------------------------------------------------------------
void BuildModeDialog::ShowHelp()
{
	char helpText[]= "In the Build Mode Dialog Window:\n" 
		"Delete button - deletes the currently selected panel if it is deletable.\n"
		"Apply button - applies changes to the Context Panel.\n"
		"Save button - saves all settings to file. \n"
		"Revert to saved- reloads the last saved file.\n"
		"Auto Update - any changes apply instantly.\n"
		"Typing Enter in any text field applies changes.\n"
		"New Control menu - creates a new panel in the upper left corner.\n\n" 
		"In the Context Panel:\n"
		"After selecting and moving a panel Ctrl-z will undo the move.\n"
		"Shift clicking panels allows multiple panels to be selected into a group.\n"
		"Ctrl-c copies the settings of the last selected panel.\n"
		"Ctrl-v creates a new panel with the copied settings at the location of the mouse pointer.\n"
		"Arrow keys slowly move panels, holding shift + arrow will slowly resize it.\n"
		"Holding right mouse button down opens a dropdown panel creation menu.\n"
		"  Panel will be created where the menu was opened.\n"
		"Delete key deletes the currently selected panel if it is deletable.\n"
		"  Does nothing to multiple selections.";
		
	MessageBox *helpDlg = new MessageBox ("Build Mode Help", helpText, this);
	helpDlg->AddActionSignalTarget(this);
	helpDlg->DoModal();
}

	
void BuildModeDialog::ShutdownBuildMode()
{
	m_pBuildGroup->SetEnabled(false);
}

void BuildModeDialog::OnPanelMoved()
{
	m_pApplyButton->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: message handles thats sets the text in the clipboard
//-----------------------------------------------------------------------------
void BuildModeDialog::OnSetClipboardText(const char *text)
{
	system()->SetClipboardText(text, strlen(text));
}

void BuildModeDialog::OnCreateNewControl( char const *text )
{
	if ( !Q_stricmp( text, "None" ) )
		return;

	OnNewControl( text, m_nClick[ 0 ], m_nClick[ 1 ] );
}

void BuildModeDialog::OnShowNewControlMenu()
{
	if ( !m_pBuildGroup )
		return;

	int i;

	input()->GetCursorPos( m_nClick[ 0 ], m_nClick[ 1 ] );
	m_pBuildGroup->GetContextPanel()->ScreenToLocal( m_nClick[ 0 ], m_nClick[ 1 ] );

	if ( m_hContextMenu )
		delete m_hContextMenu.Get();

	m_hContextMenu = new Menu( this, "NewControls" );

	// Show popup menu
	m_hContextMenu->AddMenuItem( "None", "None", new KeyValues( "CreateNewControl", "text", "None" ), this );

	CUtlVector< char const * >	names;
	CBuildFactoryHelper::GetFactoryNames( names );
	// Sort the names
	CUtlRBTree< char const *, int > sorted( 0, 0, StringLessThan );

	for ( i = 0; i < names.Count(); ++i )
	{
		sorted.Insert( names[ i ] );
	}

	for ( i = sorted.FirstInorder(); i != sorted.InvalidIndex(); i = sorted.NextInorder( i ) )
	{
		m_hContextMenu->AddMenuItem( sorted[ i ], sorted[ i ], new KeyValues( "CreateNewControl", "text", sorted[ i ] ), this );
	}

	Menu::PlaceContextMenu( this, m_hContextMenu );
}

void BuildModeDialog::OnReloadLocalization()
{
	// reload localization files
	g_pVGuiLocalize->ReloadLocalizationFiles( );
}

bool BuildModeDialog::IsBuildGroupEnabled()
{
	// Don't ever edit the actual build dialog!!!
	return false;
}

void BuildModeDialog::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
    BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

    SetMinimumSize(GetScaledVal(250), GetScaledVal(256));
    InvalidateLayout(true, true);
}

void BuildModeDialog::OnChangeChild( int direction )
{
	Assert( direction == 1 || direction == -1 );
	if ( !m_pBuildGroup )
		return;

	Panel *current = m_pCurrentPanel;
	Panel *context = m_pBuildGroup->GetContextPanel();

	if ( !current || current == context )
	{
		current = nullptr;
		if ( context->GetChildCount() > 0 )
		{
			current = context->GetChild( 0 );
		}
	}
	else
	{
		int i;
		// Move in direction requested
		int children = context->GetChildCount();
		for ( i = 0; i < children; ++i )
		{
			Panel *child = context->GetChild( i );
			if ( child == current )
			{
				break;
			}
		}

		if ( i < children )
		{
			for ( int offset = 1; offset < children; ++offset )
			{
				int test = ( i + ( direction * offset ) ) % children;
				if ( test < 0 )
					test += children;
				if ( test == i )
					continue;

				Panel *check = context->GetChild( test );
				BuildModeDialog *bm = dynamic_cast< BuildModeDialog * >( check );
				if ( bm )
					continue;

				current = check;
				break;
			}
		}
	}

	if ( !current )
	{
		return;
	}

	SetActiveControl( current );
}
