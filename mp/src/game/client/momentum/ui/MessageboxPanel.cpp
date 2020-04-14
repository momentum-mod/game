#include "cbase.h"

#include "MessageboxPanel.h"

#include "mom_shareddefs.h"
#include <vgui_controls/CvarToggleCheckButton.h>
#include "hud_macros.h"

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_toggle_versionwarn, "0", FCVAR_HIDDEN | FCVAR_ARCHIVE, "Controls if the initial version warning should be shown.\n");

using namespace vgui;

void __MsgFunc_MB_PlayerTriedSaveOrLoad(bf_read &msg)
{
    messageboxpanel->CreateMessagebox("#MOM_MB_TrySaveLoad_Title", "#MOM_MB_TrySaveLoad");
}

MessageBoxVarRef::MessageBoxVarRef(const char* title, const char* msg, const char* cvarName) : MessageBox(title, msg)
{
    m_pToggleCheckButton = new CvarToggleCheckButton(this, "MessageboxVarRef", "#MOM_MB_DontShowAgain", cvarName);
    AddActionSignalTarget(m_pToggleCheckButton); // Catch that OK button press
    m_pToggleCheckButton->SetAutoWide(true);
    m_pToggleCheckButton->SetAutoTall(true);
    m_pToggleCheckButton->SetCheckInset(0);

    // Needed for saving the variable
    SetCommand(new KeyValues("ApplyChanges"));
}

MessageBoxVarRef::~MessageBoxVarRef()
{
}

void MessageBoxVarRef::PerformLayout()
{
    BaseClass::PerformLayout();

    int x = m_pMessageLabel->GetXPos();
    int y = m_pMessageLabel->GetYPos() + m_pMessageLabel->GetTall() + GetScaledVal(16);

    m_pToggleCheckButton->SetPos(x, y);

    m_pOkButton->SetPos((m_pMessageLabel->GetXPos() + m_pMessageLabel->GetWide()) - m_pOkButton->GetWide(), y);
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
}

CMessageboxPanel::~CMessageboxPanel() { }

void CMessageboxPanel::Close() { FlushMessageboxes(); }

Panel *CMessageboxPanel::CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept)
{
    MessageBox *pMessageBox = new MessageBox(pTitle, pMessage);
    // If it is not a nullptr and it's not an empty string...
    if (pAccept && Q_strlen(pAccept) > 0)
    {
        pMessageBox->SetOKButtonText(pAccept);
    }
    pMessageBox->MoveToCenterOfScreen();
    m_mbItems.AddToTail(pMessageBox);
    pMessageBox->DoModal();
    return pMessageBox;
}

Panel *CMessageboxPanel::CreateConfirmationBox(Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
    KeyValues *cancelCommand, const char *pAcceptText, const char *pCancelText)
{
    MessageBox *pMessageBox = new MessageBox(pTitle, pMessage);
    if (pTarget)
    {
        pMessageBox->AddActionSignalTarget(pTarget);
        // This does not make sense if the target is nullptr so..
        if (okCommand)
        {
            pMessageBox->SetCommand(okCommand);
        }
        if (cancelCommand)
        {
            pMessageBox->SetCancelCommand(cancelCommand);
        }
    }

    pMessageBox->SetOKButtonVisible(true);
    if (pAcceptText && Q_strlen(pAcceptText) > 0)
    {
        pMessageBox->SetOKButtonText(pAcceptText);
    }

    pMessageBox->SetCancelButtonVisible(true);
    if (pCancelText && Q_strlen(pCancelText) > 0)
    {
        pMessageBox->SetCancelButtonText(pCancelText);
    }
    pMessageBox->MoveToCenterOfScreen();

    m_mbItems.AddToTail(pMessageBox);
    pMessageBox->DoModal();
    pMessageBox->MoveToFront();
    return pMessageBox;
}

Panel* CMessageboxPanel::CreateMessageboxVarRef(const char* pTitle, const char* pMessage, const char* cvarName, const char* pAccept)
{
    ConVarRef varref(cvarName);
    if (!varref.IsValid())
        return nullptr;
    // Preliminary check, if the var is already 1 then bail
    if (varref.GetBool())
        return nullptr;
    MessageBoxVarRef *pMessageBox = new MessageBoxVarRef(pTitle, pMessage, cvarName);
    // If it is not a nullptr and it's not an empty string...
    if (pAccept && Q_strlen(pAccept) > 0)
    {
        pMessageBox->SetOKButtonText(pAccept);
    }
    // Needed for saving the ConVarRef
    pMessageBox->SetCommand(new KeyValues("ApplyChanges"));
    pMessageBox->MoveToCenterOfScreen();

    m_mbItems.AddToTail(pMessageBox);
    pMessageBox->DoModal();
    return pMessageBox;
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