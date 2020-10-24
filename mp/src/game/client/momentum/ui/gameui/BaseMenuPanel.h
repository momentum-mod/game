#pragma once

#include "vgui_controls/EditablePanel.h"

class MainMenu;
class CLoadingScreen;

enum EBackgroundState
{
    BACKGROUND_INITIAL,
    BACKGROUND_LOADING,
    BACKGROUND_MAINMENU,
    BACKGROUND_LEVEL,
    BACKGROUND_DISCONNECTED
};

class CBaseMenuPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CBaseMenuPanel, vgui::EditablePanel);

    CBaseMenuPanel();
    ~CBaseMenuPanel();

    static void Init();

    bool IsInLoading() const;

    Panel* GetMainMenu();

    void OnShutdownFromQuit();

    // fading to game
    MESSAGE_FUNC_CHARPTR(RunEngineCommand, "RunEngineCommand", command);
    MESSAGE_FUNC_CHARPTR(RunMenuCommand, "RunMenuCommand", command);

    MESSAGE_FUNC(OnGameUIActivated, "GameUIActivated");
    MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden");

    MESSAGE_FUNC_FLOAT(OnLoadProgress, "ProgressFraction", percent);

    MESSAGE_FUNC_PARAMS(OnLevelLoadingStarted, "LevelLoadStarted", pKvData);
    MESSAGE_FUNC_PARAMS(OnLevelLoadingFinished, "LevelLoadFinished", pKvData);
    
    EBackgroundState GetMenuBackgroundState() const { return m_eBackgroundState; }

protected:
    void OnThink() OVERRIDE;
    void PaintBackground() OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnCommand(const char *command) OVERRIDE{ RunMenuCommand(command); }
    bool RequestInfo(KeyValues* data) override;

private:
    void SetBackgroundRenderState(EBackgroundState state);

    MainMenu *m_pMainMenu;
    CLoadingScreen *m_pLoadingScreen;

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

extern CBaseMenuPanel *g_pBasePanel;