#include "cbase.h"

#include "hud_chat.h"

#include "hud_element_helper.h"
#include "iclientmode.h"
#include "chat/ChatPanel.h"
#include "run/mom_run_safeguards.h"

#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudChat);

using namespace vgui;

static CHudChat *g_pHudChat = nullptr;

CON_COMMAND(chat_open, "Opens the HUD chat window.\n")
{
    if (!g_pHudChat)
        return;

    g_pHudChat->StartMessageMode();
}

CHudChat::CHudChat(const char *pElementName) : CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "HudChat")
{
    g_pHudChat = this;

    SetHiddenBits(HIDEHUD_CHAT);

    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);

    m_pChatContainer = new ChatContainer(this);
    m_pChatContainer->SetAutomaticMessageMode(MESSAGE_MODE_HUD);
    m_pChatContainer->SetPaintBackgroundEnabled(false);
}

void CHudChat::StartMessageMode()
{
    // avoid softlock of starting message mode when hud/viewport isn't visible
    if (!g_pClientMode->GetViewport()->IsVisible())
        return;

    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_CHAT_OPEN))
        return;

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);

    m_pChatContainer->StartMessageMode(MESSAGE_MODE_HUD);
}

void CHudChat::PerformLayout()
{
    BaseClass::PerformLayout();

    int wide, tall;
    GetSize(wide, tall);

    m_pChatContainer->SetSize(wide, tall);
}

void CHudChat::OnStopMessageMode()
{
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}