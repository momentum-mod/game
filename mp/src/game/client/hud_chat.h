//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_CHAT_H
#define HUD_CHAT_H
#ifdef _WIN32
#pragma once
#endif

#include <hud_basechat.h>
#include "steam/steam_api.h"

class CHudSpectatorInfo;

class CHudChat : public CBaseHudChat
{
    DECLARE_CLASS_SIMPLE(CHudChat, CBaseHudChat);

public:
    CHudChat(const char *pElementName);

    virtual void	Init(void);

    void MsgFunc_SayText(bf_read &msg);
    void MsgFunc_SpecUpdateMsg(bf_read &msg);
    void MsgFunc_LobbyUpdateMsg(bf_read &msg);

    // MOM_TODO: Move these elsewhere. Maybe in clientmode? Something that has access to multiple UI components.
    STEAM_CALLBACK(CHudChat, OnLobbyMessage, LobbyChatMsg_t);
    STEAM_CALLBACK(CHudChat, OnLobbyDataUpdate, LobbyDataUpdate_t);

    void StartMessageMode(int) OVERRIDE;
    void StopMessageMode() OVERRIDE;

    void Paint() OVERRIDE;
    void OnThink() OVERRIDE;
    Color GetDefaultTextColor() OVERRIDE;

private:
    CUtlVector<CSteamID> m_vTypingMembers;
    CUtlVector<CSteamID> m_vMomentumOfficers;

    vgui::HFont m_hfInfoTextFont;
    Color m_cInfoTextColor, m_cDefaultTextColor;
    bool m_bTyping;
    bool m_bIsVisible;

    CHudSpectatorInfo *m_pSpectatorInfo;
};

#endif	//HUD_CHAT_H