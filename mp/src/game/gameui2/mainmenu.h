#pragma once

#include "vgui2d/panel2d.h"

#include "button_mainmenu.h"

#include <algorithm>
#include <functional>
#include <string>

class MainMenu : public Panel2D
{
    DECLARE_CLASS_SIMPLE(MainMenu, Panel2D);

  public:
    MainMenu(vgui::Panel *parent);

    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void CreateMenu(const char *menu);
    virtual void OnThink();
    virtual bool IsVisible();
    virtual void DrawMainMenu();
    virtual void DrawLogo();
    virtual void Paint();
    virtual void OnCommand(char const *cmd);
    virtual void OnSetFocus();
    virtual void OnKillFocus();
    virtual bool Equals(char const *inputA, char const *inputB);

  private:
    CUtlVector<Button_MainMenu *> m_pButtons;
    CUtlVector<Button_MainMenu *> m_pButtonsInGame;
    CUtlVector<Button_MainMenu *> m_pButtonsBackground;
    CUtlVector<Button_MainMenu *> m_pButtonsShared;

    // MOM_TODO: Remove this when it's no longer needed
    Button_MainMenu *m_pButtonFeedback;

    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bFocused;
    wchar_t *m_logoLeft;
    wchar_t *m_logoRight;

    float m_fButtonsSpace;

    float m_fButtonsOffsetX;
    float m_fButtonsOffsetY;

    float m_fLogoOffsetX;
    float m_fLogoOffsetY;

    bool m_bLogoText;
    vgui::ImagePanel *m_pLogoImage;
    bool m_bLogoAttachToMenu;

    Color m_cLogoLeft;
    Color m_cLogoRight;

    vgui::HFont m_fLogoFont;
};