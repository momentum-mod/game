#pragma once

#include <vgui_controls/CheckButton.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/TextEntry.h>
#include "hudelement.h"

class CBaseHudChat;
class CBaseHudChatInputLine;
class CBaseHudChatEntry;
class CHudChatFilterPanel;

namespace vgui
{
class IScheme;
class Panel;
}; // namespace vgui

#define CHATLINE_NUM_FLASHES 8.0f
#define CHATLINE_FLASH_TIME 5.0f
#define CHATLINE_FADE_TIME 1.0f

#define CHAT_HISTORY_FADE_TIME 0.25f
#define CHAT_HISTORY_IDLE_TIME 15.0f
#define CHAT_HISTORY_IDLE_FADE_TIME 2.5f

extern Color g_ColorBlue;
extern Color g_ColorRed;
extern Color g_ColorGreen;
extern Color g_ColorDarkGreen;
extern Color g_ColorYellow;
extern Color g_ColorGrey;

extern ConVar cl_showtextmsg;

enum ChatFilters
{
    CHAT_FILTER_NONE = 0,
    CHAT_FILTER_JOINLEAVE = 0x000001,
    CHAT_FILTER_NAMECHANGE = 0x000002,
    CHAT_FILTER_PUBLICCHAT = 0x000004,
    CHAT_FILTER_SERVERMSG = 0x000008,
    CHAT_FILTER_TEAMCHANGE = 0x000010,
    //=============================================================================
    // HPE_BEGIN:
    // [tj]Added a filter for achievement announce
    //=============================================================================

    CHAT_FILTER_ACHIEVEMENT = 0x000020,

    //=============================================================================
    // HPE_END
    //=============================================================================
};

//-----------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------------------
struct TextRange
{
    TextRange() { preserveAlpha = false; }
    int start;
    int end;
    Color color;
    bool preserveAlpha;
};

void StripEndNewlineFromString(char *str);
void StripEndNewlineFromString(wchar_t *str);

char *ConvertCRtoNL(char *str);
wchar_t *ConvertCRtoNL(wchar_t *str);
wchar_t *ReadLocalizedString(bf_read &msg, OUT_Z_BYTECAP(outSizeInBytes) wchar_t *pOut, int outSizeInBytes,
                             bool bStripNewline, OUT_Z_CAP(originalSize) char *originalString = NULL,
                             int originalSize = 0);
wchar_t *ReadChatTextString(bf_read &msg, OUT_Z_BYTECAP(outSizeInBytes) wchar_t *pOut, int outSizeInBytes);
char *RemoveColorMarkup(char *str);

//--------------------------------------------------------------------------------------------------------
/**
 * Simple utility function to allocate memory and duplicate a wide string
 */
inline wchar_t *CloneWString(const wchar_t *str)
{
    const int nLen = V_wcslen(str) + 1;
    wchar_t *cloneStr = new wchar_t[nLen];
    const int nSize = nLen * sizeof(wchar_t);
    V_wcsncpy(cloneStr, str, nSize);
    return cloneStr;
}

//-----------------------------------------------------------------------------
// Purpose: An output/display line of the chat interface
//-----------------------------------------------------------------------------
class CBaseHudChatLine : public vgui::RichText
{
    DECLARE_CLASS_SIMPLE(CBaseHudChatLine, RichText);

    CBaseHudChatLine(Panel *parent, const char *panelName);
    ~CBaseHudChatLine();

    void SetExpireTime();

    bool IsReadyToExpire();

    void Expire();

    float GetStartTime();

    int GetCount();

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

    vgui::HFont GetFont() { return m_hFont; }

    Color GetTextColor() { return m_clrText; }
    void SetNameLength(int iLength) { m_iNameLength = iLength; }
    void SetNameColor(Color cColor) { m_clrNameColor = cColor; }

    virtual void PerformFadeout();
    virtual void InsertAndColorizeText(wchar_t *buf, int clientIndex);
    virtual void Colorize(int alpha = 255); ///< Re-inserts the text in the appropriate colors at the given alpha

    void SetNameStart(int iStart) { m_iNameStart = iStart; }

  protected:
    int m_iNameLength;
    vgui::HFont m_hFont;

    Color m_clrText;
    Color m_clrNameColor;

    float m_flExpireTime;

    CUtlVector<TextRange> m_textRanges;
    wchar_t *m_text;

    int m_iNameStart;

  private:
    float m_flStartTime;
    int m_nCount;

    vgui::HFont m_hFontMarlett;
    CBaseHudChat *m_pChat;

    CBaseHudChatLine(const CBaseHudChatLine &); // not defined, not accessible
};

class CHudChatHistory : public vgui::RichText
{
    DECLARE_CLASS_SIMPLE(CHudChatHistory, vgui::RichText);

  public:
    CHudChatHistory(Panel *pParent, const char *panelName);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
};

class CHudChatFilterCheckButton : public vgui::CheckButton
{
    DECLARE_CLASS_SIMPLE(CHudChatFilterCheckButton, vgui::CheckButton);

  public:
    CHudChatFilterCheckButton(Panel *pParent, const char *pName, const char *pText, int iFlag);

    int GetFilterFlag() { return m_iFlag; }

  private:
    int m_iFlag;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CBaseHudChat : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CBaseHudChat, vgui::EditablePanel);

  public:
    DECLARE_MULTIPLY_INHERITED();

    enum
    {
        CHAT_INTERFACE_LINES = 6,
        MAX_CHARS_PER_LINE = 128
    };

    CBaseHudChat(const char *pElementName);
    ~CBaseHudChat();

    void Init() override;

    void LevelInit() override;
    void LevelShutdown() override;

    void MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);

    virtual void Printf(int iFilter, PRINTF_FORMAT_STRING const char *fmt, ...);
    virtual void FMTFUNCTION_WIN(4, 5) ChatPrintf(int iPlayerIndex, int iFilter, PRINTF_FORMAT_STRING const char *fmt, ...) FMTFUNCTION(4, 5);

    virtual void StartMessageMode(int iMessageModeType);
    virtual void StopMessageMode();
    void Send();

    MESSAGE_FUNC(OnChatEntrySend, "ChatEntrySend");
    MESSAGE_FUNC(OnChatEntryStopMessageMode, "ChatEntryStopMessageMode");

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnKeyCodeReleased(vgui::KeyCode code) override;
    void Paint() override;
    void OnTick() override;
    void Reset() override;
    void OnCommand(const char* command) OVERRIDE;
    CBaseHudChatEntry *GetInputPanel();

    static int m_nLineCounter;

    virtual int GetChatInputOffset();

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) override;

    CHudChatHistory *GetChatHistory() const { return m_pChatHistory; }

    void FadeChatHistory();
    float m_flHistoryFadeTime;
    float m_flHistoryIdleTime;

    virtual void MsgFunc_SayText(bf_read &msg);
    virtual void MsgFunc_SayText2(bf_read &msg);
    virtual void MsgFunc_TextMsg(bf_read &msg);

    CBaseHudChatEntry *GetChatInput() { return m_pChatInput; }

    virtual int GetFilterFlags() { return m_iFilterFlags; }
    void SetFilterFlag(int iFilter);

    //-----------------------------------------------------------------------------
    virtual Color GetDefaultTextColor();
    virtual Color GetTextColorForClient(TextColor colorNum, int clientIndex);
    virtual Color GetClientColor(int clientIndex);

    virtual int GetFilterForString(const char *pString);

    virtual const char *GetDisplayedSubtitlePlayerName(int clientIndex);

    bool IsVoiceSubtitle() { return m_bEnteringVoice; }
    void SetVoiceSubtitleState(bool bState) { m_bEnteringVoice = bState; }
    int GetMessageMode() { return m_nMessageMode; }

    void SetCustomColor(Color colNew) { m_ColorCustom = colNew; }
    void SetCustomColor(const char *pszColorName);

  protected:
    CBaseHudChatLine *FindUnusedChatLine();

    CBaseHudChatEntry *m_pChatInput;
    CBaseHudChatLine *m_ChatLine;
    int m_iFontHeight;

    CHudChatHistory *m_pChatHistory;

    vgui::Button *m_pFiltersButton;
    CHudChatFilterPanel *m_pFilterPanel;

    Color m_ColorCustom;

    CPanelAnimationVar(int, m_iHistoryAlpha, "HistoryAlpha", "192");

  private:
    void Clear();

    int ComputeBreakChar(int width, const char *text, int textlen);

    int m_nMessageMode;

    int m_nVisibleHeight;

    vgui::HFont m_hChatFont;

    int m_iFilterFlags;
    bool m_bEnteringVoice;
};

class CBaseHudChatEntry : public vgui::TextEntry
{
    DECLARE_CLASS_SIMPLE(CBaseHudChatEntry, TextEntry);

    CBaseHudChatEntry(Panel *parent, char const *panelName);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnKeyCodeTyped(vgui::KeyCode code) override;

    CPanelAnimationStringVar(32, m_FontName, "font", "Default");
    CPanelAnimationVar(Color, m_cTypingColor, "TypingText", "White");
};

class CHudChatFilterPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudChatFilterPanel, vgui::EditablePanel);

  public:
    CHudChatFilterPanel(Panel *pParent, const char *pName);

    MESSAGE_FUNC_PTR(OnFilterButtonChecked, "CheckButtonChecked", panel);

    CBaseHudChat *GetChatParent() { return dynamic_cast<CBaseHudChat *>(GetParent()); }

    void SetVisible(bool state) override;
};