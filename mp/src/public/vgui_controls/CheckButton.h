//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CHECKBUTTON_H
#define CHECKBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/TextImage.h>

namespace vgui
{

class TextImage;

//-----------------------------------------------------------------------------
// Purpose: Check box image
//-----------------------------------------------------------------------------
class CheckImage : public TextImage
{
public:
    CheckImage(CheckButton *CheckButton);

    void Paint() override;

	void SetBkColor(Color color) override { _bgColor = color; }
	void SetCheckColor(Color checkColor) { _checkColor = checkColor; }

	void SetBorder(IBorder *pBorder) { _border = pBorder; }

private:
	Color _checkColor;
	Color _bgColor;

	IBorder *_border;
	CheckButton *_CheckButton;
};

//-----------------------------------------------------------------------------
// Purpose: Tick-box button
//-----------------------------------------------------------------------------
class CheckButton : public ToggleButton
{
	DECLARE_CLASS_SIMPLE( CheckButton, ToggleButton );

public:
	CheckButton(Panel *parent, const char *panelName, const char *text);
	~CheckButton();

	// Check the button
    virtual void SetSelected(bool state);
    virtual void SilentSetSelected(bool state);

	// sets whether or not the state of the check can be changed
	// if this is set to false, then no input in the code or by the user can change it's state
	virtual void SetCheckButtonCheckable(bool state);
	virtual bool IsCheckButtonCheckable() const { return m_bCheckButtonCheckable; }

	Color GetDisabledFgColor() { return _disabledFgColor; }
	Color GetDisabledBgColor() { return _disabledBgColor; }

	CheckImage *GetCheckImage() { return _checkBoxImage; }

	void SetCheckInset(int inset) { m_iCheckInset = inset; }

	virtual void SetHighlightColor(Color fgColor);

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );
	virtual Color GetButtonFgColor();

	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus);

	/* MESSAGES SENT
		"CheckButtonChecked" - sent when the check button state is changed
			"state"	- button state: 1 is checked, 0 is unchecked
	*/


private:
    void SetSelected(bool state, bool bPostActionSignal);

	int m_iCheckInset;
	bool m_bCheckButtonCheckable;
	CheckImage *_checkBoxImage;
	Color _disabledFgColor;
	Color _disabledBgColor;
	Color _highlightFgColor;
};

} // namespace vgui

#endif // CHECKBUTTON_H
