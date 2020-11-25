#pragma once

#include <steam/steam_api_common.h>
#include <steam/isteammatchmaking.h>

#include "igameevents.h"
#include "vgui_controls/EditablePanel.h"

enum
{
    CHAT_INTERFACE_LINES = 6,
    MAX_CHARS_PER_LINE = 128
};

enum TextColor
{
    COLOR_NORMAL = 1,
    COLOR_USEOLDCOLORS = 2,
    COLOR_PLAYERNAME = 3,
    COLOR_LOCATION = 4,
    COLOR_ACHIEVEMENT = 5,
    COLOR_CUSTOM = 6,        // Will use the most recently SetCustomColor()
    COLOR_HEXCODE = 7,       // Reads the color from the next six characters
    COLOR_HEXCODE_ALPHA = 8, // Reads the color and alpha from the next eight characters
    COLOR_MAX
};

enum MessageMode_t
{
    MESSAGE_MODE_NONE = 0,
    MESSAGE_MODE_HUD,
    MESSAGE_MODE_MENU
};

enum ChatValidationState_t
{
    CHAT_STATE_OK = 0,
    CHAT_STATE_SPAMMING, // This user is spamming
    CHAT_STATE_EMPTY,    // this message is empty
    CHAT_STATE_TOO_LONG, // this message is too long
    CHAT_STATE_MUTED,    // this user is temporarily muted by us
    CHAT_STATE_COMMAND,  // This message is a command
};

#define MAX_CHAT_LENGTH 256
#define CHAT_HISTORY_FADE_TIME 0.25f

class ChatEntry;
class ChatHistory;
class ChatLine;
class ChatFilterPanel;

// Use this class to draw the chat somewhere. It will fill to the bounds of this container exactly.
class ChatContainer : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(ChatContainer, EditablePanel);
    ChatContainer(Panel *pParent);

    void Paint() override;
    void SetVisible(bool state) override;

    void StartMessageMode(MessageMode_t mode);
    void StopMessageMode();
    void SetAutomaticMessageMode(MessageMode_t mode) { m_hAutomaticMessageMode = mode; }

    MESSAGE_FUNC(OnStopMessageMode, "StopMessageMode");

private:
    MessageMode_t m_hAutomaticMessageMode;
};

class ChatPanel : public vgui::EditablePanel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(ChatPanel, vgui::EditablePanel);

    ChatPanel();
    ~ChatPanel();

    static void Init();

    void Reset();

    void MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);

    void Printf(int iFilter, PRINTF_FORMAT_STRING const char *fmt, ...);
    void FMTFUNCTION_WIN(4, 5) ChatPrintf(int iPlayerIndex, int iFilter, PRINTF_FORMAT_STRING const char *fmt, ...) FMTFUNCTION(4, 5);

    void ClearCurrentTypingStatus();

    void StartMessageMode(int iMessageModeType);
    void StopMessageMode(float fFadeoutOverride = CHAT_HISTORY_FADE_TIME);
    void Send();

    bool SendMessageToLobby(const char *pText);

    bool ValidateAndSendMessageLocal(const char *pText);
    ChatValidationState_t ValidateChatMessage(CUtlString &textStr, const CSteamID &playerID);
    void FormatAndPrintMessage(const CUtlString &textStr, const CSteamID &playerID);

    MESSAGE_FUNC(OnChatEntrySend, "ChatEntrySend");
    MESSAGE_FUNC(OnChatEntryStopMessageMode, "ChatEntryStopMessageMode");

    int GetChatInputOffset() const { return m_iFontHeight; }

    ChatHistory *GetChatHistory() const { return m_pChatHistory; }
    ChatEntry *GetChatInput() const { return m_pChatInput; }

    void FadeChatHistory();
    float m_flHistoryFadeTime;
    float m_flHistoryIdleTime;

    int GetFilterFlags() { return m_iFilterFlags; }
    void SetFilterFlag(int iFilter);

    Color GetDefaultTextColor() const { return m_cDefaultTextColor; }
    Color GetTextColorForClient(TextColor colorNum, int clientIndex);
    Color GetClientColor(int clientIndex);

    int GetFilterForString(const char *pString);

    bool IsVoiceSubtitle() { return m_bEnteringVoice; }
    void SetVoiceSubtitleState(bool bState) { m_bEnteringVoice = bState; }
    int GetMessageMode() { return m_nMessageMode; }

    void SetCustomColor(Color colNew) { m_ColorCustom = colNew; }

    // MOM_TODO: Move these elsewhere. Maybe in clientmode? Something that has access to multiple UI components.
    STEAM_CALLBACK(ChatPanel, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(ChatPanel, OnLobbyMessage, LobbyChatMsg_t);
    STEAM_CALLBACK(ChatPanel, OnLobbyDataUpdate, LobbyDataUpdate_t);

    // talk control
    void	NotePlayerTalked() { m_fLastPlayerTalkTime = gpGlobals->curtime; }
    float	LastTimePlayerTalked() { return m_fLastPlayerTalkTime; }

protected:
    void FireGameEvent(IGameEvent *event) override;

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnKeyCodeTyped(vgui::KeyCode code) override;
    void OnTick() override;
    void OnThink() override;
    void OnCommand(const char *command) override;
    void PerformLayout() override;

    ChatEntry *m_pChatInput;
    ChatLine *m_ChatLine;
    int m_iFontHeight;

    ChatHistory *m_pChatHistory;

    vgui::Button *m_pFiltersButton;
    ChatFilterPanel *m_pFilterPanel;

    Color m_ColorCustom;

    CPanelAnimationVar(int, m_iHistoryAlpha, "HistoryAlpha", "192");

private:
    void Clear();

    void GetTimestamp(char *pBuffer, int maxLen);
    int ComputeBreakChar(int width, const char *text, int textlen);
    void UpdateTypingMembersLabel();
    bool PlayerIsSpamming(CUtlMap<uint64, float> &mapOfDelays, uint64 person, float delayAmount);

    int m_nMessageMode;

    vgui::HFont m_hChatFont;

    int m_iFilterFlags;
    bool m_bEnteringVoice;

    CUtlMap<uint64, float> m_mapChatterSpammerBlocker;
    CUtlMap<uint64, float> m_mapPlayerSpawnerSpammerBlocker;
    CUtlVector<uint64> m_vTypingMembers;
    CUtlVector<uint64> m_vMomentumOfficers;
    CSteamID m_LobbyID;

    Color m_cDefaultTextColor;
    bool m_bTyping;
    bool m_bIsVisible;

    float m_fLastPlayerTalkTime;

    vgui::Label *m_pTypingMembers;
};

extern ChatPanel *g_pChatPanel;