#include "basepanel.h"
#include "gameui2_interface.h"
#include "mainmenu.h"

#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui_controls/ImagePanel.h"

#include "KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

MainMenu::MainMenu(Panel *parent) : BaseClass(parent, "MainMenu")
{
    HScheme Scheme = scheme()->LoadSchemeFromFile("resource2/schememainmenu.res", "SchemeMainMenu");
    SetScheme(Scheme);

    SetProportional(false);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetAutoDelete(true);

    m_bFocused = true;
    m_bNeedSort = false;
    m_nSortFlags = FL_SORT_SHARED;

    m_logoLeft = GameUI2().GetLocalizedString("#GameUI2_LogoLeft");
    m_logoRight = GameUI2().GetLocalizedString("#GameUI2_LogoRight");
    m_pLogoImage = nullptr;
    m_pButtonFeedback = new Button_MainMenu(this, this, "engine mom_contact_show");
    m_pButtonFeedback->SetButtonText("#GameUI2_SendFeedback");
    m_pButtonFeedback->SetButtonDescription("#GameUI2_SendFeedbackDescription");
    m_pButtonFeedback->SetPriority(1);
    m_pButtonFeedback->SetBlank(false);
    m_pButtonFeedback->SetVisible(true);
    m_pButtonFeedback->SetTextAlignment(RIGHT);
    CreateMenu("resource2/mainmenu.res");

    MakeReadyForUse();
    SetZPos(0);
    RequestFocus();
}

void MainMenu::CreateMenu(const char *menu)
{
    KeyValues *datafile = new KeyValues("MainMenu");
    datafile->UsesEscapeSequences(true);
    if (datafile->LoadFromFile(g_pFullFileSystem, menu))
    {
        FOR_EACH_SUBKEY(datafile, dat)
        {
            Button_MainMenu *button = new Button_MainMenu(this, this, dat->GetString("command", ""));
            button->SetPriority(dat->GetInt("priority", 1));
            button->SetButtonText(dat->GetString("text", "no text"));
            button->SetButtonDescription(dat->GetString("description", "no description"));
            button->SetBlank(dat->GetBool("blank"));

            const char *specifics = dat->GetString("specifics", "shared");
            if (!Q_strcasecmp(specifics, "ingame"))
                button->SetButtonType(IN_GAME);
            else if (!Q_strcasecmp(specifics, "mainmenu"))
                button->SetButtonType(MAIN_MENU);

            m_pButtons.AddToTail(button);
        }
    }

    datafile->deleteThis();
}

int32 __cdecl ButtonsPositionBottom(Button_MainMenu *const *s1, Button_MainMenu *const *s2)
{
    return ((*s1)->GetPriority() > (*s2)->GetPriority());
}

int32 __cdecl ButtonsPositionTop(Button_MainMenu *const *s1, Button_MainMenu *const *s2)
{
    return ((*s1)->GetPriority() < (*s2)->GetPriority());
}

void MainMenu::ApplySchemeSettings(IScheme *pScheme)
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
            m_pLogoImage = new ImagePanel(this, "GameLogo");

        m_pLogoImage->SetAutoDelete(true);
        m_pLogoImage->SetShouldScaleImage(true);
        m_pLogoImage->SetImage(pScheme->GetResourceString("MainMenu.Logo.Image"));
        int imageW = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Width")));
        int imageH = SCALE(atoi(pScheme->GetResourceString("MainMenu.Logo.Image.Height")));
        m_pLogoImage->SetSize(imageW, imageH);
        // Pos is handled in Paint()
    }
    m_fLogoFont = pScheme->GetFont("MainMenu.Logo.Font", true);

    Q_strncpy(m_pszMenuOpenSound, pScheme->GetResourceString("MainMenu.Sound.Open"), sizeof(m_pszMenuOpenSound));
    Q_strncpy(m_pszMenuCloseSound, pScheme->GetResourceString("MainMenu.Sound.Close"), sizeof(m_pszMenuCloseSound));
}

void MainMenu::OnThink()
{
    BaseClass::OnThink();

    if (ipanel())
        SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);
}

bool MainMenu::IsVisible(void)
{
    if (GameUI2().IsInLoading())
        return false;

    return BaseClass::IsVisible();
}

inline Button_MainMenu *GetNextVisible(CUtlVector<Button_MainMenu *> *vec, int start)
{
    for (int i = start + 1; i < vec->Count(); i++)
    {
        Button_MainMenu *pButton = vec->Element(i);
        if (pButton->IsVisible())
            return pButton;
    }
    return nullptr;
}

void MainMenu::DrawMainMenu()
{
    for (int8 i = 0; i < m_pButtons.Count(); i++)
    {
        Button_MainMenu *pButton = m_pButtons[i];
        switch (pButton->GetButtonType())
        {
        default:
        case SHARED:
            pButton->SetVisible(GameUI2().IsInLevel() || GameUI2().IsInBackgroundLevel());
            break;
        case IN_GAME:
            pButton->SetVisible(GameUI2().IsInLevel());
            break;
        case MAIN_MENU:
            pButton->SetVisible(GameUI2().IsInBackgroundLevel());
            break;
        }
    }

    if (GameUI2().IsInLevel())
    {
        m_nSortFlags &= ~FL_SORT_MENU;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_INGAME));
        m_nSortFlags |= FL_SORT_INGAME;
    }
    else if (GameUI2().IsInBackgroundLevel())
    {
        m_nSortFlags &= ~FL_SORT_INGAME;

        m_bNeedSort = (!m_bNeedSort && !(m_nSortFlags & FL_SORT_MENU));
        m_nSortFlags |= FL_SORT_MENU;
    }

    if (m_pButtons.Count() > 0 && m_bNeedSort)
    {
        m_bNeedSort = false;
        m_pButtons.Sort(ButtonsPositionTop);
    }

    for (int8 i = 0; i < m_pButtons.Count(); i++)
    {
        Button_MainMenu *pNextVisible = GetNextVisible(&m_pButtons, i);
        if (pNextVisible)
        {
            int32 x0, y0;
            pNextVisible->GetPos(x0, y0);
            m_pButtons[i]->SetPos(m_fButtonsOffsetX, y0 - (m_pButtons[i]->GetHeight() + m_fButtonsSpace));
        }
        else
        {
            m_pButtons[i]->SetPos(m_fButtonsOffsetX,
                                  GameUI2().GetViewport().y - (m_fButtonsOffsetY + m_pButtons[i]->GetHeight()));
        }
    }

    // MOM_TODO: Remove this after it's not needed
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
        surface()->DrawSetTextColor(m_cLogoLeft);
        surface()->DrawSetTextFont(m_fLogoFont);

        int32 logoW, logoH;
        surface()->GetTextSize(m_fLogoFont, m_logoLeft, logoW, logoH);

        int32 logoX, logoY;
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
            logoY = y0 - (logoH + m_fLogoOffsetY);
        }
        surface()->DrawSetTextPos(logoX, logoY);
        surface()->DrawPrintText(m_logoLeft, wcslen(m_logoLeft));

        surface()->DrawSetTextColor(m_cLogoRight);
        surface()->DrawSetTextFont(m_fLogoFont);
        surface()->DrawSetTextPos(logoX + logoW, logoY);
        surface()->DrawPrintText(m_logoRight, wcslen(m_logoRight));
    }
    else
    {
        // This is a logo, but let's make sure it's still valid
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

void MainMenu::OnCommand(char const *cmd)
{
    GameUI2().GetGameUI()->SendMainMenuCommand(cmd);

    BaseClass::OnCommand(cmd);
}

void MainMenu::OnSetFocus()
{
    BaseClass::OnSetFocus();
    m_bFocused = true;
    surface()->PlaySound(m_pszMenuOpenSound);
}

void MainMenu::OnKillFocus()
{
    BaseClass::OnKillFocus();
    m_bFocused = false;
    surface()->PlaySound(m_pszMenuCloseSound);
}