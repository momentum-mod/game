#include "cbase.h"

#include "IMenuOverride.h"
#include "mom_menu_rootpanel.h"

#include "filesystem.h"
#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "vgui_controls/pch_vgui_controls.h"
#include <engine/IEngineSound.h>

using namespace vgui;

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we
// don't call Sys_LoadModule over and over again.
static CDllDemandLoader g_GameUIDLL("GameUI");

// Expose this interface to the DLL
static COverrideInterface g_MOMMenu;
IMenuOverride *MOMMenuOverride = static_cast<IMenuOverride *>(&g_MOMMenu);

CMOMMenuOverride::CMOMMenuOverride(VPANEL parent) : EditablePanel(nullptr, "CMOMMenuOverride")
{
    SetParent(parent);

    SetProportional(true);
    LoadControlSettings("resource/ui/MainMenu.res");
    

    
    m_bCopyFrameBuffer = false;
    m_pGameUI = nullptr;
    
    LoadGameUI();

    m_ExitingFrameCount = 0;

}

IGameUI *CMOMMenuOverride::GetGameUI()
{
    if (!m_pGameUI)
    {
        if (!LoadGameUI())
            return nullptr;
    }

    return m_pGameUI;
}

bool CMOMMenuOverride::LoadGameUI()
{
    if (!m_pGameUI)
    {
        CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
        if (gameUIFactory)
        {
            m_pGameUI = static_cast<IGameUI *>(gameUIFactory(GAMEUI_INTERFACE_VERSION, nullptr));
            if (!m_pGameUI)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

CMOMMenuOverride::~CMOMMenuOverride()
{
    m_pGameUI = nullptr;
    g_GameUIDLL.Unload();
}

void CMOMMenuOverride::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // Resize the panel to the screen size
    // Otherwise, it'll just be in a little corner
    int wide, tall;
    surface()->GetScreenSize(wide, tall);
    SetSize(wide, tall);
}

void CMOMMenuOverride::OnCommand(const char* command)
{
    BaseClass::OnCommand(command);

    GetGameUI()->SendMainMenuCommand(command);
}
