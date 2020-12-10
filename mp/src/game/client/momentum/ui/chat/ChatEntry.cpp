#include "cbase.h"

#include "ChatEntry.h"
#include "ChatPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define MAX_CHAT_HISTORY_ITEMS 20

ChatEntry::ChatEntry(ChatPanel *parent) : BaseClass(parent, "ChatEntry")
{
    m_iHistoryIndex = QUEUE_ITERATOR_INVALID;
    m_pChatParent = parent;
    SetCatchEnterKey(true);
    SetAllowNonAsciiCharacters(true);
    SetDrawLanguageIDAtLeft(true);
    SetMaximumCharCount(MAX_CHAT_LENGTH);
}

void ChatEntry::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetFont(GetSchemeFont(pScheme, m_FontName, "Chat.Font"));

    SetFgColor(m_cTypingColor);

    SetMouseInputEnabled(true);
    SetPaintBorderEnabled(false);
    SetMaximumCharCount(MAX_CHAT_LENGTH);
}

void ChatEntry::OnKeyCodeTyped(KeyCode code)
{
    if (GetTextLength() == 0)
        m_iHistoryIndex = QUEUE_ITERATOR_INVALID;

    if (code == KEY_ENTER || code == KEY_PAD_ENTER || (code == KEY_ESCAPE && m_pChatParent->GetMessageMode() != MESSAGE_MODE_MENU))
    {
        if (code != KEY_ESCAPE)
        {
            PostMessage(GetVParent(), new KeyValues("ChatEntrySend"));
            AddInputToHistory();
        }

        if (m_pChatParent->GetMessageMode() != MESSAGE_MODE_MENU)
        {
            PostMessage(GetVParent(), new KeyValues("ChatEntryStopMessageMode"));
        }

        m_iHistoryIndex = QUEUE_ITERATOR_INVALID;
    }
    else if (code == KEY_UP)
    {
        TraverseHistory(true);
    }
    else if (code == KEY_DOWN)
    {
        TraverseHistory(false);
    }
    else if (code == KEY_TAB)
    {
        // Ignore tab, otherwise vgui will screw up the focus.
        return;
    }
    else
    {
        BaseClass::OnKeyCodeTyped(code);
    }
}

void ChatEntry::AddInputToHistory()
{
    CUtlString str;
    str.SetLength(MAX_CHAT_LENGTH);
    GetText(str.GetForModify(), MAX_CHAT_LENGTH);

    if (str.IsEmpty())
        return;

    if (m_queueChatHistory.Check(str))
        return;

    if (m_queueChatHistory.Count() >= MAX_CHAT_HISTORY_ITEMS)
        m_queueChatHistory.RemoveAtHead();

    m_queueChatHistory.Insert(str);
}

void ChatEntry::TraverseHistory(bool bUpKeyPressed)
{
    if (m_iHistoryIndex == QUEUE_ITERATOR_INVALID)
    {
        m_iHistoryIndex = bUpKeyPressed ? m_queueChatHistory.Last() : m_queueChatHistory.First();
    }
    else
    {
        m_iHistoryIndex = bUpKeyPressed ? m_queueChatHistory.Previous(m_iHistoryIndex) : m_queueChatHistory.Next(m_iHistoryIndex);

        // Loop back around
        if (m_iHistoryIndex == QUEUE_ITERATOR_INVALID)
            m_iHistoryIndex = bUpKeyPressed ? m_queueChatHistory.Last() : m_queueChatHistory.First();
    }

    if (m_iHistoryIndex != QUEUE_ITERATOR_INVALID)
    {
        SetText(m_queueChatHistory.Element(m_iHistoryIndex));
        GotoTextEnd();
    }
}