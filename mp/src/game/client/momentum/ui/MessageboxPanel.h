#pragma once

#include <vgui_controls/MessageBox.h>

class MessageBoxVarRef : public vgui::MessageBox
{
public:
    DECLARE_CLASS_SIMPLE(MessageBoxVarRef, vgui::MessageBox);

    MessageBoxVarRef(const char *title, const char *msg, const char *cvar);
    ~MessageBoxVarRef();

    void PerformLayout() OVERRIDE;
private:
    vgui::CvarToggleCheckButton* m_pToggleCheckButton;
};

class CMessageboxInterface
{
public:
    CMessageboxInterface();

    // Creates a messagebox, with pTitle as title and pMessage as message.
    // It does not disappear until Close is pressed or DiscardMessageboxes() is called
    // Returns the pointer of the newly created panel
    vgui::Panel *CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr);

    // Creates a messagebox with a confirmation prompt, okCommand happens when user accepts, cancelCommand happens when user refuses
    vgui::Panel *CreateConfirmationBox(vgui::Panel *pTarget, const char *pTitle, const char *pMessage, KeyValues *okCommand,
                                       KeyValues *cancelCommand, const char *pAcceptText = nullptr, const char *pCancelText = nullptr);

    // Creates a messagebox with a "Don't show me this again" toggle controlled by a convar (Defined in its params)
    // TIP: ConVar should be defined as FCVAR_HIDDEN | FCVAR_ARCHIVE with default to 0.
    vgui::Panel *CreateMessageboxVarRef(const char *pTitle, const char *pMessage, const char *cvarName, const char *pAccept = nullptr);


    void DiscardMessageboxes();

private:
    CUtlVector<vgui::DHANDLE<vgui::MessageBox>> m_mbItems;
};

extern CMessageboxInterface *g_pMessageBox;