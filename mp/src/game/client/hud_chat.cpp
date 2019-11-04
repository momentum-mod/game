#include "cbase.h"

#include <ctime>
#include "hud_basechat.h"
#include "hud_chat.h"
#include "hud_macros.h"
#include "momentum/mom_shareddefs.h"

#include "hud_spectatorinfo.h"
#include "vgui/IScheme.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudChat);

DECLARE_HUD_MESSAGE(CHudChat, SayText);
DECLARE_HUD_MESSAGE(CHudChat, SayText2);
DECLARE_HUD_MESSAGE(CHudChat, TextMsg);
DECLARE_HUD_MESSAGE(CHudChat, LobbyUpdateMsg);
DECLARE_HUD_MESSAGE(CHudChat, SpecUpdateMsg);

using namespace vgui;

MAKE_TOGGLE_CONVAR(mom_chat_timestamps_enable, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE,
            "Toggles timestamps on chat messages. 0 = OFF, 1 = ON\n");

//=====================
// CHudChat
//=====================

CHudChat::CHudChat(const char *pElementName) : BaseClass(pElementName), m_pSpectatorInfo(nullptr)
{
    m_bIsVisible = m_bTyping = false;

    m_pTypingMembers = new Label(this, "TypingMembers", "");

    LoadControlSettings("resource/ui/BaseChat.res");

    ListenForGameEvent("lobby_leave");
}

void CHudChat::Init(void)
{
    BaseClass::Init();

    HOOK_HUD_MESSAGE(CHudChat, SayText);
    HOOK_HUD_MESSAGE(CHudChat, SayText2);
    HOOK_HUD_MESSAGE(CHudChat, TextMsg);
    HOOK_HUD_MESSAGE(CHudChat, LobbyUpdateMsg);
    HOOK_HUD_MESSAGE(CHudChat, SpecUpdateMsg);

    //@tuxxi: I tired to query this automatically but we can only query steamgroups we are members of.. rip.
    // So i'm just hard coding this here for now
    // https://partner.steamgames.com/doc/api/ISteamFriends#RequestClanOfficerList

    // MOM_TODO: query web API for officers / members/ other groups instead of this crap
    m_vMomentumOfficers.AddToTail(76561198018587940ull); // tuxxi
    m_vMomentumOfficers.AddToTail(76561197979963054ull); // gocnak
    m_vMomentumOfficers.AddToTail(76561198047369620ull); // rusty
    m_vMomentumOfficers.AddToTail(76561197982874432ull); // juxtapo
}

void CHudChat::OnLobbyEnter(LobbyEnter_t *pParam)
{
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
        m_LobbyID = pParam->m_ulSteamIDLobby;
}

void CHudChat::OnLobbyMessage(LobbyChatMsg_t *pParam)
{
    const CSteamID msgSender = CSteamID(pParam->m_ulSteamIDUser);
    /*
    if (msgSender == SteamUser()->GetSteamID().ConvertToUint64())
    {
        //DevLog("Got our own message! Just ignoring it...\n");
        return;
    }
    */
    const bool isMomentumTeam = m_vMomentumOfficers.IsValidIndex(m_vMomentumOfficers.Find(pParam->m_ulSteamIDUser));
    char personName[MAX_PLAYER_NAME_LENGTH];
    Q_strncpy(personName, SteamFriends()->GetFriendPersonaName(msgSender), MAX_PLAYER_NAME_LENGTH);

    const char *spectatingText = SteamMatchmaking()->GetLobbyMemberData(m_LobbyID, msgSender, LOBBY_DATA_IS_SPEC);
    const bool isSpectating = spectatingText != nullptr && Q_strlen(spectatingText) > 0;

    char timestamp[16] = "";
    if (mom_chat_timestamps_enable.GetBool())
        GetTimestamp(timestamp, 16);

    char message[4096];
    // MOM_TODO: This won't be just text in the future, if we capitalize on being able to send binary data. Wrap this is
    // something and parse it
    SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID,
                                                           nullptr, message, 4096, nullptr);
    SetCustomColor(COLOR_RED);
    ChatPrintf(1, CHAT_FILTER_NONE, "%s%c%s%s%c: %s",
               timestamp,
               isMomentumTeam ? COLOR_CUSTOM : COLOR_PLAYERNAME,
               isSpectating ? "*SPEC* " : "", 
               personName,
               COLOR_NORMAL,
               ConvertCRtoNL(message));
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pszName -
//          iSize -
//          *pbuf -
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
    const char *pName = SteamFriends()->GetFriendPersonaName(personID);

    msg.ReadBytes(&target, sizeof(uint64));
    CSteamID targetID = CSteamID(target);

    const char *spectateText = target != 1 ? "%s is now spectating." : "%s is now watching a replay.";
    if (type == SPEC_UPDATE_STOP)
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
        const char *pTargetName = SteamFriends()->GetFriendPersonaName(targetID);
        DevLog("%s is now spectating %s.\n", pName, pTargetName);
        // Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG,
        //    "%s is now spectating %s.", pName, pTargetName);
    }

    SpectatorUpdate(personID, target);
}

void CHudChat::MsgFunc_LobbyUpdateMsg(bf_read &msg)
{
    uint8 type = msg.ReadByte();

    bool isJoin = (type == LOBBY_UPDATE_MEMBER_JOIN_MAP) || (type == LOBBY_UPDATE_MEMBER_JOIN);
    bool isMap = (type == LOBBY_UPDATE_MEMBER_JOIN_MAP) || (type == LOBBY_UPDATE_MEMBER_LEAVE_MAP);

    uint64 person;
    msg.ReadBytes(&person, sizeof(uint64));
    CSteamID personID = CSteamID(person);
    const char *pName = SteamFriends()->GetFriendPersonaName(personID);
    Printf(CHAT_FILTER_JOINLEAVE | CHAT_FILTER_SERVERMSG, "%s has %s the %s.", pName, isJoin ? "joined" : "left",
           isMap ? "map" : "lobby");

    if (!isJoin)
        SpectatorUpdate(personID, k_steamIDNil);
}

void CHudChat::GetTimestamp(char *pBuffer, int maxLen)
{
    time_t now = time(nullptr);
    struct tm *tm = localtime(&now);
    Q_snprintf(pBuffer, maxLen, "%c%s[%02d:%02d] ", COLOR_HEXCODE, "CCCCCC", tm->tm_hour, tm->tm_min);
}

void CHudChat::Printf(int iFilter, const char *fmt, ...)
{
    va_list marker;
    char msg[4096];

    va_start(marker, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, marker);
    va_end(marker);

    char timestamp[16] = "";
    if (mom_chat_timestamps_enable.GetBool())
        GetTimestamp(timestamp, 16);

    ChatPrintf(0, iFilter, "%s%c%s", timestamp, COLOR_NORMAL, msg);
}

void CHudChat::StartMessageMode(int iMessageMode)
{
    BaseClass::StartMessageMode(iMessageMode);
    m_bIsVisible = true;
    m_pTypingMembers->SetVisible(true);
}

void CHudChat::StopMessageMode()
{
    BaseClass::StopMessageMode();

    if (m_LobbyID.IsValid())
        SteamMatchmaking()->SetLobbyMemberData(m_LobbyID, LOBBY_DATA_TYPING, nullptr);

    // Can't be typing if we close the chat
    m_bIsVisible = m_bTyping = false;
    m_pTypingMembers->SetVisible(false);
}

void CHudChat::FireGameEvent(IGameEvent *event)
{
    if (FStrEq(event->GetName(), "lobby_leave"))
    {
        m_LobbyID.Clear();
        m_vTypingMembers.RemoveAll();
    }

    BaseClass::FireGameEvent(event);
}

void CHudChat::OnThink()
{
    BaseClass::OnThink();

    if (m_LobbyID.IsValid() && GetMessageMode() != 0 && GetInputPanel())
    {
        const bool isSomethingTyped = GetInputPanel()->GetTextLength() > 0;
        if (isSomethingTyped != m_bTyping)
        {
            m_bTyping = isSomethingTyped;
            SteamMatchmaking()->SetLobbyMemberData(m_LobbyID, LOBBY_DATA_TYPING, m_bTyping ? "y" : nullptr);
        }
    }

    const int count = m_vTypingMembers.Count();
    if (m_bIsVisible)
    {
        if (count > 0)
        {
            CUtlString typingText;
            if (count <= 3)
            {
                FOR_EACH_VEC(m_vTypingMembers, i)
                {
                    typingText.Append(SteamFriends()->GetFriendPersonaName(CSteamID(m_vTypingMembers[i])));
                    typingText.Append(i < count - 1 ? ", " : " ");
                }
                typingText.Append("typing...");
            }
            else
            {
                typingText.Format("%d people are typing...", count);
            }

            m_pTypingMembers->SetText(typingText.Get());
        }
        else
        {
            m_pTypingMembers->SetText("");
        }
    }
}

void CHudChat::OnLobbyDataUpdate(LobbyDataUpdate_t *pParam)
{
    // If something other than the lobby and local player...
    if (pParam->m_bSuccess &&
        pParam->m_ulSteamIDLobby != pParam->m_ulSteamIDMember &&
        pParam->m_ulSteamIDMember != SteamUser()->GetSteamID().ConvertToUint64())
    {
        // Typing Status
        const char *typingText = SteamMatchmaking()->GetLobbyMemberData(m_LobbyID, pParam->m_ulSteamIDMember, LOBBY_DATA_TYPING);
        const bool isTyping = typingText != nullptr && Q_strlen(typingText) == 1;
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

Color CHudChat::GetDefaultTextColor(void) // why the fuck is this not a .res file color in CHudBaseChat !?!?!?
{
    return m_cDefaultTextColor;
}

void CHudChat::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cDefaultTextColor = pScheme->GetColor("OffWhite", COLOR_WHITE);
}

void CHudChat::SpectatorUpdate(const CSteamID &personID, const CSteamID &target)
{
    if (!m_pSpectatorInfo)
        m_pSpectatorInfo = GET_HUDELEMENT(CHudSpectatorInfo);

    if (m_pSpectatorInfo)
        m_pSpectatorInfo->SpectatorUpdate(personID, target);
}
