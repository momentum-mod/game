// The following include files are necessary to allow The Panel .cpp to compile.
#include "cbase.h"

#include "MessageboxPanel.h"
#include "mom_shareddefs.h"
#include <vgui_controls/CvarToggleCheckButton.h>
#include "hud_macros.h"

static MAKE_TOGGLE_CONVAR(mom_toggle_nostartorend, "0", FCVAR_HIDDEN | FCVAR_ARCHIVE, "Controls if No Start or End should be shown.\n");
static MAKE_TOGGLE_CONVAR(mom_toggle_versionwarn, "0", FCVAR_HIDDEN | FCVAR_ARCHIVE, "Controls if the initial version warning should be shown.\n");

using namespace vgui;

void __MsgFunc_MB_PlayerTriedSaveOrLoad(bf_read &msg)
{
    messageboxpanel->CreateMessagebox("#MOM_MB_TrySaveLoad_Title", "#MOM_MB_TrySaveLoad");
}
void __MsgFunc_MB_NoStartOrEnd(bf_read &msg)
{
    messageboxpanel->CreateMessageboxVarRef("#MOM_MB_NoStartOrEnd_Title", "#MOM_MB_NoStartOrEnd", "mom_toggle_nostartorend");
}

void __MsgFunc_MB_EditingZone(bf_read &msg)
{
    messageboxpanel->CreateMessagebox("#MOM_MB_EditingZone_Title", "#MOM_MB_EditingZone");
}

MessageBoxVarRef::MessageBoxVarRef(const char* title, const char* msg, const char* cvar) : MessageBox(title, msg)
{
    // When toggled, will not allow the panel to be created (We don't check it here because we've done it on our 2 interfaces (Messaging and IMEssageBox)
    // this also allows us to show this even if the toggle says no! (Like, for important stuff)
    m_pToggleCheckButton = new vgui::CvarToggleCheckButton(this, "MessageboxVarRef", "#MOM_MB_DontShowAgain", cvar);
    AddActionSignalTarget(m_pToggleCheckButton); // Catch that OK button press
}

MessageBoxVarRef::~MessageBoxVarRef()
{
    if (m_pToggleCheckButton)
    {
        m_pToggleCheckButton->DeletePanel();
        m_pToggleCheckButton = nullptr;
    }
}

// Overridden 
void MessageBoxVarRef::PerformLayout()
{
    int x, y, wide, tall;
    GetClientArea(x, y, wide, tall);
    wide += x;
    tall += y;

    int boxWidth, boxTall;
    GetSize(boxWidth, boxTall);

    int oldWide, oldTall;
    m_pOkButton->GetSize(oldWide, oldTall);

    int btnWide, btnTall;
    m_pOkButton->GetContentSize(btnWide, btnTall);
    btnWide = max(oldWide, btnWide + 10);
    btnTall = max(oldTall, btnTall + 10);
    m_pOkButton->SetSize(btnWide, btnTall);

    boxWidth = max(boxWidth, m_pMessageLabel->GetWide() + 100);
    boxWidth = max(boxWidth, btnWide * 2 + 30);
    SetSize(boxWidth, boxTall);

    m_pMessageLabel->SetPos((wide / 2) - (m_pMessageLabel->GetWide() / 2) + x, y + 5);
    m_pOkButton->SetPos((wide / 2) - (m_pOkButton->GetWide() / 2) + x, tall - m_pOkButton->GetTall() - 25);

    if (m_pToggleCheckButton)
    {
        int dummy, okY;
        m_pOkButton->GetPos(dummy, okY);
        m_pToggleCheckButton->SetAutoWide(true);
        m_pToggleCheckButton->SetPos(x, tall - m_pToggleCheckButton->GetTall());
    }

    // Bypass BaseClass and call its BaseClass
    Frame::PerformLayout();
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
    HOOK_MESSAGE(MB_EditingZone);
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
    return pMessageBox;
}

Panel* CMessageboxPanel::CreateMessageboxVarRef(const char* pTitle, const char* pMessage, const char* cvar, const char* pAccept)
{
    ConVarRef varref(cvar);
    if (!varref.IsValid())
        return nullptr;
    // Preliminary check, if the var is already 1 then bail
    if (varref.GetBool())
        return nullptr;
    MessageBoxVarRef *pMessageBox = new MessageBoxVarRef(pTitle, pMessage, cvar);
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