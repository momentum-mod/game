//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the interface that the GameUI dll exports
//
// $NoKeywords: $
//=============================================================================//

#ifndef GAMEUI_INTERFACE_H
#define GAMEUI_INTERFACE_H
#pragma once

#include "GameUI/IGameUI.h"

#include "vgui_controls/PHandle.h"
#include "vgui_controls/Panel.h"
#include "steam/steam_api.h"
#include "view_shared.h"
#include "ivrenderview.h"

class IGameClientExports;
class CCommand;

//-----------------------------------------------------------------------------
// Purpose: Implementation of GameUI's exposed interface
//-----------------------------------------------------------------------------
class CGameUI : public IGameUI
{
  public:
    CGameUI();
    ~CGameUI();

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

    void SetLoadingBackgroundDialog(vgui::VPANEL panel) OVERRIDE;;

    // notifications
    void OnGameUIActivated() OVERRIDE;
    void OnGameUIHidden() OVERRIDE;

    void OnLevelLoadingStarted(bool bShowProgressDialog) OVERRIDE;
    void OnLevelLoadingFinished(bool bError, const char *failureReason, const char *extendedReason) OVERRIDE;

    // progress
    bool UpdateProgressBar(float progress, const char *statusText) OVERRIDE;
    // Shows progress desc, returns previous setting... (used with custom progress bars )
    bool SetShowProgressText(bool show) OVERRIDE;

    // Allows the level loading progress to show map-specific info
    virtual void SetProgressLevelName(const char *levelName);

    void SetProgressOnStart() OVERRIDE;

    void SendMainMenuCommand(const char *pszCommand) OVERRIDE;
    void OnConfirmQuit() OVERRIDE;
    bool IsMainMenuVisible() OVERRIDE;

    CSteamAPIContext *GetSteamContext() { return &m_SteamAPIContext; }

    // ====== UNUSED ========
    // Xbox 360
    void OnCreditsFinished() OVERRIDE {}
    void SystemNotification(const int notification) OVERRIDE {}
    void SessionNotification(const int notification, const int param) OVERRIDE {}
    void ShowMessageDialog(const uint nType, vgui::Panel *pOwner) OVERRIDE {}
    bool ValidateStorageDevice(int *pStorageDeviceValidated) OVERRIDE { return true; }
    void SessionSearchResult(int searchIdx, void *pHostData, XSESSION_SEARCHRESULT *pResult, int ping) OVERRIDE {}
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
    void SetMainMenuOverride(vgui::VPANEL panel) OVERRIDE{}
    void UpdatePlayerInfo(uint64 nPlayerId, const char *pName, int nTeam, byte cVoiceState, int nPlayersNeeded, bool bHost) OVERRIDE{}
    // Connection
    void OLD_OnConnectToServer(const char *game, int IP, int port) OVERRIDE {} // OLD: use OnConnectToServer2
    void OnConnectToServer2(const char *game, int IP, int connectionPort, int queryPort) OVERRIDE {}
    void OnDisconnectFromServer(uint8 eSteamLoginFailure) OVERRIDE{}
    void OnDisconnectFromServer_OLD(uint8 eSteamLoginFailure, const char *username) OVERRIDE{}

    // state
    bool IsInLevel();
    bool IsInBackgroundLevel();
    bool IsInMenu();
    bool IsInMultiplayer();
    bool HasSavedThisMenuSession();
    void SetSavedThisMenuSession(bool bState);

    void ShowLoadingBackgroundDialog();
    void HideLoadingBackgroundDialog();
    bool HasLoadingBackgroundDialog();

    virtual CViewSetup GetView() const { return m_pView; }
    virtual VPlane *GetFrustum() const { return m_pFrustum; }
    virtual ITexture *GetMaskTexture() const { return m_pMaskTexture; }
    virtual IVRenderView* GetRenderView() const { return m_pRenderView; }
    virtual IMaterialSystem* GetMaterialSystem() const { return m_pMaterialSystem; }
    virtual Vector2D GetViewport() const;
    void SetView(const CViewSetup &view) { m_pView = view; }
    void SetFrustum(VPlane *frustum) { m_pFrustum = frustum; }
    void SetMaskTexture(ITexture *maskTexture) { m_pMaskTexture = maskTexture; }

    void GetLocalizedString(const char *pToken, wchar_t **pOut);

  private:
    virtual void StartProgressBar();
    virtual bool ContinueProgressBar(float progressFraction);
    virtual void StopProgressBar(bool bError, const char *failureReason, const char *extendedReason = NULL);
    virtual bool SetProgressBarStatusText(const char *statusText);

    //!! these functions currently not implemented
    virtual void SetSecondaryProgressBar(float progress /* range [0..1] */);
    virtual void SetSecondaryProgressBarText(const char *statusText);

    bool m_bActivatedUI : 1;
    bool m_bHasSavedThisMenuSession : 1;
    bool m_bOpenProgressOnStart : 1;
    int m_iPlayGameStartupSound;

    char m_szPreviousStatusText[128];
    char m_szPlatformDir[MAX_PATH];

    CSteamAPIContext m_SteamAPIContext;

    CViewSetup m_pView;
    VPlane *m_pFrustum;
    ITexture *m_pMaskTexture;
    IVRenderView *m_pRenderView;
    IMaterialSystem *m_pMaterialSystem;
};

// Purpose: singleton accessor
extern CGameUI &GameUI();

// expose client interface
extern IGameClientExports *GameClientExports();

#endif // GAMEUI_INTERFACE_H
