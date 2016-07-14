#pragma once

#include "vgui2d/frame2d.h"
#include "vgui_controls/AnimationController.h"

#include "button_panel.h"

class Panel_Options : public Frame2D
{
	DECLARE_CLASS_SIMPLE(Panel_Options, Frame2D);

public:
	Panel_Options(vgui::VPANEL parent, const char* pName);

	virtual void		ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual void 		OnThink();
	virtual void 		SetContentBounds();
	virtual void 		Paint();
	virtual void 		PaintBlurMask();
	virtual void		Animations();
	virtual void 		DrawBackground();
	virtual void 		DrawTitle();
	virtual void 		DrawTabs();
	virtual void 		DrawBasicButtons();
	virtual void		OnCommand(char const* cmd);

private:
	vgui::AnimationController* m_pAnimController;

	Button_Panel*		m_pBtnApply;
	Button_Panel*		m_pBtnDone;
	Button_Panel*		m_pBtnBack;

	int32				m_iContentX0;
	int32				m_iContentY0;
	int32				m_iContentX1;
	int32				m_iContentY1;
	int32				m_iContentW;
	int32				m_iContentH;

	wchar_t*			m_PanelTitle;

	int32				m_iTitlePositionX;
	int32				m_iTitlePositionY;
	int32				m_iTitleSizeX;
	int32				m_iTitleSizeY;

	Color				m_cBackgroundGradientTop;
	Color				m_cBackgroundGradientBottom;

	Color				m_cTitleColor;

	float				m_fTitleOffsetX;
	float				m_fTitleOffsetY;

	vgui::HFont			m_fTitleFont;
};