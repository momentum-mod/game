#include "cbase.h"

#include "ChatEntry.h"
#include "ChatPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

ChatEntry::ChatEntry(ChatPanel *parent) : BaseClass(parent, "ChatEntry")
{
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
    if (code == KEY_ENTER || code == KEY_PAD_ENTER || code == KEY_ESCAPE)
    {
        if (code != KEY_ESCAPE)
        {
            PostMessage(GetVParent(), new KeyValues("ChatEntrySend"));
        }

        if (m_pChatParent->GetMessageMode() == MESSAGE_MODE_MENU)
        {
            BaseClass::OnKeyCodeTyped(code);
        }
        else
        {
            PostMessage(GetVParent(), new KeyValues("ChatEntryStopMessageMode"));
        }
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