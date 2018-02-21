#pragma once

#include "vgui/ISurface.h"
#include "vgui2d/panel2d.h"

class MainMenu;

enum EBackgroundState
{
    BACKGROUND_INITIAL,
    BACKGROUND_LOADING,
    BACKGROUND_MAINMENU,
    BACKGROUND_LEVEL,
    BACKGROUND_DISCONNECTED
};

class CBasePanel : public Panel2D
{
    DECLARE_CLASS_SIMPLE(CBasePanel, Panel2D);

    CBasePanel();
    virtual ~CBasePanel();

    // notifications
    void OnLevelLoadingStarted();
    void OnLevelLoadingFinished();

    bool IsInLoading() const;

    // update the taskbar a frame
    void RunFrame();

    // fades to black then runs an engine command (usually to start a level)
    void FadeToBlackAndRunEngineCommand(const char *engineCommand);

    // handles gameUI being shown/hidden
    void OnGameUIActivated();
    void OnGameUIHidden();

    // game dialogs
    void OnOpenOptionsDialog();
    void OnOpenAchievementsDialog();

    void PositionDialog(vgui::PHandle dlg);

    // forces any changed options dialog settings to be applied immediately, if it's open
    void ApplyOptionsDialogSettings();

    MainMenu *GetMainMenu() const { return m_pMainMenu; }

    // fading to game
    MESSAGE_FUNC_CHARPTR(RunEngineCommand, "RunEngineCommand", command);
    MESSAGE_FUNC_CHARPTR(RunMenuCommand, "RunMenuCommand", command);
    
    EBackgroundState GetMenuBackgroundState() { return m_eBackgroundState; }

protected:
    void OnThink() OVERRIDE;
    void PaintBlurMask() OVERRIDE;
    void PaintBackground() OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnCommand(const char *command) OVERRIDE{ RunMenuCommand(command); }

private:

    void SetBackgroundRenderState(EBackgroundState state);

    vgui::DHANDLE<vgui::PropertyDialog> m_hOptionsDialog;
    vgui::DHANDLE<vgui::Frame> m_hAchievementsDialog;
    MainMenu *m_pMainMenu;

    // sets the menu alpha [0..255]
    void SetMenuAlpha(int alpha);

    void UpdateBackgroundState();
    EBackgroundState m_eBackgroundState;

    void DrawBackgroundImage();
    int m_iBackgroundImageID;
    int m_iLoadingImageID;
    bool m_bLevelLoading;
    bool m_bEverActivated;

    // background transition
    bool m_bFadingInMenus;
    float m_flFadeMenuStartTime;
    float m_flFadeMenuEndTime;

    bool m_bRenderingBackgroundTransition;
    float m_flTransitionStartTime;
    float m_flTransitionEndTime;

    // background fill transition
    bool m_bHaveDarkenedBackground;
    float m_flFrameFadeInTime;
    Color m_BackdropColor;
    CPanelAnimationVar(float, m_flBackgroundFillAlpha, "m_flBackgroundFillAlpha", "0");
};

extern CBasePanel *GetBasePanel();