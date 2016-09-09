#pragma once

#include "cbase.h"

#include "IMessageboxPanel.h"
#include "hud_macros.h"
#include "usermessages.h"
#include <vgui/ILocalize.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/MessageBox.h>

#include "momentum/mom_shareddefs.h"

using namespace vgui;

// CChangelogPanel class
class CMessageboxPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CMessageboxPanel, Frame);
    // CChangelogPanel : This Class / vgui::Frame : BaseClass

    CMessageboxPanel(VPANEL parent); // Constructor
    ~CMessageboxPanel();             // Destructor

    void Close() override;

    // Creates a messagebox, with pTitle as title and pMessage as message.
    // It does not disappear until Close is pressed or FlushMessageboxes() is called
    // Returns the pointer of the newly created panel
    Panel *CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr);

    Panel *CreateConfirmationBox(Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
        KeyValues *cancelCommand, const char *pAcceptText = nullptr, const char *pCancelText = nullptr);
    // This function deletes all the messageboxes
    void FlushMessageboxes();
    // Removes the HPanel messagebox
    void FlushMessageboxes(HPanel pHPanel);

  private:
    CUtlVector<MessageBox *> m_mbItems;
};

class CMessageboxInterface : public IMessageboxPanel
{
  private:
    CMessageboxPanel *pPanel;

  public:
    CMessageboxInterface() { pPanel = nullptr; }
    ~CMessageboxInterface() {}
    void Create(VPANEL parent) override { pPanel = new CMessageboxPanel(parent); }

    void Destroy() override
    {
        if (pPanel)
        {
            pPanel->DeletePanel();
        }
        pPanel = nullptr;
    }

    void Activate(void) override
    {
        if (pPanel)
        {
            pPanel->Activate();
        }
    }

    void Close() override
    {
        if (pPanel)
        {
            pPanel->Close();
        }
    }

    // is the default parameter specifier needed here?
    Panel *CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr) override
    {
        if (pPanel)
        {
            return pPanel->CreateMessagebox(pTitle, pMessage, pAccept);
        }
        return nullptr;
    }

    Panel *CreateConfirmationBox(Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
        KeyValues *cancelCommand, const char *pAcceptText = nullptr, const char *pCancelText = nullptr) override
    {
        if (pPanel)
        {
            return pPanel->CreateConfirmationBox(pTarget, pTitle, pMessage, okCommand, cancelCommand, pAcceptText, pCancelText);
        }
        return nullptr;
    }

    void FlushMessageboxes() override
    {
        if (pPanel)
        {
            pPanel->FlushMessageboxes();
        }
    }

    void FlushMessageboxes(HPanel pHp) override
    {
        if (pPanel)
        {
            pPanel->FlushMessageboxes(pHp);
        }
    }
};