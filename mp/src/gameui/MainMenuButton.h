#pragma once

#include "vgui_controls/Button.h"

enum ButtonState
{
    Out,
    Over,
    Pressed,
    Released
};

enum ButtonType
{
    SHARED,
    MAIN_MENU,
    IN_GAME
};

class MainMenuButton : public vgui::Button
{
    DECLARE_CLASS_SIMPLE(MainMenuButton, vgui::Button);

  public:
    MainMenuButton(Panel *parent);

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnThink() OVERRIDE;
    virtual void DrawButton_Blur();
    void Paint() OVERRIDE;
    virtual void Animations();

    virtual void AdditionalCursorCheck();
    void OnCursorEntered() OVERRIDE;
    void OnCursorExited() OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;

    int32 GetWidth() const { return m_iWidth; }
    int32 GetHeight() const { return m_iHeight; }

    virtual void SetPriority(int32 Priority) { m_iPriority = Priority; }
    int32 GetPriority() const { return m_iPriority; }

    virtual void SetBlank(bool blank) { m_bIsBlank = blank; }
    bool IsBlank() const { return m_bIsBlank; }

    virtual void SetButtonType(ButtonType type) { m_nType = type; }
    virtual ButtonType GetButtonType() const { return m_nType; }

    void SetCommand(const char* cmd) OVERRIDE;
    void SetEngineCommand(const char *cmd);

  private:

    ButtonState m_sButtonState;
    ButtonState m_sButtonStateOld;
    int32 m_iPriority;
    int32 m_iTextPositionX;
    int32 m_iTextPositionY;
    int32 m_iTextSizeX;
    int32 m_iTextSizeY;

    CPanelAnimationVar(int, m_iWidth, "m_iWidth", "340");
    CPanelAnimationVar(int, m_iHeight, "m_iHeight", "0");
    CPanelAnimationVar(Color, m_cBackground, "m_cBackground", "0 0 0 0");
    CPanelAnimationVar(Color, m_cBackgroundOutline, "m_cBackgroundOutline", "0 0 0 0");
    CPanelAnimationVar(Color, m_cText, "m_cText", "0 0 0 0");
    CPanelAnimationVar(Color, m_cBackgroundBlurAlpha, "m_cBackgroundBlurAlpha", "0 0 0 0");

    int m_iWidthOut;
    int m_iWidthOver;
    int m_iWidthPressed;
    int m_iWidthReleased;

    int m_iHeightOut;
    int m_iHeightOver;
    int m_iHeightPressed;
    int m_iHeightReleased;

    int m_iTextOffsetX;
    int m_iTextOffsetY;

    // Animation durations (in seconds)
    float m_fAnimationWidth;
    float m_fAnimationHeight;
    float m_fAnimationBackground;
    float m_fAnimationText;
    float m_fAnimationDescription;

    Color m_cBackgroundOut;
    Color m_cBackgroundOver;
    Color m_cBackgroundPressed;
    Color m_cBackgroundReleased;

    Color m_cBackgroundOutlineOut;
    Color m_cBackgroundOutlineOver;
    Color m_cBackgroundOutlinePressed;
    Color m_cBackgroundOutlineReleased;

    Color m_cTextOut;
    Color m_cTextOver;
    Color m_cTextPressed;
    Color m_cTextReleased;

    bool m_bIsBlank;
    ButtonType m_nType;
};
