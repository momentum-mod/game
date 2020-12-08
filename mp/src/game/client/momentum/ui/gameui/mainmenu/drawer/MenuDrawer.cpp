#include "cbase.h"

#include "MenuDrawer.h"

#include "changelog/DrawerPanel_Changelog.h"
#include "lobby/DrawerPanel_Lobby.h"
#include "profile/DrawerPanel_Profile.h"

#include "mom_shareddefs.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/PropertySheet.h"

#include "tier0/memdbgon.h"
#include "vgui_controls/AnimationController.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_drawer_animation_enable, "1", FCVAR_ARCHIVE, "Toggle animating the opening/closing of the drawer panel. 0 = OFF, 1 = ON\n");
static MAKE_CONVAR(mom_drawer_animation_time, "0.33", FCVAR_ARCHIVE, "Controls the amount of time the main menu drawer animated its opening/closing, in seconds.\n", 0.01f, 500.0f);

MenuDrawerPanel::MenuDrawerPanel(Panel *pParent) : BaseClass(pParent, "MenuDrawer")
{
    SetProportional(true);

    m_bDrawerOpen = false;
    m_fPanelX = -1.0f;

    const auto hScheme = scheme()->LoadSchemeFromFile("resource/MenuDrawerScheme.res", "MenuDrawerScheme");
    if (hScheme)
    {
        SetScheme(hScheme);
    }

    m_pDrawerHandleButton = new Button(this, "DrawerHandleButton", "<", this, "DrawerToggle");

    m_pDrawerContent = new PropertySheet(this, "DrawerContent");
    m_pDrawerContent->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/mainmenu/MenuDrawer.res");

    m_pDrawerContent->InvalidateLayout(true, true);

    m_pProfileDrawerPanel = new DrawerPanel_Profile(m_pDrawerContent);
    m_pLobbyDrawerPanel = new DrawerPanel_Lobby(m_pDrawerContent);
    m_pChangelogDrawerPanel = new DrawerPanel_Changelog(m_pDrawerContent);

    m_pDrawerContent->AddPage(m_pProfileDrawerPanel, "#MOM_Drawer_Profile");
    m_pDrawerContent->AddPage(m_pLobbyDrawerPanel, "#MOM_Drawer_Lobby");
    m_pDrawerContent->AddPage(m_pChangelogDrawerPanel, "#MOM_Drawer_Changelog");
    // we need the raw strings here
    m_pDrawerContent->DisablePage("Profile");
    m_pDrawerContent->DisablePage("Lobby");
    m_pDrawerContent->SetActivePage(m_pChangelogDrawerPanel);
    m_iActivePage = DRAWER_TAB_CHANGELOG;
}

void MenuDrawerPanel::PerformLayout()
{
    BaseClass::PerformLayout();

    if (mom_drawer_animation_enable.GetBool())
    {
        if (m_fPanelX < GetDesiredDrawerPosition(true) || m_fPanelX > GetDesiredDrawerPosition(false))
        {
            m_fPanelX = GetDesiredDrawerPosition(false);
        }

        return;
    }

    m_pDrawerContent->SetVisible(m_bDrawerOpen);

    SetPos(GetDesiredDrawerPosition(m_bDrawerOpen), 0);
}

void MenuDrawerPanel::OnCommand(const char* command)
{
    if (FStrEq(command, "DrawerToggle"))
    {
        ToggleDrawer();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void MenuDrawerPanel::OnReloadControls()
{
    BaseClass::OnReloadControls();

    m_pDrawerHandleButton->SetText(m_bDrawerOpen ? ">" : "<");
}

void MenuDrawerPanel::OnThink()
{
    BaseClass::OnThink();

    if (mom_drawer_animation_enable.GetBool())
    {
        const auto fClosedPosition = GetDesiredDrawerPosition(false);

        if (!m_pDrawerContent->IsVisible() && !CloseEnough(m_fPanelX, fClosedPosition))
            m_fPanelX = fClosedPosition;

        SetPos(m_fPanelX, 0);

        if (!m_bDrawerOpen && CloseEnough(m_fPanelX, fClosedPosition) && m_pDrawerContent->IsVisible())
        {
            m_pDrawerContent->SetVisible(false);
        }
        else if (m_bDrawerOpen && !m_pDrawerContent->IsVisible())
        {
            m_pDrawerContent->SetVisible(true);
        }
    }
}

void MenuDrawerPanel::OnScreenSizeChanged(int oldwide, int oldtall)
{
    if (m_bDrawerOpen)
    {
        m_pDrawerContent->SetVisible(false);
        ToggleDrawer(false);
    }

    BaseClass::OnScreenSizeChanged(oldwide, oldtall);
}

void MenuDrawerPanel::OnPageChanged()
{
    m_iActivePage = m_pDrawerContent->GetActivePageNum();
}

int MenuDrawerPanel::GetDesiredDrawerPosition(bool bOpen)
{
    if (bOpen)
        return ScreenWidth() - GetWide();

    return ScreenWidth() - m_pDrawerHandleButton->GetWide();
}

void MenuDrawerPanel::ToggleDrawer(bool bAnimate /*= true*/)
{
    m_bDrawerOpen = !m_bDrawerOpen;

    m_pDrawerHandleButton->SetText(m_bDrawerOpen ? ">" : "<");

    if (bAnimate)
        GetAnimationController()->RunAnimationCommand(this, "PanelX", GetDesiredDrawerPosition(m_bDrawerOpen), 0.0f, mom_drawer_animation_time.GetFloat(), AnimationController::INTERPOLATOR_SIMPLESPLINE);

    InvalidateLayout();

    RequestFocus(0);
}

void MenuDrawerPanel::OnLobbyEnter()
{
    OpenDrawerTo(DRAWER_TAB_LOBBY);
}

void MenuDrawerPanel::OnLobbyLeave()
{
    m_pLobbyDrawerPanel->OnLobbyLeave();
}

void MenuDrawerPanel::OnSpecStart()
{
    
}

void MenuDrawerPanel::OnSpecStop()
{
    
}


void MenuDrawerPanel::OnSiteAuth()
{

}

void MenuDrawerPanel::OpenDrawerTo(DrawerTab_t tab)
{
    if (!m_bDrawerOpen)
        ToggleDrawer();

    if (m_pDrawerContent->GetActivePageNum() != tab)
        m_pDrawerContent->SetActivePage(m_pDrawerContent->GetPage(tab));
}

int MenuDrawerPanel::GetDrawerButtonWidth() const
{
    return m_pDrawerHandleButton ? m_pDrawerHandleButton->GetWide() : 0;
}

void MenuDrawerPanel::OnKeyCodeTyped(KeyCode code)
{
    if (code == KEY_ESCAPE)
    {
        if (m_bDrawerOpen && !engine->IsInGame())
        {
            ToggleDrawer();
        }
    }
    else
    {
        BaseClass::OnKeyCodeTyped(code);
    }
}