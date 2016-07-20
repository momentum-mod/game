// The following include files are necessary to allow The Panel .cpp to compile.
#include "cbase.h"

#include "MessageboxPanel.h"


void __MsgFunc_MB_PlayerTriedSaveOrLoad(bf_read &msg)
{
    messageboxpanel->CreateMessagebox("#MOM_MB_TrySaveLoad_Title", "#MOM_MB_TrySaveLoad");
}
void __MsgFunc_MB_NoStartOrEnd(bf_read &msg)
{
    messageboxpanel->CreateMessagebox("#MOM_MB_NoStartOrEnd_Title", "#MOM_MB_NoStartOrEnd");
}

// Constuctor: Initializes the Panel
CMessageboxPanel::CMessageboxPanel(VPANEL parent) : BaseClass(nullptr, "MessageboxPanel")
{
    SetParent(parent);

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMinimumSize(0, 0);
    SetSize(0, 0);
    MoveToCenterOfScreen();
    SetMoveable(true);
    SetVisible(false);
    SetProportional(true);

    HOOK_MESSAGE(MB_PlayerTriedSaveOrLoad);
    HOOK_MESSAGE(MB_NoStartOrEnd);
}

CMessageboxPanel::~CMessageboxPanel() { FlushMessageboxes(); }

void CMessageboxPanel::Close() { FlushMessageboxes(); }

HPanel CMessageboxPanel::CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept)
{
    MessageBox *pMessageBox = new MessageBox(pTitle, pMessage);
    // If it is not a nullptr and it's not an empty string...
    if (pAccept && Q_strlen(pAccept) > 0)
    {
        pMessageBox->SetOKButtonText(pAccept);
    }
    m_mbItems.AddToTail(pMessageBox);
    pMessageBox->ShowWindow();
    return pMessageBox->ToHandle();
}

void CMessageboxPanel::FlushMessageboxes()
{
    FOR_EACH_VEC(m_mbItems, iIterator)
    {
        MessageBox *pItem = m_mbItems[iIterator];
        if (pItem)
        {
            pItem->Close();
            pItem->DeletePanel();
        }
    }
    m_mbItems.RemoveAll();
}

void CMessageboxPanel::FlushMessageboxes(HPanel pHPanel)
{
    FOR_EACH_VEC(m_mbItems, iIterator)
    {
        MessageBox *pItem = m_mbItems[iIterator];
        if (pItem && pItem->ToHandle() == pHPanel)
        {
            m_mbItems.FastRemove(iIterator);
            pItem->Close();
            pItem->DeletePanel();
            break;
        }
    }
}

// Interface this class to the rest of the DLL
static CMessageboxInterface g_Messagebox;
IMessageboxPanel *messageboxpanel = static_cast<IMessageboxPanel *>(&g_Messagebox);