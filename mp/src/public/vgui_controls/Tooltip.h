//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Creates a Message box with a question in it and yes/no buttons
//
// $NoKeywords: $
//=============================================================================//

#ifndef TOOLTIP_H
#define TOOLTIP_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Controls.h>
#include <utlvector.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Tooltip for a panel - shows text when cursor hovers over a panel
//-----------------------------------------------------------------------------
class BaseTooltip
{
public:
	BaseTooltip(Panel *parent, const char *text = NULL);
	virtual ~BaseTooltip() = default;

	virtual void SetText(const char *text);
	virtual const char *GetText();
	virtual void SetText(const wchar_t *text);

	virtual void ShowTooltip(Panel *currentPanel);
	virtual void HideTooltip();

	bool		 ShouldLayout( void );
	virtual void PerformLayout() { return; }
	virtual void PositionWindow( void );

	void ResetDelay();
	void SetTooltipFormatToSingleLine();
	void SetTooltipFormatToMultiLine();
	void SetTooltipDelay(int tooltipDelayMilliseconds);
	int GetTooltipDelay();
	void SetEnabled( bool bState );
    bool IsVisible() { return _visible; }

private:
	Panel *m_pParent;
	virtual void ApplySchemeSettings(HScheme hScheme) {};
protected:
	wchar_t *m_wText;
	CUtlVector<char> m_Text;
	int _delay;			// delay that counts down
	int _tooltipDelay;	// delay before tooltip comes up.
	bool _makeVisible : 1;
	bool _displayOnOneLine : 1;
	bool _isDirty : 1;
	bool _enabled : 1;
    bool _visible : 1;
};

class TextTooltip : public BaseTooltip
{
public:
	TextTooltip(Panel *parent, const char *text = NULL);
	~TextTooltip();

	virtual void SetText(const char *text);
	virtual void SetText(const wchar_t *text) override;
	virtual void ShowTooltip(Panel *currentPanel);
	virtual void HideTooltip();
	virtual void SizeTextWindow();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(HScheme hScheme);
};

};

#endif // TOOLTIP_H
