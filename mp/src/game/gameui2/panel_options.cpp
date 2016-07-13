#include "gameui2_interface.h"
#include "panel_options.h"

#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

#include <string>
#include <algorithm>
#include <functional>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Panel_Options::Panel_Options(vgui::VPANEL parent, const char* pName) : BaseClass(NULL, "")
{
	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("resource2/schemepanel.res", "SchemePanel");
	SetScheme(Scheme);

	m_pAnimController = new vgui::AnimationController(this);
	m_pAnimController->SetScheme(Scheme);
	m_pAnimController->SetProportional(false);

	SetProportional(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetDeleteSelfOnClose(true);
	SetTitleBarVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetEnabled(true);
	SetVisible(false);
	SetParent(parent);

	Activate();

	m_PanelTitle = GameUI2().GetLocalizedString("#GameUI2_TitleOptions");

	m_pBtnDone = new Button_Panel(this, this, "");
	m_pBtnDone->SetButtonText("#GameUI2_Done");
	m_pBtnDone->SetButtonDescription("");

	m_pBtnApply = new Button_Panel(this, this, "");
	m_pBtnApply->SetButtonText("#GameUI2_Apply");
	m_pBtnApply->SetButtonDescription("");

	m_pBtnBack = new Button_Panel(this, this, "action_back");
	m_pBtnBack->SetButtonText("#GameUI2_Back");
	m_pBtnBack->SetButtonDescription("");
}

void Panel_Options::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_cBackgroundGradientTop = GetSchemeColor("Panel.Background.Gradient.Top", pScheme);
	m_cBackgroundGradientBottom = GetSchemeColor("Panel.Background.Gradient.Bottom", pScheme);

	m_cTitleColor = GetSchemeColor("Panel.Title.Color", pScheme);

	m_fTitleOffsetX = atof(pScheme->GetResourceString("Panel.Title.OffsetX"));
	m_fTitleOffsetY = atof(pScheme->GetResourceString("Panel.Title.OffsetY"));

	m_fTitleFont = pScheme->GetFont("Panel.Title.Font");
}

void Panel_Options::Animations()
{
	if (m_pAnimController != nullptr)
		m_pAnimController->UpdateAnimations(GameUI2().GetTime()); // (gpGlobals->realtime);

	SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);
}

void Panel_Options::OnThink()
{
	BaseClass::OnThink();

	SetContentBounds();
	Animations();

	if (IsVisible() == false)
	{
		ConColorMsg(Color(0, 148, 255, 255), "Options panel is not visible, running all animations to completion...\n");

		if (m_pAnimController != nullptr)
			m_pAnimController->RunAllAnimationsToCompletion();
	}
}

void Panel_Options::SetContentBounds()
{	
	m_iContentW = (GameUI2().GetViewport().x / 100) * 75;
	if (m_iContentW > 1920)
		m_iContentW = 1920;
	else if(m_iContentW < 800)
		m_iContentW = 800;

	m_iContentH = (GameUI2().GetViewport().y / 100) * 75;
	if (m_iContentH > 1080)
		m_iContentH = 1080;
	else if(m_iContentH < 600)
		m_iContentH = 600;

	m_iContentX0 = GameUI2().GetViewport().x / 2 - m_iContentW / 2;
	m_iContentY0 = GameUI2().GetViewport().y / 2 - m_iContentH / 2;

	m_iContentX1 = m_iContentX0 + m_iContentW;
	m_iContentY1 = m_iContentY0 + m_iContentH;
}

void Panel_Options::Paint()
{
	BaseClass::Paint();

	DrawBackground();
	DrawTitle();
	DrawTabs();
	DrawBasicButtons();
}

void Panel_Options::PaintBlurMask()
{
	BaseClass::PaintBlurMask();
}

void Panel_Options::DrawBackground()
{
	vgui::surface()->DrawSetColor(m_cBackgroundGradientTop);
	vgui::surface()->DrawFilledRectFade(0, 0, GameUI2().GetViewport().x + 0, GameUI2().GetViewport().y + 0, 255, 0, false);

	vgui::surface()->DrawSetColor(m_cBackgroundGradientBottom);
	vgui::surface()->DrawFilledRectFade(0, 0, GameUI2().GetViewport().x + 0, GameUI2().GetViewport().y + 0, 0, 255, false);
}


void Panel_Options::DrawTitle()
{
	std::wstring panel_title = m_PanelTitle;
	std::transform(panel_title.begin(), panel_title.end(), panel_title.begin(), std::function<int32(int32)>(::toupper));
	
	vgui::surface()->DrawSetTextColor(m_cTitleColor);
	vgui::surface()->DrawSetTextFont(m_fTitleFont);

	vgui::surface()->GetTextSize(m_fTitleFont, panel_title.data(), m_iTitleSizeX, m_iTitleSizeY);
	m_iTitlePositionX = m_iContentX0 + m_fTitleOffsetX;
	m_iTitlePositionY = m_iContentY0 + m_fTitleOffsetY;
	
	vgui::surface()->DrawSetTextPos(m_iTitlePositionX, m_iTitlePositionY);
	vgui::surface()->DrawPrintText(panel_title.data(), wcslen(panel_title.data()));
}

void Panel_Options::DrawTabs()
{
	// TEST!
	int32 x0, y0;
	m_pBtnDone->GetPos(x0, y0);
	
	int8 item_height = 48;
	int16 content_height = y0 - (m_iTitlePositionY - m_fTitleOffsetY + m_iTitleSizeY);
	int8 items_per_height = content_height / item_height;

	for (int8 i = 0; i < items_per_height; i++)
	{
		if (i % 2)
			vgui::surface()->DrawSetColor(Color(0, 0, 0, 20));
		else
			vgui::surface()->DrawSetColor(Color(255, 255, 255, 1));

		int32 item_y = m_iTitlePositionY + m_iTitleSizeY + (item_height * (i + 1)) - item_height;
		vgui::surface()->DrawFilledRect(m_iContentX0, item_y, m_iContentW + m_iContentX0, item_height + item_y);
	}
	// TEST!
}

void Panel_Options::DrawBasicButtons()
{
	m_pBtnDone->SetPos(m_iContentX0, m_iContentY1 - m_pBtnDone->GetHeight());
	m_pBtnDone->SetVisible(true);

	int32 x0, y0;
	m_pBtnDone->GetPos(x0, y0);
	
	m_pBtnApply->SetPos(x0 + m_pBtnDone->GetWidth(), m_iContentY1 - m_pBtnApply->GetHeight());
	m_pBtnApply->SetVisible(true);
	
	m_pBtnBack->SetPos(m_iContentX1 - m_pBtnBack->GetWidth(), m_iContentY1 - m_pBtnBack->GetHeight());
	m_pBtnBack->SetVisible(true);
}

void Panel_Options::OnCommand(char const* cmd)
{
	if (!Q_stricmp(cmd, "action_back"))
		Close();
	else
		BaseClass::OnCommand(cmd);
}

CON_COMMAND(gameui2_openoptionsdialog, "")
{
	new Panel_Options(GameUI2().GetVPanel(), "");
}