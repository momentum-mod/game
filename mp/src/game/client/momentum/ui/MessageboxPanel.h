#pragma once

#include "IMessageboxPanel.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/MessageBox.h>

class MessageBoxVarRef : public vgui::MessageBox
{
public:
    MessageBoxVarRef(const char *title, const char *msg, const char *cvar);
    ~MessageBoxVarRef();

    void PerformLayout() OVERRIDE;
private:
    vgui::CvarToggleCheckButton* m_pToggleCheckButton;
};

// CChangelogPanel class
class CMessageboxPanel : public vgui::Frame  // We're not a child of MessageBox for a good reason. I guess...
{
    DECLARE_CLASS_SIMPLE(CMessageboxPanel, Frame);
    // CChangelogPanel : This Class / vgui::Frame : BaseClass

    CMessageboxPanel(vgui::VPANEL parent); // Constructor
    ~CMessageboxPanel();             // Destructor

    void Close() OVERRIDE;

    // Creates a messagebox, with pTitle as title and pMessage as message.
    // It does not disappear until Close is pressed or FlushMessageboxes() is called
    // Returns the pointer of the newly created panel
    Panel *CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr);

    Panel *CreateConfirmationBox(Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
        KeyValues *cancelCommand, const char *pAcceptText = nullptr, const char *pCancelText = nullptr);

    // Creates a messagebox with a "Don't show me this again" toggle controlled by a convar (Defined in its params)
    // TIP: ConVar should be defined as FCVAR_HIDDEN | FCVAR_ARCHIVE with default to 0.
    Panel *CreateMessageboxVarRef(const char *pTitle, const char *pMessage, const char* cvar, const char *pAccept = nullptr);
    // This function deletes all the messageboxes
    void FlushMessageboxes();
    // Removes the HPanel messagebox
    void FlushMessageboxes(vgui::HPanel pHPanel);

  private:
    CUtlVector<vgui::MessageBox *> m_mbItems;
};

class CMessageboxInterface : public IMessageboxPanel
{
  private:
    CMessageboxPanel *pPanel;

  public:
    CMessageboxInterface() { pPanel = nullptr; }
    virtual ~CMessageboxInterface() {}
    void Create(vgui::VPANEL parent) OVERRIDE { pPanel = new CMessageboxPanel(parent); }

    void Destroy() OVERRIDE
    {
        if (pPanel)
        {
            pPanel->DeletePanel();
        }
        pPanel = nullptr;
    }

    void Activate(void) OVERRIDE
    {
        if (pPanel)
        {
            pPanel->Activate();
        }
    }

    void Close() OVERRIDE
    {
        if (pPanel)
        {
            pPanel->Close();
        }
    }

    // is the default parameter specifier needed here?
    vgui::Panel *CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr) OVERRIDE
    {
        if (pPanel)
        {
            return pPanel->CreateMessagebox(pTitle, pMessage, pAccept);
        }
        return nullptr;
    }

    vgui::Panel *CreateConfirmationBox(vgui::Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
        KeyValues *cancelCommand, const char *pAcceptText = nullptr, const char *pCancelText = nullptr) OVERRIDE
    {
        if (pPanel)
        {
            return pPanel->CreateConfirmationBox(pTarget, pTitle, pMessage, okCommand, cancelCommand, pAcceptText, pCancelText);
        }
        return nullptr;
    }

    vgui::Panel *CreateMessageboxVarRef(const char *pTitle, const char *pMessage, const char *cvarName, const char *pAccept = nullptr) OVERRIDE
    {
        if (pPanel)
        {
            return pPanel->CreateMessageboxVarRef(pTitle, pMessage, cvarName, pAccept);
        }
        return nullptr;
    }

    void FlushMessageboxes() OVERRIDE
    {
        if (pPanel)
        {
            pPanel->FlushMessageboxes();
        }
    }

    void FlushMessageboxes(vgui::HPanel pHp) OVERRIDE
    {
        if (pPanel)
        {
            pPanel->FlushMessageboxes(pHp);
        }
    }
};