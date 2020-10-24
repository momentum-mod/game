#include "GameConsoleDialog.h"
#include "GameUI_Interface.h"
#include "IGameUIFuncs.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "vgui/KeyCode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "cdll_int.h"
#include "tier0/memdbgon.h"

using namespace vgui;

extern IVEngineClient *engine;
extern IGameUIFuncs *gameuifuncs;

CGameConsoleDialog::CGameConsoleDialog() : BaseClass(nullptr, "GameConsole") { }

//-----------------------------------------------------------------------------
// HACK: Allow F key bindings to operate even when typing in the text entry field
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnKeyCodeTyped(KeyCode code)
{
    BaseClass::OnKeyCodeTyped(code);

    // check for processing
    if (m_pConsolePanel->TextEntryHasFocus())
    {
        // HACK: Allow F key bindings to operate even here
        if (code >= KEY_F1 && code <= KEY_F12)
        {
            // See if there is a binding for the FKey
            const char *binding = gameuifuncs->GetBindingForButtonCode(code);
            if (binding && binding[0])
            {
                // submit the entry as a console command
                char szCommand[256];
                Q_strncpy(szCommand, binding, sizeof(szCommand));
                engine->ClientCmd_Unrestricted(szCommand);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Submits a command
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnCommandSubmitted(const char *pCommand) { engine->ClientCmd_Unrestricted(pCommand); }

void CGameConsoleDialog::OnClosedByHittingTilde()
{
    if (!GameUI().IsInLoading())
    {
        Hide();
        GameUI().HideGameUI();
    }
}