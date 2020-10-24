#pragma once

#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"

class ChatContainer;

class CHudChat : public CHudElement, public vgui::EditablePanel
{
  public:
    DECLARE_CLASS_SIMPLE(CHudChat, EditablePanel);

    CHudChat(const char *pElementName);

    void StartMessageMode();

    void Reset() override;
    void PerformLayout() override;

    MESSAGE_FUNC(OnStopMessageMode, "StopMessageMode");

private:
    ChatContainer *m_pChatContainer;
};