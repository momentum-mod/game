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

class Button_Panel : public Button2D
{
    DECLARE_CLASS_SIMPLE(Button_Panel, Button2D);

  public:
    Button_Panel(vgui::Panel *parent, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL);

    virtual void Init();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void OnThink();
    virtual void DrawButton();
    virtual void DrawButton_Blur();
    virtual void DrawText();
    virtual void DrawDescription();
    virtual void Paint();
    virtual void PaintBlurMask();
    virtual void Animations();

    virtual void AdditionalCursorCheck();
    virtual void OnCursorEntered();
    virtual void OnCursorExited();
    virtual void OnMouseReleased(vgui::MouseCode code);
    virtual void OnMousePressed(vgui::MouseCode code);

    virtual void SetButtonText(const char *text);
    virtual void SetButtonDescription(const char *description);

    int32 GetWidth() { return m_fWidth; }
    int32 GetHeight() { return m_fHeight; }

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
};