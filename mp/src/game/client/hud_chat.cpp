//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include <vgui/ILocalize.h>
#include "hud_basechat.h"
#include "hud_chat.h"
#include "hud_macros.h"
#include "momentum/mom_shareddefs.h"

#include "clientmode.h"
#include "hud_spectatorinfo.h"
#include "mom_steam_helper.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudChat);

DECLARE_HUD_MESSAGE(CHudChat, SayText);
DECLARE_HUD_MESSAGE(CHudChat, SayText2);
DECLARE_HUD_MESSAGE(CHudChat, TextMsg);
DECLARE_HUD_MESSAGE(CHudChat, LobbyUpdateMsg);
DECLARE_HUD_MESSAGE(CHudChat, SpecUpdateMsg);

//=====================
// CHudChat
//=====================

CHudChat::CHudChat(const char *pElementName) : BaseClass(pElementName), m_pSpectatorInfo(nullptr)
{
    m_vTypingMembers = CUtlVector<CSteamID>();
    m_hfInfoTextFont = 0;

    m_bIsVisible = m_bTyping = false;
}

void CHudChat::Init(void)
{
    BaseClass::Init();
    IScheme *pChatScheme = scheme()->GetIScheme(scheme()->GetScheme("ChatScheme"));
    if (pChatScheme)
    {
        m_hfInfoTextFont = pChatScheme->GetFont("ChatFont");
        m_cInfoTextColor = pChatScheme->GetColor("OffWhite", COLOR_WHITE);
        m_cDefaultTextColor = pChatScheme->GetColor("OffWhite", COLOR_WHITE);
    }

    HOOK_HUD_MESSAGE(CHudChat, SayText);
    HOOK_HUD_MESSAGE(CHudChat, SayText2);
    HOOK_HUD_MESSAGE(CHudChat, TextMsg);
    HOOK_HUD_MESSAGE(CHudChat, LobbyUpdateMsg);
    HOOK_HUD_MESSAGE(CHudChat, SpecUpdateMsg);

    //@tuxxi: I tired to query this automatically but we can only query steamgroups we are members of.. rip.
    // So i'm just hard coding this here for now
    // https://partner.steamgames.com/doc/api/ISteamFriends#RequestClanOfficerList

    // MOM_TODO: query wep API for officers / members/ other groups instead of this crap
    m_vMomentumOfficers.AddToTail(CSteamID(uint64(76561198018587940))); // tuxxi
    m_vMomentumOfficers.AddToTail(CSteamID(uint64(76561197979963054))); // gocnak
    m_vMomentumOfficers.AddToTail(CSteamID(uint64(76561198011358548))); // ruben
    m_vMomentumOfficers.AddToTail(CSteamID(uint64(76561198047369620))); // rusty
    m_vMomentumOfficers.AddToTail(CSteamID(uint64(76561197982874432))); // juxtapo
}

void CHudChat::OnLobbyMessage(LobbyChatMsg_t *pParam)
{
    const CSteamID msgSender = CSteamID(pParam->m_ulSteamIDUser);
    /*
    if (msgSender == steamapicontext->SteamUser()->GetSteamID().ConvertToUint64())
    {
        //DevLog("Got our own message! Just ignoring it...\n");
        return;
    }
    */
    bool isMomentumTeam = false;
    FOR_EACH_VEC(m_vMomentumOfficers, i)
    {
        if (m_vMomentumOfficers[i] == msgSender)
        {
            isMomentumTeam = true;
            break;
        }
    }
    char personName[MAX_PLAYER_NAME_LENGTH];
    Q_strncpy(personName, steamapicontext->SteamFriends()->GetFriendPersonaName(msgSender), MAX_PLAYER_NAME_LENGTH);

    const char *spectatingText = g_pMomentumSteamHelper->GetLobbyMemberData(msgSender, LOBBY_DATA_IS_SPEC);
    const bool isSpectating = spectatingText != nullptr && Q_strlen(spectatingText) > 0;
    char message[4096];
    // MOM_TODO: This won't be just text in the future, if we captialize on being able to send binary data. Wrap this is
    // something and parse it
    steamapicontext->SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID,
                                                           nullptr, message, 4096, nullptr);
    SetCustomColor(COLOR_RED);
    ChatPrintf(1, CHAT_FILTER_NONE, "%c%s%s%c: %s", 
               isMomentumTeam ? COLOR_CUSTOM : COLOR_PLAYERNAME,
               isSpectating ? "*SPEC* " : "", 
               personName,
               COLOR_NORMAL,
               ConvertCRtoNL(message));
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pszName -
//			iSize -
//			*pbuf -
//-----------------------------------------------------------------------------
void CHudChat::MsgFunc_SayText(bf_read &msg)
{
    /*
    //MOM_TODO: @tuxxi: I commented this out for now because having the formatting and colors line
    //up with the local sayText vs the lobby SayText is a nightmare.
    char szString[256];

    msg.ReadByte(); // client ID
    msg.ReadString(szString, sizeof(szString));
    Printf(CHAT_FILTER_NONE, "%s", szString);
    */
}

void CHudChat::MsgFunc_SpecUpdateMsg(bf_read &msg)
{
    uint8 type = msg.ReadByte();

    uint64 person, target;
    msg.ReadBytes(&person, sizeof(uint64));
    CSteamID personID = CSteamID(person);
    const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(personID);

    msg.ReadBytes(&target, sizeof(uint64));
    CSteamID targetID = CSteamID(target);

    const char *spectateText = target != 1 ? "%s is now spectating." : +"%s is now watching a replay.";
    if (type == SPEC_UPDATE_LEAVE)
    {
        Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG, "%s has respawned.", pName);
    }
    else if (type == SPEC_UPDATE_JOIN)
    {
        Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG, spectateText, pName);
    }
    else if (type == SPEC_UPDATE_CHANGETARGET)
    {
        // MOM_TODO: Removeme?
        const char *pTargetName = steamapicontext->SteamFriends()->GetFriendPersonaName(targetID);
        DevLog("%s is now spectating %s.\n", pName, pTargetName);
        // Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG,
        //    "%s is now spectating %s.", pName, pTargetName);
    }

    if (!m_pSpectatorInfo)
        m_pSpectatorInfo = GET_HUDELEMENT(CHudSpectatorInfo);

    if (m_pSpectatorInfo)
        m_pSpectatorInfo->SpectatorUpdate(personID, targetID);
}

void CHudChat::MsgFunc_LobbyUpdateMsg(bf_read &msg)
{
    uint8 type = msg.ReadByte();

    bool isJoin = (type == LOBBY_UPDATE_MEMBER_JOIN_MAP) || (type == LOBBY_UPDATE_MEMBER_JOIN);
    bool isMap = (type == LOBBY_UPDATE_MEMBER_JOIN_MAP) || (type == LOBBY_UPDATE_MEMBER_LEAVE_MAP);

    uint64 person;
    msg.ReadBytes(&person, sizeof(uint64));
    CSteamID personID = CSteamID(person);
    const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(personID);
    Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG, "%s has %s the %s.", pName, isJoin ? "joined" : "left",
           isMap ? "map" : "lobby");
}

void CHudChat::StartMessageMode(int iMessageMode)
{
    BaseClass::StartMessageMode(iMessageMode);
    m_bIsVisible = true;
}

void CHudChat::StopMessageMode()
{
    BaseClass::StopMessageMode();

    g_pMomentumSteamHelper->SetLobbyMemberData(LOBBY_DATA_TYPING, nullptr);

    // Can't be typing if we close the chat
    m_bIsVisible = m_bTyping = false;
}

void CHudChat::OnThink()
{
    if (g_pMomentumSteamHelper->IsLobbyValid() && GetMessageMode() != 0 && GetInputPanel())
    {
        const int isSomethingTyped = GetInputPanel()->GetTextLength() > 0;
        if (isSomethingTyped && !m_bTyping)
        {
            g_pMomentumSteamHelper->SetLobbyMemberData(LOBBY_DATA_TYPING, "y");
            m_bTyping = true;
        }
        else if (!isSomethingTyped && m_bTyping)
        {
            g_pMomentumSteamHelper->SetLobbyMemberData(LOBBY_DATA_TYPING, nullptr);
            m_bTyping = false;
        }
    }
}

void CHudChat::OnLobbyDataUpdate(LobbyDataUpdate_t *pParam)
{
    // If something other than the lobby...
    if (pParam->m_bSuccess && pParam->m_ulSteamIDLobby != pParam->m_ulSteamIDMember)
    {
        // Typing Status
        const char *typingText =
            g_pMomentumSteamHelper->GetLobbyMemberData(pParam->m_ulSteamIDMember, LOBBY_DATA_TYPING);
        const bool isTyping = typingText != nullptr && Q_strlen(typingText) > 0;
        const int typingIndex = m_vTypingMembers.Find(pParam->m_ulSteamIDMember);
        const bool isValidIndex = m_vTypingMembers.IsValidIndex(typingIndex);
        if (isTyping)
        {
            if (!isValidIndex)
                m_vTypingMembers.AddToTail(pParam->m_ulSteamIDMember);
        }
        else if (isValidIndex)
        {
            m_vTypingMembers.FastRemove(typingIndex);
        }
    }
}

void CHudChat::Paint()
{
    BaseClass::Paint();
    if (m_vTypingMembers.Count() > 0 && m_bIsVisible)
    {
        char typingText[BUFSIZ];
        // This line is a shameful reminder of my lack of control over C-strings
        typingText[0] = '\0';
        wchar_t wcTypingText[BUFSIZ];
        if (m_vTypingMembers.Count() <= 3)
        {
            char nameChunk[MAX_PLAYER_NAME_LENGTH + 3];
            for (int i = 0; i < m_vTypingMembers.Count(); i++)
            {
                V_strncpy(nameChunk,
                          steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(m_vTypingMembers[i])),
                          MAX_PLAYER_NAME_LENGTH);
                V_strcat(nameChunk, i < m_vTypingMembers.Count() - 1 ? ", " : " ", MAX_PLAYER_NAME_LENGTH + 2);
                V_strcat(typingText, nameChunk, BUFSIZ);
            }
            V_strcat(typingText, "typing...", BUFSIZ);
        }
        else
        {
            V_snprintf(typingText, BUFSIZ, "%d people typing...", m_vTypingMembers.Count());
        }
        const int count = g_pVGuiLocalize->ConvertANSIToUnicode(typingText, wcTypingText, BUFSIZ);
        int w, h;
        GetSize(w, h);
        vgui::surface()->DrawSetTextFont(m_hfInfoTextFont);
        vgui::surface()->DrawSetTextPos(20, h - 24);
        vgui::surface()->DrawSetTextColor(m_cInfoTextColor);
        vgui::surface()->DrawPrintText(wcTypingText, count);
    }
}

Color CHudChat::GetDefaultTextColor(void) // why the fuck is this not a .res file color in CHudBaseChat !?!?!?
{
    return m_cDefaultTextColor;
}
