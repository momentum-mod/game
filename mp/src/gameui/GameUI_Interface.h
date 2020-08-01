#pragma once

#include "GameUI/IGameUI.h"

class IGameClientExports;
class CCommand;

class CGameUI : public IGameUI
{
  public:
    CGameUI();
    virtual ~CGameUI();

    void Initialize(CreateInterfaceFn appFactory) OVERRIDE;
    void Connect(CreateInterfaceFn gameFactory) OVERRIDE;
    void Start() OVERRIDE;
    void Shutdown() OVERRIDE;
    void RunFrame() OVERRIDE;
    void PostInit() OVERRIDE;

    // plays the startup mp3 when GameUI starts
    void PlayGameStartupSound();

    // Engine wrappers for activating / hiding the gameUI
    void ActivateGameUI();
    void HideGameUI();

    // Toggle allowing the engine to hide the game UI with the escape key
    void PreventEngineHideGameUI();
    void AllowEngineHideGameUI();

    // notifications
    void OnGameUIActivated() OVERRIDE;
    void OnGameUIHidden() OVERRIDE;

    bool IsInLoading() const { return m_bIsInLoading; }
    void OnLevelLoadingStarted(bool bShowProgressDialog) OVERRIDE;
    void OnLevelLoadingFinished(bool bError, const char *failureReason, const char *extendedReason) OVERRIDE;
    bool UpdateProgressBar(float progress, const char *statusText) OVERRIDE;

    void SendMainMenuCommand(const char *pszCommand) OVERRIDE;
    void OnConfirmQuit() OVERRIDE;
    bool IsMainMenuVisible() OVERRIDE;

    // ====== UNUSED ========
    // Xbox 360: Can't remove these without engine license
    void OnCreditsFinished() OVERRIDE {}
    void SystemNotification() OVERRIDE {}
    void SessionNotification() OVERRIDE {}
    void ShowMessageDialog() OVERRIDE {}
    bool ValidateStorageDevice() OVERRIDE { return true; }
    void SessionSearchResult() OVERRIDE {}
    // Bonus Maps
    void BonusMapUnlock(const char *pchFileName, const char *pchMapName) OVERRIDE {}
    void BonusMapComplete(const char *pchFileName, const char *pchMapName) OVERRIDE {}
    void BonusMapChallengeUpdate(const char *pchFileName, const char *pchMapName, const char *pchChallengeName,int iBest) OVERRIDE{}
    void BonusMapChallengeNames(char *pchFileName, char *pchMapName, char *pchChallengeName) OVERRIDE {}
    void BonusMapChallengeObjectives(int &iBronze, int &iSilver, int &iGold) OVERRIDE {}
    void BonusMapDatabaseSave() OVERRIDE {}
    void BonusMapNumMedals(int piNumMedals[3]) OVERRIDE {}
    int BonusMapNumAdvancedCompleted() OVERRIDE { return 0; }
    // Dialogs
    void ShowNewGameDialog(int chapter) OVERRIDE{}
    void SetMainMenuOverride(vgui::VPANEL panel) OVERRIDE {}
    void SetLoadingBackgroundDialog(vgui::VPANEL panel) OVERRIDE {}
	void UpdatePlayerInfo() OVERRIDE {}
    // Progress
    void SetProgressOnStart() OVERRIDE {}
    bool SetShowProgressText(bool show) OVERRIDE { return false; }
    // Connection
    void OLD_OnConnectToServer(const char *game, int IP, int port) OVERRIDE {} // OLD: use OnConnectToServer2
    void OnConnectToServer2(const char *game, int IP, int connectionPort, int queryPort) OVERRIDE {}
    void OnDisconnectFromServer(uint8 eSteamLoginFailure) OVERRIDE{}
    void OnDisconnectFromServer_OLD(uint8 eSteamLoginFailure, const char *username) OVERRIDE{}

  private:
    int m_iPlayGameStartupSound;
    bool m_bIsInLoading;

    vgui::VPANEL m_hBasePanel;
};

// Purpose: singleton accessor
extern CGameUI &GameUI();