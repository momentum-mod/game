#pragma once

#include <vgui_controls/SectionedListPanel.h>

//-----------------------------------------------------------------------------
// Purpose: Special list subclass to handle drawing of trap mode prompt on top of
//			lists client area
//-----------------------------------------------------------------------------
class VControlsListPanel : public vgui::SectionedListPanel
{
public:
	// Construction
					VControlsListPanel( vgui::Panel *parent, const char *listName );
	virtual			~VControlsListPanel();

	// Start/end capturing
	virtual void	StartCaptureMode(vgui::HCursor hCursor = NULL);
	virtual void	EndCaptureMode(vgui::HCursor hCursor = NULL);
	virtual bool	IsCapturing();

	// Set which item should be associated with the prompt
	virtual void	SetItemOfInterest(int itemID);
	virtual int		GetItemOfInterest();

	virtual void	OnMousePressed(vgui::MouseCode code);
	virtual void	OnMouseDoublePressed(vgui::MouseCode code);
	void OnKeyCodeTyped(vgui::KeyCode code) override;

private:
	// Are we showing the prompt?
	bool			m_bCaptureMode;
	// If so, where?
	int				m_nClickRow;
	// panel used to edit
	class CInlineEditPanel *m_pInlineEditPanel;
	int m_iMouseX, m_iMouseY;

	typedef SectionedListPanel BaseClass;
};