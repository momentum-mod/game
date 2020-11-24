#pragma once

#include "vgui_controls/TextEntry.h"

class ChatPanel;

class ChatEntry : public vgui::TextEntry
{
public:
    DECLARE_CLASS_SIMPLE(ChatEntry, TextEntry);

    ChatEntry(ChatPanel *parent);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnKeyCodeTyped(vgui::KeyCode code) override;

    CPanelAnimationStringVar(32, m_FontName, "font", "Default");
    CPanelAnimationVar(Color, m_cTypingColor, "TypingText", "White");
private:
    void AddInputToHistory();
    // Pressing "up" should go to previously sent message (end of history list and then up, decreasing the index to 0)
    // Pressing "down" should go to first sent message, going oldest -> newest in history
    void TraverseHistory(bool bUpKeyPressed);

    QueueIter_t m_iHistoryIndex; // Currently at this point in history, partial backspaces will still allow traversal
    CUtlQueue<CUtlString> m_queueChatHistory;
    ChatPanel *m_pChatParent;
};