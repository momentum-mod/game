#include "button_mainmenu.h"
#include "GameUI_Interface.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(Button_MainMenu, Button_MainMenu);

using namespace vgui;

Button_MainMenu::Button_MainMenu(Panel *parent, Panel *pActionSignalTarget, const char *pCmd)
    : BaseClass(parent, "", "", pActionSignalTarget, pCmd), m_ButtonText(nullptr), m_ButtonDescription(nullptr)
{
    Init();
}

void Button_MainMenu::SetButtonText(const char *text)
{
    if (m_ButtonText)
    {
        free(m_ButtonText);
        m_ButtonText = nullptr;
    }

    GameUI().GetLocalizedString(text, &m_ButtonText);
}

void Button_MainMenu::SetButtonDescription(const char *description)
{
    if (m_ButtonDescription)
    {
        free(m_ButtonDescription);
        m_ButtonDescription = nullptr;
    }

    GameUI().GetLocalizedString(description, &m_ButtonDescription);
}

void Button_MainMenu::SetCommand(const char* pCmd)
{
    BaseClass::SetCommand(new KeyValues("MenuButtonCommand", "command", pCmd));
}

void Button_MainMenu::SetEngineCommand(const char* cmd)
{
    BaseClass::SetCommand(new KeyValues("MenuButtonCommand", "EngineCommand", cmd));
}

void Button_MainMenu::Init()
{
    SetProportional(true);
    m_bIsBlank = false;
    m_iPriority = 0;
    m_iTextAlignment = LEFT;
    m_nType = SHARED;

    HScheme menuScheme = scheme()->GetScheme("SchemeMainMenu");
    // Load this scheme only if it doesn't exist yet
    if (!menuScheme)
        scheme()->LoadSchemeFromFile("resource/schememainmenu.res", "SchemeMainMenu");

    SetScheme(menuScheme);

    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetEnabled(true);
    SetVisible(false);
    SetAutoDelete(true);
}

#define SC(val) scheme()->GetProportionalScaledValueEx(GetScheme(), val)

void Button_MainMenu::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetDefaultColor(Color(0, 0, 0, 0), Color(0, 0, 0, 0));
    SetArmedColor(Color(0, 0, 0, 0), Color(0, 0, 0, 0));
    SetSelectedColor(Color(0, 0, 0, 0), Color(0, 0, 0, 0));
    SetDepressedColor(Color(0, 0, 0, 0), Color(0, 0, 0, 0));
    SetBlinkColor(Color(0, 0, 0, 0));
    SetArmedSound(pScheme->GetResourceString("MainMenu.Button.Sound.Armed"));
    SetDepressedSound(pScheme->GetResourceString("MainMenu.Button.Sound.Depressed"));
    SetReleasedSound(pScheme->GetResourceString("MainMenu.Button.Sound.Released"));
    
    m_iWidth = m_iWidthOut = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Out")));
    m_iWidthOver = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Over")));
    m_iWidthPressed = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Pressed")));
    m_iWidthReleased = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Released")));

    m_iHeight = m_iHeightOut = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Out")));
    m_iHeightOver = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Over")));
    m_iHeightPressed = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Pressed")));
    m_iHeightReleased = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Released")));

    m_iTextOffsetX = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Text.OffsetX")));
    m_iTextOffsetY = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Text.OffsetY")));

    m_iDescriptionOffsetX = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.OffsetX")));
    m_iDescriptionOffsetY = SC(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.OffsetY")));

    m_bDescriptionHideOut = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Out"));
    m_bDescriptionHideOver = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Over"));
    m_bDescriptionHidePressed = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Pressed"));
    m_bDescriptionHideReleased = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Released"));

    m_fAnimationWidth = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Width"));
    m_fAnimationHeight = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Height"));
    m_fAnimationBackground = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Background"));
    m_fAnimationText = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Text"));
    m_fAnimationDescription = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Description"));

    m_cBackground = m_cBackgroundOut = GetSchemeColor("MainMenu.Button.Background.Out", pScheme);
    m_cBackgroundOver = GetSchemeColor("MainMenu.Button.Background.Over", pScheme);
    m_cBackgroundPressed = GetSchemeColor("MainMenu.Button.Background.Pressed", pScheme);
    m_cBackgroundReleased = GetSchemeColor("MainMenu.Button.Background.Released", pScheme);

    m_cBackgroundOutline = m_cBackgroundOutlineOut = GetSchemeColor("MainMenu.Button.Background.Outline.Out", pScheme);
    m_cBackgroundOutlineOver = GetSchemeColor("MainMenu.Button.Background.Outline.Over", pScheme);
    m_cBackgroundOutlinePressed = GetSchemeColor("MainMenu.Button.Background.Outline.Pressed", pScheme);
    m_cBackgroundOutlineReleased = GetSchemeColor("MainMenu.Button.Background.Outline.Released", pScheme);

    m_cText = m_cTextOut = GetSchemeColor("MainMenu.Button.Text.Out", pScheme);
    m_cTextOver = GetSchemeColor("MainMenu.Button.Text.Over", pScheme);
    m_cTextPressed = GetSchemeColor("MainMenu.Button.Text.Pressed", pScheme);
    m_cTextReleased = GetSchemeColor("MainMenu.Button.Text.Released", pScheme);

    m_cDescription = m_cDescriptionOut = GetSchemeColor("MainMenu.Button.Description.Out", pScheme);
    m_cDescriptionOver = GetSchemeColor("MainMenu.Button.Description.Over", pScheme);
    m_cDescriptionPressed = GetSchemeColor("MainMenu.Button.Description.Pressed", pScheme);
    m_cDescriptionReleased = GetSchemeColor("MainMenu.Button.Description.Released", pScheme);

    m_cBackgroundBlurAlpha = Color(0, 0, 0, 0);
    m_bBackgroundBlurOut = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Out"));
    m_bBackgroundBlurOver = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Over"));
    m_bBackgroundBlurPressed = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Pressed"));
    m_bBackgroundBlurReleased = Q_atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Released"));

    m_fTextFont = pScheme->GetFont("MainMenu.Button.Text.Font", true);
    m_fDescriptionFont = pScheme->GetFont("MainMenu.Button.Description.Font", true);

    m_sButtonState = m_sButtonStateOld = Out;
}

void Button_MainMenu::Animations()
{
    if (m_sButtonStateOld != m_sButtonState)
    {
        switch (m_sButtonState)
        {
        case Out:
            GetAnimationController()->RunAnimationCommand(this, "m_iWidth", m_iWidthOut, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_iHeight", m_iHeightOut, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOut, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOut, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOut ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Over:
            GetAnimationController()->RunAnimationCommand(this, "m_iWidth", m_iWidthOver, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_iHeight", m_iHeightOver, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOver, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOver, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOver, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOver, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOver ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Pressed:
            GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_iWidthPressed, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_iHeightPressed, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundPressed, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlinePressed, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextPressed, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionPressed, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha",
                m_bBackgroundBlurPressed ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0), 0.0f, m_fAnimationBackground,
                AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Released:
            GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_iWidthReleased, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_iHeightReleased, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundReleased, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineReleased, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextReleased, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionReleased, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha",
                m_bBackgroundBlurReleased ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0), 0.0f, m_fAnimationBackground,
                AnimationController::INTERPOLATOR_LINEAR);
            break;

        default:
            GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_iWidthOut, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_iHeightOut, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOut, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOut, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOut ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;
        }

        m_sButtonStateOld = m_sButtonState;
    }

    SetSize(m_iWidth, m_iHeight);
}

void Button_MainMenu::OnThink()
{
    BaseClass::OnThink();

    if (m_bIsBlank)
        return;

    Animations();
    AdditionalCursorCheck();
}

void Button_MainMenu::DrawButton()
{
    surface()->DrawSetColor(m_cBackground);
    surface()->DrawFilledRect(0, 0, m_iWidth, m_iHeight);

    surface()->DrawSetColor(m_cBackgroundOutline);
    surface()->DrawOutlinedRect(0, 0, m_iWidth, m_iHeight);
}

void Button_MainMenu::DrawButton_Blur()
{
    surface()->DrawSetColor(m_cBackgroundBlurAlpha);
    surface()->DrawFilledRect(0, 0, m_iWidth + 0, m_iHeight + 0);
}

void Button_MainMenu::CalculateTextX(int textOffset, int textWide, int &out)
{
    switch (m_iTextAlignment)
    {
    default:
    case LEFT: // LEFT ALIGN
        out = textOffset;
        break;
    case CENTER: // CENTER
        out = m_iWidth / 2 - textWide / 2;
        break;
    case RIGHT: // RIGHT ALIGN
        out = m_iWidth - textOffset - textWide;
        break;
    }
}

int Button_MainMenu::CalculateDescOffsetX(int descWide)
{
    int toReturn;
    int descOffset = m_iDescriptionOffsetX;
    switch (m_iTextAlignment)
    {
    default:
    case LEFT:
        toReturn = m_iTextPositionX + descOffset;
        break;
    case CENTER:
        toReturn = descOffset;//Doesn't matter for center
        break;
    case RIGHT:
        toReturn = descWide <= m_iTextSizeX ? 
            (m_iTextSizeX - descWide + descOffset) : // Shift off the difference so we line up with the text's start
            m_iWidth - (m_iTextPositionX + m_iTextSizeX) + descOffset; // Shift our end to match the end of the main text
        break;
    }

    return toReturn;
}

void Button_MainMenu::DrawText()
{
    surface()->DrawSetTextColor(m_cText);
    surface()->DrawSetTextFont(m_fTextFont);

    surface()->GetTextSize(m_fTextFont, m_ButtonText, m_iTextSizeX, m_iTextSizeY);
    CalculateTextX(m_iTextOffsetX, m_iTextSizeX, m_iTextPositionX);
    m_iTextPositionY = m_iHeight / 2 - m_iTextSizeY / 2 + m_iTextOffsetY;

    surface()->DrawSetTextPos(m_iTextPositionX, m_iTextPositionY);
    surface()->DrawPrintText(m_ButtonText, Q_wcslen(m_ButtonText));
}

void Button_MainMenu::DrawDescription()
{
    if ((m_sButtonState == Out && m_bDescriptionHideOut) ||
        (m_sButtonState == Over && m_bDescriptionHideOver) ||
        (m_sButtonState == Pressed && m_bDescriptionHidePressed) ||
        (m_sButtonState == Released && m_bDescriptionHideReleased))
        return;

    surface()->DrawSetTextColor(m_cDescription);
    surface()->DrawSetTextFont(m_fDescriptionFont);
    int descWide, descTall;
    surface()->GetTextSize(m_fDescriptionFont, m_ButtonDescription, descWide, descTall);
    int offsetX = CalculateDescOffsetX(descWide);
    int descriptionX;
    CalculateTextX(offsetX, descWide, descriptionX);

    surface()->DrawSetTextPos(descriptionX, m_iTextPositionY + m_iTextSizeY + m_iDescriptionOffsetY);
    surface()->DrawPrintText(m_ButtonDescription, Q_wcslen(m_ButtonDescription));
}

void Button_MainMenu::Paint()
{
    if (m_bIsBlank)
        return;

    BaseClass::Paint();

    DrawButton();
    DrawText();
    DrawDescription();
}

void Button_MainMenu::PaintBlurMask()
{
    BaseClass::PaintBlurMask();

    if (GameUI().IsInBackgroundLevel())
        DrawButton_Blur();
}

void Button_MainMenu::OnCursorExited()
{
    if (m_bIsBlank)
        return;

    BaseClass::OnCursorExited();

    m_sButtonState = Out;
}

void Button_MainMenu::OnCursorEntered()
{
    if (m_bIsBlank)
        return;

    BaseClass::OnCursorEntered();

    m_sButtonState = Over;
}

void Button_MainMenu::AdditionalCursorCheck()
{
    if (!input())
        return;

    if (m_bIsBlank)
        return;

    // Essentially IsCursorOver, needed because animations mess up IsCursorOver
    if (input()->GetMouseOver() == GetVPanel())
    {
        m_sButtonState = Over;
    }
    else
        m_sButtonState = Out;
}

void Button_MainMenu::OnMousePressed(MouseCode code)
{
    if (m_bIsBlank)
        return;

    if (code == MOUSE_LEFT)
    {
        m_sButtonState = Pressed;
    }

    BaseClass::OnMousePressed(code);
}

void Button_MainMenu::OnMouseReleased(MouseCode code)
{
    if (m_bIsBlank)
        return;

    if (code == MOUSE_LEFT)
    {
        m_sButtonState = Released;
    }

    BaseClass::OnMouseReleased(code);
}
