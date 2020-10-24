#pragma once

#include "vgui_controls/RichText.h"

#define CHAT_HISTORY_IDLE_TIME 15.0f
#define CHAT_HISTORY_IDLE_FADE_TIME 2.5f

class ChatHistory : public vgui::RichText
{
public:
    DECLARE_CLASS_SIMPLE(ChatHistory, vgui::RichText);

    ChatHistory(Panel *pParent);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
};