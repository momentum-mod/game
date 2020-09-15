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
    ChatPanel *m_pChatParent;
};