#include "button_mainmenu.h"
#include "gameui2_interface.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(Button_MainMenu, Button_MainMenu);

extern CUtlSymbolTable g_ButtonSoundNames;

using namespace vgui;

Button_MainMenu::Button_MainMenu(Panel *parent, Panel *pActionSignalTarget, const char *pCmd)
    : BaseClass(parent, "", "", pActionSignalTarget, pCmd)
{
    Init();
    Q_strncpy(m_pCmd, pCmd, sizeof(m_pCmd));
}

void Button_MainMenu::SetButtonText(const char *text) { m_ButtonText = GameUI2().GetLocalizedString(text); }

void Button_MainMenu::SetButtonDescription(const char *description)
{
    m_ButtonDescription = GameUI2().GetLocalizedString(description);
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
        scheme()->LoadSchemeFromFile("resource2/schememainmenu.res", "SchemeMainMenu");

    SetScheme(menuScheme);

    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetEnabled(true);
    SetVisible(false);
    SetAutoDelete(true);
}

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

    m_fWidth = m_fWidthOut = atof(pScheme->GetResourceString("MainMenu.Button.Width.Out"));
    m_fWidthOver = atof(pScheme->GetResourceString("MainMenu.Button.Width.Over"));
    m_fWidthPressed = atof(pScheme->GetResourceString("MainMenu.Button.Width.Pressed"));
    m_fWidthReleased = atof(pScheme->GetResourceString("MainMenu.Button.Width.Released"));

    m_fHeight = m_fHeightOut = atof(pScheme->GetResourceString("MainMenu.Button.Height.Out"));
    m_fHeightOver = atof(pScheme->GetResourceString("MainMenu.Button.Height.Over"));
    m_fHeightPressed = atof(pScheme->GetResourceString("MainMenu.Button.Height.Pressed"));
    m_fHeightReleased = atof(pScheme->GetResourceString("MainMenu.Button.Height.Released"));

    m_fTextOffsetX = atof(pScheme->GetResourceString("MainMenu.Button.Text.OffsetX"));
    m_fTextOffsetY = atof(pScheme->GetResourceString("MainMenu.Button.Text.OffsetY"));

    m_fDescriptionOffsetX = atof(pScheme->GetResourceString("MainMenu.Button.Description.OffsetX"));
    m_fDescriptionOffsetY = atof(pScheme->GetResourceString("MainMenu.Button.Description.OffsetY"));

    m_bDescriptionHideOut = atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Out"));
    m_bDescriptionHideOver = atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Over"));
    m_bDescriptionHidePressed = atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Pressed"));
    m_bDescriptionHideReleased = atoi(pScheme->GetResourceString("MainMenu.Button.Description.Hide.Released"));

    m_fAnimationWidth = atof(pScheme->GetResourceString("MainMenu.Button.Animation.Width"));
    m_fAnimationHeight = atof(pScheme->GetResourceString("MainMenu.Button.Animation.Height"));
    m_fAnimationBackground = atof(pScheme->GetResourceString("MainMenu.Button.Animation.Background"));
    m_fAnimationText = atof(pScheme->GetResourceString("MainMenu.Button.Animation.Text"));
    m_fAnimationDescription = atof(pScheme->GetResourceString("MainMenu.Button.Animation.Description"));

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
    m_bBackgroundBlurOut = atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Out"));
    m_bBackgroundBlurOver = atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Over"));
    m_bBackgroundBlurPressed = atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Pressed"));
    m_bBackgroundBlurReleased = atoi(pScheme->GetResourceString("MainMenu.Button.Background.Blur.Released"));

    m_fTextFont = pScheme->GetFont("MainMenu.Button.Text.Font", IsProportional());
    m_fDescriptionFont = pScheme->GetFont("MainMenu.Button.Description.Font", false);

    m_sButtonState = m_sButtonStateOld = Out;
}

void Button_MainMenu::Animations()
{

    if (m_sButtonStateOld != m_sButtonState)
    {
        switch (m_sButtonState)
        {
        case Out:
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_fWidthOut, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_fHeightOut, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOut, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOut, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOut ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Over:
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_fWidthOver, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_fHeightOver, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOver, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOver, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOver, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOver, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOver ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Pressed:
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_fWidthPressed, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_fHeightPressed, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundPressed, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlinePressed, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextPressed, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionPressed, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha",
                m_bBackgroundBlurPressed ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0), 0.0f, m_fAnimationBackground,
                AnimationController::INTERPOLATOR_LINEAR);
            break;

        case Released:
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_fWidthReleased, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_fHeightReleased, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundReleased, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineReleased, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextReleased, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionReleased, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha",
                m_bBackgroundBlurReleased ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0), 0.0f, m_fAnimationBackground,
                AnimationController::INTERPOLATOR_LINEAR);
            break;

        default:
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fWidth", m_fWidthOut, 0.0f, m_fAnimationWidth,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_fHeight", m_fHeightOut, 0.0f, m_fAnimationHeight,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackground", m_cBackgroundOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cBackgroundOutline", m_cBackgroundOutlineOut, 0.0f,
                                                   m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cText", m_cTextOut, 0.0f, m_fAnimationText,
                                                   AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(this, "m_cDescription", m_cDescriptionOut, 0.0f,
                                                   m_fAnimationDescription, AnimationController::INTERPOLATOR_LINEAR);
            GameUI2().GetAnimationController()->RunAnimationCommand(
                this, "m_cBackgroundBlurAlpha", m_bBackgroundBlurOut ? Color(255, 255, 255, 255) : Color(0, 0, 0, 0),
                0.0f, m_fAnimationBackground, AnimationController::INTERPOLATOR_LINEAR);
            break;
        }

        m_sButtonStateOld = m_sButtonState;
    }

    SetSize(m_fWidth, m_fHeight);
}

void Button_MainMenu::OnThink()
{
    BaseClass::OnThink();

    Animations();
    AdditionalCursorCheck();
}

void Button_MainMenu::DrawButton()
{
    surface()->DrawSetColor(m_cBackground);
    surface()->DrawFilledRect(0, 0, static_cast<int>(m_fWidth), static_cast<int>(m_fHeight));

    surface()->DrawSetColor(m_cBackgroundOutline);
    surface()->DrawOutlinedRect(0, 0, static_cast<int>(m_fWidth), static_cast<int>(m_fHeight));
}

void Button_MainMenu::DrawButton_Blur()
{
    surface()->DrawSetColor(m_cBackgroundBlurAlpha);
    surface()->DrawFilledRect(0, 0, m_fWidth + 0, m_fHeight + 0);
}

inline int CalculateDescOffsetFromAlignment(TextAlignment align, int mainTextSize, int mainTextPos, int descWide, float m_fDescOffsetX)
{
    int toReturn;
    int descOffset = static_cast<int>(m_fDescOffsetX);
    switch (align)
    {
    default:
    case LEFT:
        toReturn = mainTextPos + descOffset;
        break;
    case CENTER:
        toReturn = descOffset;//Doesn't matter for center
        break;
    case RIGHT:
        toReturn = descWide <= mainTextSize ? (mainTextSize - descWide + descOffset) : descOffset;
        break;
    }

    return toReturn;
}

inline int CalculateTextXFromAlignment(TextAlignment align, float buttonWide, int textWide, float m_fTextOffsetX)
{
    int toReturn;
    int iTextOffset = static_cast<int>(m_fTextOffsetX);
    int iButtonWide = static_cast<int>(buttonWide);
    switch (align)
    {
    default:
    case LEFT: // LEFT ALIGN
        toReturn = iTextOffset;
        break;
    case CENTER: // CENTER
        toReturn = iButtonWide / 2 - textWide / 2;
        break;
    case RIGHT: // RIGHT ALIGN
        toReturn = iButtonWide - iTextOffset - textWide;
        break;
    }
    return toReturn;
}

void Button_MainMenu::DrawText()
{
    surface()->DrawSetTextColor(m_cText);
    surface()->DrawSetTextFont(m_fTextFont);

    surface()->GetTextSize(m_fTextFont, m_ButtonText, m_iTextSizeX, m_iTextSizeY);
    m_iTextPositionX = CalculateTextXFromAlignment(m_iTextAlignment, m_fWidth, m_iTextSizeX, m_fTextOffsetX);
    m_iTextPositionY = m_fHeight / 2 - m_iTextSizeY / 2 + m_fTextOffsetY;

    surface()->DrawSetTextPos(m_iTextPositionX, m_iTextPositionY);
    surface()->DrawPrintText(m_ButtonText, wcslen(m_ButtonText));
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
    int offsetX = CalculateDescOffsetFromAlignment(m_iTextAlignment, m_iTextSizeX, m_iTextPositionX, descWide, m_fDescriptionOffsetX);
    int descriptionX = CalculateTextXFromAlignment(m_iTextAlignment, m_fWidth, descWide, offsetX);

    surface()->DrawSetTextPos(descriptionX, m_iTextPositionY + m_iTextSizeY + m_fDescriptionOffsetY);
    surface()->DrawPrintText(m_ButtonDescription, wcslen(m_ButtonDescription));
}

void Button_MainMenu::Paint()
{
    BaseClass::Paint();

    DrawButton();
    DrawText();
    DrawDescription();
}

void Button_MainMenu::PaintBlurMask()
{
    BaseClass::PaintBlurMask();

    if (GameUI2().IsInBackgroundLevel())
        DrawButton_Blur();
}

void Button_MainMenu::OnCursorExited()
{
    BaseClass::OnCursorExited();

    if (IsBlank())
        return;

    m_sButtonState = Out;
}

void Button_MainMenu::OnCursorEntered()
{
    BaseClass::OnCursorEntered();

    if (IsBlank())
        return;

    m_sButtonState = Over;
}

void Button_MainMenu::AdditionalCursorCheck()
{
    if (!input())
        return;

    if (IsBlank())
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
    if (IsBlank())
        return;

    if (code == MOUSE_LEFT)
    {
        if (m_sDepressedSoundName != UTL_INVAL_SYMBOL)
            surface()->PlaySound(g_ButtonSoundNames.String(m_sDepressedSoundName));

        m_sButtonState = Pressed;
    }
}

void Button_MainMenu::OnMouseReleased(MouseCode code)
{
    if (IsBlank())
        return;

    if (code == MOUSE_LEFT)
    {
        if (m_sReleasedSoundName != UTL_INVAL_SYMBOL)
            surface()->PlaySound(g_ButtonSoundNames.String(m_sReleasedSoundName));

        m_sButtonState = Released;

        GetParent()->OnCommand(m_pCmd);
    }
}
