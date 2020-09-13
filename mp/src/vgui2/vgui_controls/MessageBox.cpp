//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/IInput.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/TextImage.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

vgui::Panel *MessageBox_Factory()
{
	return new MessageBox("MessageBox", "MessageBoxText");
}

DECLARE_BUILD_FACTORY_CUSTOM( MessageBox, MessageBox_Factory );


MessageBox::MessageBox(const char *title, const char *text, Panel *parent) : Frame(parent, "MessageBoxFrame", false)
{
	SetTitle(title, true);
	m_pMessageLabel = new Label(this, "MessageLabel", text);

	Init();
}

MessageBox::MessageBox(const wchar_t *wszTitle, const wchar_t *wszText, Panel *parent) : Frame(parent, "MessageBoxFrame", false)
{	
	SetTitle(wszTitle, true);
	m_pMessageLabel = new Label(this, "MessageLabel", wszText);

	Init();
}

void MessageBox::Init()
{
	SetDeleteSelfOnClose(true);
	m_pFrameOver = nullptr;
	m_bShowMessageBoxOverCursor = false;

	SetMenuButtonResponsive(false);
	SetMinimizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetProportional(true);
	
	m_pOkButton = new Button(this, "OkButton", "#MessageBox_OK");
	m_pOkButton->SetCommand( "OnOk" );
	m_pOkButton->AddActionSignalTarget(this);
	m_pOkButton->SetContentAlignment(Label::a_center);

	m_pCancelButton = new Button(this, "CancelButton", "#MessageBox_Cancel");
	m_pCancelButton->SetCommand( "OnCancel" );
	m_pCancelButton->AddActionSignalTarget(this);
	m_pCancelButton->SetVisible( false );
	m_pCancelButton->SetContentAlignment(Label::a_center);

	m_pOkCommand = m_pCancelCommand = nullptr;
	m_bAutoClose = true;
}

MessageBox::~MessageBox()
{
	if ( m_pOkCommand )
	{
		m_pOkCommand->deleteThis();
	}
	if ( m_pCancelCommand )
	{
		m_pCancelCommand->deleteThis();
	}
}

void MessageBox::ShowMessageBoxOverCursor( bool bEnable )
{
	m_bShowMessageBoxOverCursor = bEnable;
}

void MessageBox::OnCommand( const char *pCommand )
{
	if ( input()->GetAppModalSurface() == GetVPanel() )
	{
		input()->ReleaseAppModalSurface();
	}

	if ( !Q_stricmp( pCommand, "OnOk" ) )
	{
		if ( m_pOkCommand )
		{
			PostActionSignal(m_pOkCommand->MakeCopy());
		}
	}
	else if ( !Q_stricmp( pCommand, "OnCancel" ) || !Q_stricmp(pCommand, "Close") )
	{
		if ( m_pCancelCommand )
		{
			PostActionSignal(m_pCancelCommand->MakeCopy());
		}
	}

	if ( m_bAutoClose )
	{
		OnShutdownRequest();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Put the message box into a modal state
//			Does not suspend execution - use addActionSignal to get return value
//-----------------------------------------------------------------------------
void MessageBox::DoModal(Panel* pFrameOver)
{
    ShowWindow(pFrameOver);

	input()->SetAppModalSurface(GetVPanel());
}

void MessageBox::ShowWindow(Panel *pFrameOver)
{
	m_pFrameOver = pFrameOver;

	SetVisible( true );
	SetEnabled( true );
	MoveToFront();

	if ( m_pOkButton->IsVisible() )
	{
		m_pOkButton->RequestFocus();
	}
	else	 // handle message boxes with no button
	{
		RequestFocus();
	}

	InvalidateLayout();
}

void MessageBox::PerformLayout()
{
	int x, y, wide, tall;
	GetClientArea(x, y, wide, tall);
	wide += x;
	tall += y;

	int boxWidth, boxTall;
	GetSize(boxWidth, boxTall);

	const auto pTitle = GetTitle();
	int titleWide, titleTall;
	pTitle->GetContentSize(titleWide, titleTall);

	boxWidth = max(boxWidth, titleWide);

	int oldWide, oldTall;
	m_pOkButton->GetSize(oldWide, oldTall);
	
	int btnWide, btnTall;
	m_pOkButton->GetContentSize(btnWide, btnTall);
	btnWide = max(oldWide, btnWide + 10);
	btnTall = max(oldTall, btnTall + 10);
	m_pOkButton->SetSize(btnWide, btnTall);

	int btnWide2 = 0, btnTall2 = 0;
	if ( m_pCancelButton->IsVisible() )
	{
		m_pCancelButton->GetSize(oldWide, oldTall);
		
		m_pCancelButton->GetContentSize(btnWide2, btnTall2);
		btnWide2 = max(oldWide, btnWide2 + 10);
		btnTall2 = max(oldTall, btnTall2 + 10);
		m_pCancelButton->SetSize(btnWide2, btnTall2);
	}

	boxWidth = max(boxWidth, m_pMessageLabel->GetWide() + GetScaledVal(50));
	boxWidth = max(boxWidth, (btnWide + btnWide2) * 2 + 30);
	SetSize(boxWidth, boxTall);

	m_pMessageLabel->SetPos((wide/2)-(m_pMessageLabel->GetWide()/2) + x, y + 5 );
	if ( !m_pCancelButton->IsVisible() )
	{
		m_pOkButton->SetPos((wide/2)-(m_pOkButton->GetWide()/2) + x, tall - m_pOkButton->GetTall() - GetScaledVal(15));
	}
	else
	{
		m_pOkButton->SetPos((wide/4)-(m_pOkButton->GetWide()/2) + x, tall - m_pOkButton->GetTall() - GetScaledVal(15));
		m_pCancelButton->SetPos((3*wide/4)-(m_pOkButton->GetWide()/2) + x, tall - m_pOkButton->GetTall() - GetScaledVal(15));
	}

	m_pMessageLabel->GetContentSize(wide, tall);
	m_pMessageLabel->SetSize(wide, tall);

	wide += GetScaledVal(75);
	tall += GetScaledVal(75);
	SetSize(wide, tall);

	if (m_bShowMessageBoxOverCursor)
	{
		PlaceUnderCursor();
		return;
	}

	if (m_pFrameOver)
	{
		int frameX, frameY;
		int frameWide, frameTall;
		m_pFrameOver->GetPos(frameX, frameY);
		m_pFrameOver->GetSize(frameWide, frameTall);

		SetPos((frameWide - wide) / 2 + frameX, (frameTall - tall) / 2 + frameY);
	}
	else
	{
		MoveToCenterOfScreen();
	}

	BaseClass::PerformLayout();
}

void MessageBox::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ENTER)
	{
		OnCommand("OnOk");
	}
	else if (code == KEY_ESCAPE)
	{
	    OnCommand("OnCancel");
	}
	else
	{
	    BaseClass::OnKeyCodeTyped(code);
	}
}

void MessageBox::SetCommand(const char *command)
{
	if (m_pOkCommand)
	{
		m_pOkCommand->deleteThis();
	}
	m_pOkCommand = new KeyValues("Command", "command", command);
}

void MessageBox::SetCommand(KeyValues *command)
{
	if (m_pOkCommand)
	{
		m_pOkCommand->deleteThis();
	}
	m_pOkCommand = command;
}

void MessageBox::OnShutdownRequest()
{
	Close();
}

void MessageBox::SetOKButtonVisible(bool state)
{
	m_pOkButton->SetVisible(state);
}

void MessageBox::SetOKButtonText(const char *buttonText)
{
	m_pOkButton->SetText(buttonText);
	InvalidateLayout();
}

void MessageBox::SetOKButtonText(const wchar_t *wszButtonText)
{
	m_pOkButton->SetText(wszButtonText);
	InvalidateLayout();
}

void MessageBox::SetCancelButtonVisible(bool state)
{
	m_pCancelButton->SetVisible(state);
	InvalidateLayout();
}

void MessageBox::SetCancelButtonText(const char *buttonText)
{
	m_pCancelButton->SetText(buttonText);
	InvalidateLayout();
}

void MessageBox::SetCancelButtonText(const wchar_t *wszButtonText)
{
	m_pCancelButton->SetText(wszButtonText);
	InvalidateLayout();
}

void MessageBox::SetCancelCommand( KeyValues *command )
{
	if (m_pCancelCommand)
	{
		m_pCancelCommand->deleteThis();
	}
	m_pCancelCommand = command;
}

void MessageBox::DisableCloseButton(bool state)
{
	BaseClass::SetCloseButtonVisible(state);

	m_bAutoClose = false;
}