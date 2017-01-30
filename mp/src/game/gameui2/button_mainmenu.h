#pragma once

#include "vgui2d/button2d.h"
#include "vgui_controls/AnimationController.h"

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

enum TextAlignment
{
    LEFT,
    CENTER,
    RIGHT
};

class Button_MainMenu : public Button2D
{
    DECLARE_CLASS_SIMPLE(Button_MainMenu, Button2D);

  public:
    Button_MainMenu(Panel *parent, Panel *pActionSignalTarget = nullptr, const char *pCmd = nullptr);

    virtual void Init();
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnThink() OVERRIDE;
    virtual void DrawButton();
    virtual void DrawButton_Blur();
    virtual void DrawText();
    virtual void DrawDescription();
    void Paint() OVERRIDE;
    void PaintBlurMask() OVERRIDE;
    virtual void Animations();

    virtual void AdditionalCursorCheck();
    void OnCursorEntered() OVERRIDE;
    void OnCursorExited() OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;

    virtual void SetButtonText(const char *text);
    virtual void SetButtonDescription(const char *description);

    int32 GetWidth() const { return m_fWidth; }
    int32 GetHeight() const { return m_fHeight; }

    virtual void SetPriority(int32 Priority) { m_iPriority = Priority; }
    int32 GetPriority() const { return m_iPriority; }

    virtual void SetBlank(bool blank) { m_bIsBlank = blank; }
    bool IsBlank() const { return m_bIsBlank; }

    virtual void SetTextAlignment(TextAlignment alignment) { m_iTextAlignment = alignment; }
    virtual void SetButtonType(ButtonType type) { m_nType = type; }
    virtual ButtonType GetButtonType() const { return m_nType; }

  private:
    ButtonState m_sButtonState;
    ButtonState m_sButtonStateOld;
    char m_pCmd[256];
    wchar_t *m_ButtonText;
    wchar_t *m_ButtonDescription;
    int32 m_iPriority;
    int32 m_iTextPositionX;
    int32 m_iTextPositionY;
    int32 m_iTextSizeX;
    int32 m_iTextSizeY;

    CPanelAnimationVar(float, m_fWidth, "m_fWidth", "340");
    CPanelAnimationVar(float, m_fHeight, "m_fHeight", "0");
    CPanelAnimationVar(Color, m_cBackground, "m_cBackground", "0 0 0 0");
    CPanelAnimationVar(Color, m_cBackgroundOutline, "m_cBackgroundOutline", "0 0 0 0");
    CPanelAnimationVar(Color, m_cText, "m_cText", "0 0 0 0");
    CPanelAnimationVar(Color, m_cDescription, "m_cDescription", "0 0 0 0");
    CPanelAnimationVar(Color, m_cBackgroundBlurAlpha, "m_cBackgroundBlurAlpha", "0 0 0 0");

    float m_fWidthOut;
    float m_fWidthOver;
    float m_fWidthPressed;
    float m_fWidthReleased;

    float m_fHeightOut;
    float m_fHeightOver;
    float m_fHeightPressed;
    float m_fHeightReleased;

    float m_fTextOffsetX;
    float m_fTextOffsetY;

    float m_fDescriptionOffsetX;
    float m_fDescriptionOffsetY;

    bool m_bDescriptionHideOut;
    bool m_bDescriptionHideOver;
    bool m_bDescriptionHidePressed;
    bool m_bDescriptionHideReleased;

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

    Color m_cDescriptionOut;
    Color m_cDescriptionOver;
    Color m_cDescriptionPressed;
    Color m_cDescriptionReleased;

    bool m_bBackgroundBlurOut;
    bool m_bBackgroundBlurOver;
    bool m_bBackgroundBlurPressed;
    bool m_bBackgroundBlurReleased;

    vgui::HFont m_fTextFont;
    vgui::HFont m_fDescriptionFont;

    bool m_bIsBlank;
    TextAlignment m_iTextAlignment;
    ButtonType m_nType;
};
