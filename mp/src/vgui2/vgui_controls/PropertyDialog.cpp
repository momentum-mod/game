//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
PropertyDialog::PropertyDialog(Panel *parent, const char *panelName) : Frame(parent, panelName)
{
	// create the property sheet
	_propertySheet = new PropertySheet(this, "Sheet");
	_propertySheet->AddActionSignalTarget(this);
	_propertySheet->SetTabPosition(1);

	// add the buttons
	_okButton = new Button(this, "OKButton", "#PropertyDialog_OK", this, "OK");
	_okButton->SetTabPosition(2);
    _okButton->SetAutoWide(true);
    _okButton->SetAutoTall(true);
    _okButton->SetContentAlignment(Label::a_center);
    _okButton->SetTextInset(10, 0);
	GetFocusNavGroup().SetDefaultButton(_okButton);

	_cancelButton = new Button(this, "CancelButton", "#PropertyDialog_Cancel", this, "Cancel");
	_cancelButton->SetTabPosition(3);
    _cancelButton->SetAutoWide(true);
    _cancelButton->SetAutoTall(true);
    _cancelButton->SetContentAlignment(Label::a_center);
    _cancelButton->SetTextInset(10, 0);

	_applyButton = new Button(this, "ApplyButton", "#PropertyDialog_Apply", this, "Apply");
	_applyButton->SetTabPosition(4);
	_applyButton->SetVisible(false);		// default to not visible
    _applyButton->SetEnabled(false);        // default to not enabled
    _applyButton->SetAutoWide(true);
    _applyButton->SetAutoTall(true);
    _applyButton->SetTextInset(10, 0);
    _applyButton->SetContentAlignment(Label::a_center);

	SetSizeable(false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
PropertyDialog::~PropertyDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the PropertySheet this dialog encapsulates
// Output : PropertySheet *
//-----------------------------------------------------------------------------
PropertySheet *PropertyDialog::GetPropertySheet()
{
	return _propertySheet;
}

//-----------------------------------------------------------------------------
// Purpose: Gets a pointer to the currently active page.
// Output : Panel
//-----------------------------------------------------------------------------
Panel *PropertyDialog::GetActivePage()
{
	return _propertySheet->GetActivePage();
}

//-----------------------------------------------------------------------------
// Purpose: Wrapped function
//-----------------------------------------------------------------------------
void PropertyDialog::AddPage(Panel *page, const char *title)
{
	_propertySheet->AddPage(page, title);
}

//-----------------------------------------------------------------------------
// Purpose: reloads the data in all the property page
//-----------------------------------------------------------------------------
void PropertyDialog::ResetAllData()
{
	_propertySheet->ResetAllData();
}

//-----------------------------------------------------------------------------
// Purpose: Applies any changes
//-----------------------------------------------------------------------------
void PropertyDialog::ApplyChanges()
{
	OnCommand("Apply");
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the sheet
//-----------------------------------------------------------------------------
void PropertyDialog::PerformLayout()
{
	BaseClass::PerformLayout();

    UpdateButtonPositions();

	int iBottom = m_iSheetInsetBottom;
	if ( IsProportional() )
	{
		iBottom = scheme()->GetProportionalScaledValueEx( GetScheme(), iBottom );
	}

	int x, y, wide, tall;
	GetClientArea(x, y, wide, tall);
	_propertySheet->SetBounds(x, y, wide, tall - iBottom);

	_propertySheet->InvalidateLayout(); // tell the propertysheet to redraw!
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Handles command text from the buttons
//-----------------------------------------------------------------------------
void PropertyDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "OK"))
	{
		if ( OnOK(false) )
		{
			OnCommand("Close");
		}
		_applyButton->SetEnabled(false);
	}
	else if (!stricmp(command, "Cancel"))
	{
		OnCancel();
		Close();
	}
	else if (!stricmp(command, "Apply"))
	{
		OnOK(true);
		_applyButton->SetEnabled(false);
		InvalidateLayout();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: called when the Cancel button is pressed
//-----------------------------------------------------------------------------
void PropertyDialog::OnCancel()
{
	// designed to be overridden
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : code - 
//-----------------------------------------------------------------------------
void PropertyDialog::OnKeyCodeTyped(KeyCode code)
{
	// this has been removed, since it conflicts with how we use the escape key in the game
//	if (code == KEY_ESCAPE)
//	{
//		OnCommand("Cancel");
//	}
//	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Command handler
//-----------------------------------------------------------------------------
bool PropertyDialog::OnOK(bool applyOnly)
{
    // the sheet should have the pages apply changes before we tell the world
	_propertySheet->ApplyChanges();

    // this should tell anybody who's watching us that we're done
	PostActionSignal(new KeyValues("ApplyChanges"));

	// default to closing
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Overrides build mode so it edits the sub panel
//-----------------------------------------------------------------------------
void PropertyDialog::ActivateBuildMode()
{
	// no subpanel, no build mode
	EditablePanel *panel = dynamic_cast<EditablePanel *>(GetActivePage());
	if (!panel)
		return;

	panel->ActivateBuildMode();
}

//-----------------------------------------------------------------------------
// Purpose: sets the text on the OK/Cancel buttons, overriding the default
//-----------------------------------------------------------------------------
void PropertyDialog::SetOKButtonText(const char *text)
{
	_okButton->SetText(text);
}

//-----------------------------------------------------------------------------
// Purpose: sets the text on the OK/Cancel buttons, overriding the default
//-----------------------------------------------------------------------------
void PropertyDialog::SetCancelButtonText(const char *text)
{
	_cancelButton->SetText(text);
}

//-----------------------------------------------------------------------------
// Purpose: sets the text on the apply buttons, overriding the default
//-----------------------------------------------------------------------------
void PropertyDialog::SetApplyButtonText(const char *text)
{
	_applyButton->SetText(text);
}

//-----------------------------------------------------------------------------
// Purpose: changes the visibility of the buttons
//-----------------------------------------------------------------------------
void PropertyDialog::SetOKButtonVisible(bool state)
{
	_okButton->SetVisible(state);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: changes the visibility of the buttons
//-----------------------------------------------------------------------------
void PropertyDialog::SetCancelButtonVisible(bool state)
{
	_cancelButton->SetVisible(state);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: changes the visibility of the buttons
//-----------------------------------------------------------------------------
void PropertyDialog::SetApplyButtonVisible(bool state)
{
	_applyButton->SetVisible(state);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: when a sheet changes, enable the apply button
//-----------------------------------------------------------------------------
void PropertyDialog::OnApplyButtonEnable()
{
	if (_applyButton->IsEnabled())
		return;

	EnableApplyButton(true);
}

//-----------------------------------------------------------------------------
// Purpose: enable/disable the apply button
//-----------------------------------------------------------------------------
void PropertyDialog::EnableApplyButton(bool bEnable)
{
	_applyButton->SetEnabled(bEnable);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertyDialog::RequestFocus(int direction)
{
    _propertySheet->RequestFocus(direction);
}

void PropertyDialog::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    const char *pFontName = pScheme->GetResourceString("PropertyDialog.ButtonFont");
    if (pFontName && pFontName[0])
    {
        HFont font = pScheme->GetFont(pFontName, IsProportional());
        if (font)
        {
            _okButton->SetFont(font);
            _applyButton->SetFont(font);
            _cancelButton->SetFont(font);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Updates pinning & positions of OK/Cancel/Apply buttons based on their visibility
//-----------------------------------------------------------------------------
void PropertyDialog::UpdateButtonPositions()
{
    if (_applyButton->IsVisible())
    {
        _applyButton->PinToSibling("Sheet", PIN_TOPRIGHT, PIN_BOTTOMRIGHT);
        _applyButton->SetPos(0, 5);
    }

    if (_cancelButton->IsVisible())
    {
        if (!_applyButton->IsVisible())
        {
            _cancelButton->PinToSibling("Sheet", PIN_TOPRIGHT, PIN_BOTTOMRIGHT);
            _cancelButton->SetPos(0, 5);
        }
        else
        {
            _cancelButton->PinToSibling("ApplyButton", PIN_TOPRIGHT, PIN_TOPLEFT);
            _cancelButton->SetPos(5, 0);
        }
    }

    if (_okButton->IsVisible())
    {
        if (_cancelButton->IsVisible())
        {
            _okButton->PinToSibling("CancelButton", PIN_TOPRIGHT, PIN_TOPLEFT);
            _okButton->SetPos(5, 0);
        }
        else
        {
            if (_applyButton->IsVisible())
            {
                _okButton->PinToSibling("ApplyButton", PIN_TOPRIGHT, PIN_TOPLEFT);
                _okButton->SetPos(5, 0);
            }
            else
            {
                _okButton->PinToSibling("Sheet", PIN_TOPRIGHT, PIN_BOTTOMRIGHT);
                _okButton->SetPos(0, 5);
            }
        }
    }
}
