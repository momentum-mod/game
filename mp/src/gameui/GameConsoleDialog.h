//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//

#ifndef GAMECONSOLEDIALOG_H
#define GAMECONSOLEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/consoledialog.h"

//-----------------------------------------------------------------------------
// Purpose: Game/dev console dialog
//-----------------------------------------------------------------------------
class CGameConsoleDialog : public vgui::CConsoleDialog
{
    DECLARE_CLASS_SIMPLE(CGameConsoleDialog, vgui::CConsoleDialog);

    CGameConsoleDialog();

protected:
    MESSAGE_FUNC(OnClosedByHittingTilde, "ClosedByHittingTilde");
    MESSAGE_FUNC_CHARPTR(OnCommandSubmitted, "CommandSubmitted", command);

    void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    void OnCommand(const char *command) OVERRIDE;
};

#endif // GAMECONSOLEDIALOG_H