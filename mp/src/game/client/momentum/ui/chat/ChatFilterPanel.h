#pragma once

#include "vgui_controls/CheckButton.h"
#include "vgui_controls/EditablePanel.h"

class ChatPanel;

enum ChatFilters
{
    CHAT_FILTER_NONE = 0,
    CHAT_FILTER_JOINLEAVE = 0x000001,
    CHAT_FILTER_NAMECHANGE = 0x000002,
    CHAT_FILTER_PUBLICCHAT = 0x000004,
    CHAT_FILTER_SERVERMSG = 0x000008,
    CHAT_FILTER_SPEC_SPAWN = 0x000010,
    CHAT_FILTER_ACHIEVEMENT = 0x000020,
};

class ChatFilterCheckButton : public vgui::CheckButton
{
public:
    DECLARE_CLASS_SIMPLE(ChatFilterCheckButton, vgui::CheckButton);

    ChatFilterCheckButton(Panel *pParent, const char *pName, int iFlag);

    int GetFilterFlag() { return m_iFlag; }

private:
    int m_iFlag;
};

class ChatFilterPanel : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(ChatFilterPanel, vgui::EditablePanel);

    ChatFilterPanel(ChatPanel *pParent);

    MESSAGE_FUNC_PTR(OnFilterButtonChecked, "CheckButtonChecked", panel);

    void SetVisible(bool state) override;

private:
    ChatPanel *m_pChatParent;
};