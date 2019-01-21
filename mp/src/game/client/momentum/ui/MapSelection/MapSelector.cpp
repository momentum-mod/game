#include "cbase.h"

#include "MapSelector.h"
#include "MapSelectorDialog.h"

#include "tier0/memdbgon.h"

static CMapSelector g_MapSelectorPanel;
IMapSelector* mapselector = static_cast<CMapSelector*>(&g_MapSelectorPanel);

//
CMapSelector::CMapSelector()
{
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapSelector::~CMapSelector()
{
}

CON_COMMAND_F(ShowMapSelectionPanel, "Shows MapSelectorPanel", FCVAR_CLIENTDLL | FCVAR_HIDDEN)
{
    mapselector->Activate();
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapSelector::Create(vgui::VPANEL parent)
{
    if (!m_hMapsDlg.Get())
    {
        m_hMapsDlg = new CMapSelectorDialog(parent); // SetParent() call below fills this in
        m_hMapsDlg->Initialize();
    }
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapSelector::Activate()
{
    static bool m_bfirstTimeOpening = true;
    if (m_bfirstTimeOpening)
    {
        m_hMapsDlg->LoadUserData(); // reload the user data the first time the dialog is made visible, 
        //helps with the lag between module load and steamui getting Deactivate() call
        m_bfirstTimeOpening = false;
    }

    Open();
}

//-----------------------------------------------------------------------------
// Purpose: called when the server browser gets closed by the enduser
//-----------------------------------------------------------------------------
void CMapSelector::Deactivate()
{
    if (m_hMapsDlg.Get())
    {
        m_hMapsDlg->SaveUserData();
        m_hMapsDlg->Close();
        CloseAllMapInfoDialogs();
    }
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapSelector::Open()
{
    m_hMapsDlg->Open();
}


//-----------------------------------------------------------------------------
// Purpose: Closes down the server browser for good
//-----------------------------------------------------------------------------
void CMapSelector::Destroy()
{
    if (m_hMapsDlg.Get())
    {
        m_hMapsDlg->Close();
        m_hMapsDlg->MarkForDeletion();
        m_hMapsDlg = nullptr;
    }
}


//-----------------------------------------------------------------------------
// Purpose: closes all the game info dialogs
//-----------------------------------------------------------------------------
void CMapSelector::CloseAllMapInfoDialogs()
{
    if (m_hMapsDlg.Get())
    {
        m_hMapsDlg->CloseAllMapInfoDialogs();
    }
}