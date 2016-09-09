#include "hudelement.h"
#include <vgui_controls/Panel.h>

#ifdef _WIN32
#pragma once
#endif

namespace vgui
{
	class Button;
	class CheckButton;
	class Label;
	class ProgressBar;
	class FileOpenDialog;
	class Slider;
};

namespace vgui
{
	//-----------------------------------------------------------------------------
	// Purpose: overrides normal button drawing to use different colors & borders
	//-----------------------------------------------------------------------------
	class OnClickButton : public Button
	{
		DECLARE_CLASS_SIMPLE(OnClickButton, Button);

	public:

		Color _selectedColor;

		OnClickButton(Panel *parent, const char *name, const char *text) : Button(parent, name, text)
		{
			Button::Button(parent, name, text);
		}

		virtual void ApplySchemeSettings(IScheme *pScheme)
		{
			Button::ApplySchemeSettings(pScheme);
			_selectedColor = GetSchemeColor("ToggleButton.SelectedTextColor", pScheme);
		}

		virtual void OnClick()
		{
			// post a button toggled message
			KeyValues *msg = new KeyValues("ButtonToggled");
			msg->SetInt("state", (int)IsSelected());
			PostActionSignal(msg);

			Repaint();
		}

		virtual Color GetButtonFgColor()
		{
			if (IsSelected())
			{
				// highlight the text when depressed
				return _selectedColor;
			}
			else
			{
				return BaseClass::GetButtonFgColor();
			}
		}

		virtual void PerformLayout()
		{
			Button::PerformLayout();
		}

		// Don't request focus.
		// This will keep items in the listpanel selected.
		virtual void OnMousePressed(MouseCode code)
		{
			if (!IsEnabled())
				return;

			if (!IsMouseClickEnabled(code))
				return;

			if (IsUseCaptureMouseEnabled())
			{
				SetSelected(true);
			}
		}
	};


} // namespace vgui

class CHudReplay : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CHudReplay, vgui::Frame);

public:
	CHudReplay(const char *pElementName);

	virtual void OnTick();

	// Command issued
	virtual void OnCommand(const char *command);

	// player controls
	vgui::Button	*m_pPlayPauseResume;
	vgui::Button	*m_pGoStart;
	vgui::Button	*m_pGoEnd;
	vgui::Button	*m_pPrevFrame;
	vgui::Button	*m_pNextFrame;
	vgui::Button	*m_pFastForward;
	vgui::Button	*m_pFastBackward;
	vgui::Button	*m_pGo;

	vgui::ProgressBar *m_pProgress;
	vgui::Label	*m_pProgressLabelFrame;
	vgui::Label	*m_pProgressLabelTime;

	vgui::Slider *m_pSpeedScale;
	vgui::Label	*m_pSpeedScaleLabel;
	vgui::TextEntry *m_pGotoTick;

	bool		m_bInputActive;
	int			m_nOldCursor[2];
	
}; extern CHudReplay *HudReplay;

