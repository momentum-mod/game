#include "cbase.h"

#include "ChatHistory.h"

#include "tier0/memdbgon.h"

using namespace vgui;

ChatHistory::ChatHistory(Panel *pParent) : BaseClass(pParent, "ChatHistory")
{
    InsertFade(-1, -1);
}

void ChatHistory::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetAlpha(255);
}