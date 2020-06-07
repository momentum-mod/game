//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "vgui_controls/ScrollableEditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/ScrollBarSlider.h"
#include "vgui_controls/Button.h"
#include "KeyValues.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

using namespace vgui;

Panel *CreateScrollableEditablePanel()
{
    return new ScrollableEditablePanel( nullptr, nullptr, nullptr );
}

DECLARE_BUILD_FACTORY_CUSTOM(ScrollableEditablePanel, CreateScrollableEditablePanel);

ScrollableEditablePanel::ScrollableEditablePanel( Panel *pParent, EditablePanel *pChild, const char *pName ) : BaseClass( pParent, pName )
{
	m_pScrollBar = new ScrollBar( this, "VerticalScrollBar", true ); 
	m_pScrollBar->SetWide( 16 );
	m_pScrollBar->SetAutoResize( PIN_TOPRIGHT, AUTORESIZE_DOWN, 0, 0, -16, 0 );
	m_pScrollBar->AddActionSignalTarget( this );

	SetChild(pChild);
}

void ScrollableEditablePanel::SetChild(Panel *pChild)
{
	m_pChild = pChild;

	if (m_pChild)
	{
	    m_pChild->SetParent(this);
		m_pScrollBar->SetZPos(m_pChild->GetZPos() + 1);
	}

	ScrollToTop();
	InvalidateLayout();
}

void ScrollableEditablePanel::ApplySettings( KeyValues *pInResourceData )
{
	BaseClass::ApplySettings( pInResourceData );

	KeyValues *pScrollbarKV = pInResourceData->FindKey( "Scrollbar" );
	if ( pScrollbarKV )
	{
		m_pScrollBar->ApplySettings( pScrollbarKV );
	}
}

void ScrollableEditablePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	if (m_pChild)
	{
		m_pChild->SetWide(GetWide());
		m_pScrollBar->SetRange(0, m_pChild->GetTall());
		m_pScrollBar->SetRangeWindow(GetTall());
	}
}

void ScrollableEditablePanel::ScrollToTop()
{
    m_pScrollBar->SetValue(0);
	// Guarantee the child sets its position
	if (m_pChild)
	{
	    m_pChild->SetPos(0, 0);
	}
    Repaint();
}

//-----------------------------------------------------------------------------
// Called when the scroll bar moves
//-----------------------------------------------------------------------------
void ScrollableEditablePanel::OnScrollBarSliderMoved()
{
	InvalidateLayout();

	if (m_pChild)
	{
		int nScrollAmount = m_pScrollBar->GetValue();
		m_pChild->SetPos(0, -nScrollAmount);
	}
}

//-----------------------------------------------------------------------------
// respond to mouse wheel events
//-----------------------------------------------------------------------------
void ScrollableEditablePanel::OnMouseWheeled(int delta)
{
	int val = m_pScrollBar->GetValue();
	val -= (delta * 50);
	m_pScrollBar->SetValue( val );
}