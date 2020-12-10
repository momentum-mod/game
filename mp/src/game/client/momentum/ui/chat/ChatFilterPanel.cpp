#include "cbase.h"

#include "ChatFilterPanel.h"

#include "ChatPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

ConVar cl_chatfilters("cl_chatfilters", "63", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Stores the chat filter settings ");

ChatFilterCheckButton::ChatFilterCheckButton(Panel *pParent, const char *pName, int iFlag) : BaseClass(pParent, pName, "")
{
    m_iFlag = iFlag;
}

ChatFilterPanel::ChatFilterPanel(ChatPanel *pParent) : BaseClass(pParent, "ChatFilterPanel")
{
    m_pChatParent = pParent;
    pParent->SetSize(10, 10); // Quiet "parent not sized yet" spew

    new ChatFilterCheckButton(this, "joinleave_button", CHAT_FILTER_JOINLEAVE);
    new ChatFilterCheckButton(this, "namechange_button", CHAT_FILTER_NAMECHANGE);
    new ChatFilterCheckButton(this, "publicchat_button", CHAT_FILTER_PUBLICCHAT);
    new ChatFilterCheckButton(this, "servermsg_button", CHAT_FILTER_SERVERMSG);
    new ChatFilterCheckButton(this, "specspawn_button", CHAT_FILTER_SPEC_SPAWN);
    new ChatFilterCheckButton(this, "achivement_button", CHAT_FILTER_ACHIEVEMENT);

    LoadControlSettings("resource/ui/ChatFilters.res");

    InvalidateLayout(true, true);
    SetMouseInputEnabled(true);
    SetPaintBorderEnabled(true);
    SetVisible(false);
}

void ChatFilterPanel::OnFilterButtonChecked(Panel *panel)
{
    const auto pButton = dynamic_cast<ChatFilterCheckButton *>(panel);

    if (pButton && m_pChatParent && IsVisible())
    {
        if (pButton->IsSelected())
        {
            m_pChatParent->SetFilterFlag(m_pChatParent->GetFilterFlags() | pButton->GetFilterFlag());
        }
        else
        {
            m_pChatParent->SetFilterFlag(m_pChatParent->GetFilterFlags() & ~pButton->GetFilterFlag());
        }
    }
}

void ChatFilterPanel::SetVisible(bool state)
{
    if (state == true)
    {
        for (int i = 0; i < GetChildCount(); i++)
        {
            const auto pButton = dynamic_cast<ChatFilterCheckButton *>(GetChild(i));

            if (pButton)
            {
                if (cl_chatfilters.GetInt() & pButton->GetFilterFlag())
                {
                    pButton->SetSelected(true);
                }
                else
                {
                    pButton->SetSelected(false);
                }
            }
        }
    }

    BaseClass::SetVisible(state);
}