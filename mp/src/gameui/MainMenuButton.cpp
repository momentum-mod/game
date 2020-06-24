#include "MainMenuButton.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

MainMenuButton::MainMenuButton(Panel *parent) : BaseClass(parent, "", "", parent, "")
{
    SetProportional(true);
    m_bIsBlank = false;
    m_iPriority = 0;
    m_nType = SHARED;

    SetPaintBorderEnabled(false);
    SetEnabled(true);
    SetVisible(false);
    SetAutoDelete(true);
}

void MainMenuButton::SetCommand(const char* pCmd)
{
    if (pCmd)
        BaseClass::SetCommand(new KeyValues("MenuButtonCommand", "command", pCmd));
}

void MainMenuButton::SetEngineCommand(const char* pCmd)
{
    if (pCmd)
        BaseClass::SetCommand(new KeyValues("MenuButtonCommand", "EngineCommand", pCmd));
}

void MainMenuButton::ApplySchemeSettings(IScheme *pScheme)
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
    
    m_iWidth = m_iWidthOut = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Out")));
    m_iWidthOver = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Over")));
    m_iWidthPressed = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Pressed")));
    m_iWidthReleased = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Width.Released")));

    m_iHeight = m_iHeightOut = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Out")));
    m_iHeightOver = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Over")));
    m_iHeightPressed = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Pressed")));
    m_iHeightReleased = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Height.Released")));

    SetSize(m_iWidth, m_iHeight);

    m_iTextOffsetX = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Text.OffsetX")));
    m_iTextOffsetY = GetScaledVal(Q_atoi(pScheme->GetResourceString("MainMenu.Button.Text.OffsetY")));
    SetTextInset(m_iTextOffsetX, m_iTextOffsetY);

    m_fAnimationWidth = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Width"));
    m_fAnimationHeight = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Height"));
    m_fAnimationBackground = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Background"));
    m_fAnimationText = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Text"));
    m_fAnimationDescription = Q_atof(pScheme->GetResourceString("MainMenu.Button.Animation.Description"));

    m_cBackground = m_cBackgroundOut = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Out"), pScheme);
    m_cBackgroundOver = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Over"), pScheme);
    m_cBackgroundPressed = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Pressed"), pScheme);
    m_cBackgroundReleased = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Released"), pScheme);

    m_cBackgroundOutline = m_cBackgroundOutlineOut = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Outline.Out"), pScheme);
    m_cBackgroundOutlineOver = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Outline.Over"), pScheme);
    m_cBackgroundOutlinePressed = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Outline.Pressed"), pScheme);
    m_cBackgroundOutlineReleased = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Background.Outline.Released"), pScheme);

    m_cText = m_cTextOut = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Text.Out"), pScheme);
    m_cTextOver = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Text.Over"), pScheme);
    m_cTextPressed = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Text.Pressed"), pScheme);
    m_cTextReleased = GetSchemeColor(pScheme->GetResourceString("MainMenu.Button.Text.Released"), pScheme);

    m_cBackgroundBlurAlpha = Color(0, 0, 0, 0);

    SetFont(pScheme->GetFont("MainMenu.Button.Text.Font", true));

    m_sButtonState = m_sButtonStateOld = Out;
}

void MainMenuButton::Animations()
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
            break;
        }

        m_sButtonStateOld = m_sButtonState;
    }

    SetSize(m_iWidth, m_iHeight);
}

void MainMenuButton::OnThink()
{
    BaseClass::OnThink();

    if (m_bIsBlank)
        return;

    Animations();
    AdditionalCursorCheck();

    SetFgColor(m_cText);
    SetBgColor(m_cBackground);
}

void MainMenuButton::DrawButton_Blur()
{
    surface()->DrawSetColor(m_cBackgroundBlurAlpha);
    surface()->DrawFilledRect(0, 0, m_iWidth + 0, m_iHeight + 0);
}

void MainMenuButton::Paint()
{
    if (m_bIsBlank)
        return;

    BaseClass::Paint();
}

void MainMenuButton::OnCursorExited()
{
    if (m_bIsBlank)
        return;

    BaseClass::OnCursorExited();

    m_sButtonState = Out;
}

void MainMenuButton::OnCursorEntered()
{
    if (m_bIsBlank)
        return;

    BaseClass::OnCursorEntered();

    m_sButtonState = Over;
}

void MainMenuButton::AdditionalCursorCheck()
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

void MainMenuButton::OnMousePressed(MouseCode code)
{
    if (m_bIsBlank)
        return;

    if (code == MOUSE_LEFT)
    {
        m_sButtonState = Pressed;
    }

    BaseClass::OnMousePressed(code);
}

void MainMenuButton::OnMouseReleased(MouseCode code)
{
    if (m_bIsBlank)
        return;

    if (code == MOUSE_LEFT)
    {
        m_sButtonState = Released;
    }

    BaseClass::OnMouseReleased(code);
}
