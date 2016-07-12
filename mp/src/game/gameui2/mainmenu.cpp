#include "gameui2_interface.h"
#include "basepanel.h"
#include "mainmenu.h"

#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui/IVGui.h"
#include "vgui_controls/ImagePanel.h"

#include "KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

MainMenu::MainMenu(vgui::Panel* parent) : BaseClass(NULL, "MainMenu")
{
	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("resource2/schememainmenu.res", "SchemeMainMenu");
	SetScheme(Scheme);

	SetProportional(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetDeleteSelfOnClose(true);
	SetSizeable(false);
	SetMoveable(false);
	SetCloseButtonVisible(false);
	SetMenuButtonVisible(false);
	Activate();

	m_bFocused = true;

	m_logoLeft = GameUI2().GetLocalizedString("#GameUI2_LogoLeft");
	m_logoRight = GameUI2().GetLocalizedString("#GameUI2_LogoRight");
    m_pLogoImage = nullptr;
    m_pButtonFeedback = new Button_MainMenu(this, this, "engine mom_contact_show");
    m_pButtonFeedback->SetButtonText("#GameUI2_SendFeedback");
    m_pButtonFeedback->SetButtonDescription("#GameUI2_SendFeedbackDescription");
    m_pButtonFeedback->SetPriority(1);
    m_pButtonFeedback->SetBlank(false);
    m_pButtonFeedback->SetVisible(true);

	CreateMenu("resource2/mainmenu.res");
}

void MainMenu::CreateMenu(const char* menu)
{
	KeyValues* datafile = new KeyValues("MainMenu");
	datafile->UsesEscapeSequences(true);
	if (datafile->LoadFromFile(g_pFullFileSystem, menu))
	{
        FOR_EACH_SUBKEY(datafile, dat)
        {
            Button_MainMenu* button = new Button_MainMenu(this, this, dat->GetString("command", ""));
            button->SetPriority(dat->GetInt("priority", 1));
            button->SetButtonText(dat->GetString("text", "no text"));
            button->SetButtonDescription(dat->GetString("description", "no description"));
            button->SetBlank(dat->GetBool("blank"));

            const char* specifics = dat->GetString("specifics", "shared");
            if (Equals(specifics, "ingame"))
                m_pButtonsInGame.AddToTail(button);
            else if (Equals(specifics, "mainmenu"))
                m_pButtonsBackground.AddToTail(button);
            else if (Equals(specifics, "shared"))
                m_pButtonsShared.AddToTail(button);
        }
	}

    datafile->deleteThis();
}

int32 __cdecl ButtonsPositionBottom(Button_MainMenu* const* s1, Button_MainMenu* const* s2)
{
	return ((*s1)->GetPriority() > (*s2)->GetPriority());
}

int32 __cdecl ButtonsPositionTop(Button_MainMenu* const* s1, Button_MainMenu* const* s2)
{
	return ((*s1)->GetPriority() < (*s2)->GetPriority());
}

void MainMenu::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_fButtonsSpace = atof(pScheme->GetResourceString("MainMenu.Buttons.Space"));
	m_fButtonsOffsetX = atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetX"));
	m_fButtonsOffsetY = atof(pScheme->GetResourceString("MainMenu.Buttons.OffsetY"));
	m_fLogoOffsetX = atof(pScheme->GetResourceString("MainMenu.Logo.OffsetX"));
	m_fLogoOffsetY = atof(pScheme->GetResourceString("MainMenu.Logo.OffsetY"));
	m_cLogoLeft = GetSchemeColor("MainMenu.Logo.Left", pScheme);
	m_cLogoRight = GetSchemeColor("MainMenu.Logo.Right", pScheme);
	m_bLogoAttachToMenu = atoi(pScheme->GetResourceString("MainMenu.Logo.AttachToMenu"));
    m_bLogoText = atoi(pScheme->GetResourceString("MainMenu.Logo.Text"));
    if (!m_bLogoText)
    {
        if (m_pLogoImage)
            m_pLogoImage->EvictImage();
        else
            m_pLogoImage = new vgui::ImagePanel(this, "GameLogo");

        m_pLogoImage->SetAutoDelete(true);
        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        int imageW = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width")));
        int imageH = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height")));
        m_pLogoImage->SetSize(imageW, imageH);
        //Pos is handled in Paint()
    }
	m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    Q_strncpy(m_pszMenuOpenSound, pScheme->GetResourceString("MainMenu.Sound.Open"), sizeof(m_pszMenuOpenSound));
    Q_strncpy(m_pszMenuCloseSound, pScheme->GetResourceString("MainMenu.Sound.Close"), sizeof(m_pszMenuCloseSound));
}

void MainMenu::OnThink()
{
	BaseClass::OnThink();

    if (vgui::ipanel())
        SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);
}

bool MainMenu::IsVisible(void)
{
	if (GameUI2().IsInLoading())
		return false;

	return m_bFocused;
}

void MainMenu::DrawMainMenu()
{
	for (int8 i = 0; i < m_pButtonsInGame.Count(); i++)
		m_pButtonsInGame[i]->SetVisible(GameUI2().IsInLevel());

	for (int8 i = 0; i < m_pButtonsBackground.Count(); i++)
		m_pButtonsBackground[i]->SetVisible(GameUI2().IsInBackgroundLevel());

	for (int8 i = 0; i < m_pButtonsShared.Count(); i++)
		m_pButtonsShared[i]->SetVisible(GameUI2().IsInLevel() || GameUI2().IsInBackgroundLevel());

	CUtlVector<Button_MainMenu*> activeButtons;
	if (GameUI2().IsInLevel())
		activeButtons.AddVectorToTail(m_pButtonsInGame);
	else if (GameUI2().IsInBackgroundLevel())
		activeButtons.AddVectorToTail(m_pButtonsBackground);
	activeButtons.AddVectorToTail(m_pButtonsShared);

	m_pButtons = activeButtons;
	
	if (m_pButtons.Count() > 0)
		m_pButtons.Sort(ButtonsPositionTop);

	for (int8 i = 0; i < m_pButtons.Count(); i++)
	{
		if ((i + 1) < m_pButtons.Count())
		{
			int32 x0, y0;
			m_pButtons[i + 1]->GetPos(x0, y0);
			m_pButtons[i]->SetPos(m_fButtonsOffsetX, y0 - (m_pButtons[i]->GetHeight() + m_fButtonsSpace));
		}
		else
		{
            m_pButtons[i]->SetPos(m_fButtonsOffsetX, GameUI2().GetViewport().y - (m_fButtonsOffsetY + m_pButtons[i]->GetHeight()));
		}

		m_pButtons[i]->SetVisible(true);
	}

    //MOM_TODO: Remove this after it's not needed
    if (m_pButtonFeedback)
    {
        m_pButtonFeedback->SetPos(GameUI2().GetViewport().x - m_pButtonFeedback->GetWidth(), 
            GameUI2().GetViewport().y - (m_pButtonFeedback->GetHeight() + m_fButtonsOffsetY));
    }
}

void MainMenu::DrawLogo()
{
    if (m_bLogoText)
    {
        vgui::surface()->DrawSetTextColor(m_cLogoLeft);
        vgui::surface()->DrawSetTextFont(m_fLogoFont);

        int32 logoW, logoH;
        vgui::surface()->GetTextSize(m_fLogoFont, m_logoLeft, logoW, logoH);

        int32 logoX, logoY;
        if (m_pButtons.Count() <= 0 || m_bLogoAttachToMenu == false)
        {
            logoX = m_fLogoOffsetX;
            logoY = 0;//GameUI2().GetViewport().y - (m_fLogoOffsetY + logoH);
        }
        else
        {
            int32 x0, y0;
            m_pButtons[0]->GetPos(x0, y0);
            logoX = m_fButtonsOffsetX + m_fLogoOffsetX;
            logoY = y0 - (logoH + m_fLogoOffsetY);
        }
        vgui::surface()->DrawSetTextPos(logoX, logoY);
        vgui::surface()->DrawPrintText(m_logoLeft, wcslen(m_logoLeft));

        vgui::surface()->DrawSetTextColor(m_cLogoRight);
        vgui::surface()->DrawSetTextFont(m_fLogoFont);
        vgui::surface()->DrawSetTextPos(logoX + logoW, logoY);
        vgui::surface()->DrawPrintText(m_logoRight, wcslen(m_logoRight));
    }
    else
    {
        //This is a logo, but let's make sure it's still valid
        if (m_pLogoImage)
        {
            int logoX, logoY;
            if (m_pButtons.Count() <= 0 || m_bLogoAttachToMenu == false)
            {
                logoX = m_fLogoOffsetX;
                logoY = m_fLogoOffsetY;
            }
            else
            {
                int32 x0, y0;
                m_pButtons[0]->GetPos(x0, y0);
                logoX = m_fButtonsOffsetX + m_fLogoOffsetX;
                int imageHeight, dummy;
                m_pLogoImage->GetImage()->GetSize(dummy, imageHeight);
                logoY = y0 - (imageHeight + m_fLogoOffsetY);
            }

            m_pLogoImage->SetPos(SCALE(logoX), SCALE(logoY));
        }
    }
}

void MainMenu::Paint()
{
	BaseClass::Paint();

	DrawMainMenu();
	DrawLogo();
}

void MainMenu::OnCommand(char const* cmd)
{
    GameUI2().SendMainMenuCommand(cmd);

	BaseClass::OnCommand(cmd);
}

void MainMenu::OnSetFocus()
{
	BaseClass::OnSetFocus();

	m_bFocused = true;
	vgui::surface()->PlaySound(m_pszMenuOpenSound);
}

void MainMenu::OnKillFocus()
{
	BaseClass::OnKillFocus();

	m_bFocused = false;
	vgui::surface()->PlaySound(m_pszMenuCloseSound);
}

bool MainMenu::Equals(char const* inputA, char const* inputB)
{
	std::string str1Cpy(inputA);
	std::string str2Cpy(inputB);

	std::transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), std::function<int32(int32)>(::tolower));
	std::transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), std::function<int32(int32)>(::tolower));

	return (str1Cpy == str2Cpy);
}

MainMenuHelper::MainMenuHelper(MainMenu* menu, vgui::Panel* parent) : BaseClass(parent)
{
	menu->SetParent(this);
	menu->MakeReadyForUse();
	menu->SetZPos(0);
}