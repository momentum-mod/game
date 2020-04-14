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
    g_pMessageBox->CreateMessagebox("#MOM_MB_TrySaveLoad_Title", "#MOM_MB_TrySaveLoad");
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

CMessageboxInterface::CMessageboxInterface()
{
    HOOK_MESSAGE(MB_PlayerTriedSaveOrLoad);
}

Panel *CMessageboxInterface::CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept /*= nullptr*/)
{
    const auto pMessageBox = new MessageBox(pTitle, pMessage);

    if (pAccept && Q_strlen(pAccept) > 0)
    {
        pMessageBox->SetOKButtonText(pAccept);
    }

    pMessageBox->MoveToCenterOfScreen();
    pMessageBox->DoModal();

    m_mbItems.AddToTail(pMessageBox);

    return pMessageBox;
}

Panel *CMessageboxInterface::CreateConfirmationBox(Panel *pTarget, const char *pTitle, const char *pMessage,
                                                    KeyValues *okCommand, KeyValues *cancelCommand, const char *pAcceptText /*= nullptr*/,
                                                    const char *pCancelText /* = nullptr*/)
{
    const auto pMessageBox = new MessageBox(pTitle, pMessage);
    if (pTarget)
    {
        pMessageBox->AddActionSignalTarget(pTarget);

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
    pMessageBox->DoModal();
    pMessageBox->MoveToFront();

    m_mbItems.AddToTail(pMessageBox);

    return pMessageBox;
}

Panel *CMessageboxInterface::CreateMessageboxVarRef(const char *pTitle, const char *pMessage, const char *cvarName, const char *pAccept /*= nullptr*/)
{
    ConVarRef varref(cvarName);

    if (!varref.IsValid())
        return nullptr;

    if (varref.GetBool())
        return nullptr;

    const auto pMessageBox = new MessageBoxVarRef(pTitle, pMessage, cvarName);

    if (pAccept && Q_strlen(pAccept) > 0)
    {
        pMessageBox->SetOKButtonText(pAccept);
    }

    pMessageBox->MoveToCenterOfScreen();
    pMessageBox->DoModal();

    m_mbItems.AddToTail(pMessageBox);

    return pMessageBox;
}

void CMessageboxInterface::DiscardMessageboxes()
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

// Interface this class to the rest of the DLL
static CMessageboxInterface g_Messagebox;
CMessageboxInterface *g_pMessageBox = &g_Messagebox;