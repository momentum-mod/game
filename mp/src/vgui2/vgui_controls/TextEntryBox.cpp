// Author: Matthew D. Campbell (matt@turtlerockstudios.com), 2003

#include "KeyValues.h"

#include "vgui_controls/TextEntryBox.h"
#include "vgui_controls/CvarTextEntry.h"
#include <vgui_controls/TextEntry.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


using namespace vgui;

TextEntryBox::TextEntryBox(const char *title, const char *queryText, const char *entryText, bool isCvar, Panel *parent /*= nullptr*/) : QueryBox(title, queryText, parent)
{
	SetProportional(true);

	if (isCvar)
	{
		m_pEntry = m_pCvarEntry = new CvarTextEntry( this, "TextEntry", entryText );
	}
	else
	{
		m_pEntry = new TextEntry( this, "TextEntry" );
		m_pEntry->SetText(entryText);
		m_pCvarEntry = nullptr;
	}

	m_pEntry->SetTabPosition(3);
	m_pEntry->GotoTextEnd();
	m_pEntry->SelectAllOnFocusAlways(true);

	SetCancelButtonVisible(true);
}

void TextEntryBox::ShowWindow(Panel *pFrameOver)
{
	BaseClass::ShowWindow( pFrameOver );

	m_pEntry->RequestFocus();

	InvalidateLayout();
}

void TextEntryBox::PerformLayout()
{
	BaseClass::PerformLayout();

	int x, y, wide, tall;
	GetClientArea(x, y, wide, tall);
	wide += x;
	tall += y;

	const int borderW = GetScaledVal(10);

	int labelW, labelH;
	m_pMessageLabel->GetSize( labelW, labelH );

	int entryW = max(GetScaledVal(10), wide - (2*borderW) - labelW);
	int entryH = max(GetScaledVal(16), labelH);
	m_pEntry->SetSize( entryW, entryH );

	int boxWidth, boxTall;
	GetSize(boxWidth, boxTall);
	if (boxWidth < labelW + entryW + borderW * 2)
		SetSize( labelW + entryW + borderW * 2, boxTall );

	m_pMessageLabel->GetPos( x, y );
	m_pMessageLabel->SetPos( borderW, y - (entryH - labelH)/2 );

	m_pEntry->SetPos( borderW + m_pMessageLabel->GetWide() + borderW, y - (entryH - labelH) );
}

void TextEntryBox::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "OnOk"))
	{
		if (m_pOkCommand)
		{
			char buf[512];
			m_pEntry->GetText(buf, sizeof(buf));

			m_pOkCommand->SetString("value", buf);
		}

		if (m_pCvarEntry)
		{
			m_pCvarEntry->ApplyChanges();
		}
	}

	BaseClass::OnCommand(command);
}