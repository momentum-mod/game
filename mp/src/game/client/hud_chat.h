#pragma once

#include <hud_basechat.h>
#include "steam/steam_api.h"

class CHudSpectatorInfo;

class CHudChat : public CBaseHudChat
{
    DECLARE_CLASS_SIMPLE(CHudChat, CBaseHudChat);

  public:
    CHudChat(const char *pElementName);

    virtual void Init(void);

    void MsgFunc_SayText(bf_read &msg);
    void MsgFunc_SpecUpdateMsg(bf_read &msg);
    void MsgFunc_LobbyUpdateMsg(bf_read &msg);

    // MOM_TODO: Move these elsewhere. Maybe in clientmode? Something that has access to multiple UI components.
    STEAM_CALLBACK(CHudChat, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(CHudChat, OnLobbyMessage, LobbyChatMsg_t);
    STEAM_CALLBACK(CHudChat, OnLobbyDataUpdate, LobbyDataUpdate_t);

    void GetTimestamp(char *pOut, int maxLen);
    void Printf(int iFilter, const char *fmt, ...) OVERRIDE;

    void StartMessageMode(int) OVERRIDE;
    void StopMessageMode() OVERRIDE;
    void FireGameEvent(IGameEvent *event) OVERRIDE;

    void OnThink() OVERRIDE;
    Color GetDefaultTextColor() OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

  private:
    void SpectatorUpdate(const CSteamID &person, const CSteamID &target);

    CUtlVector<uint64> m_vTypingMembers;
    CUtlVector<uint64> m_vMomentumOfficers;
    CSteamID m_LobbyID;

    Color m_cDefaultTextColor;
    bool m_bTyping;
    bool m_bIsVisible;

    vgui::Label *m_pTypingMembers;
    CHudSpectatorInfo *m_pSpectatorInfo;
};