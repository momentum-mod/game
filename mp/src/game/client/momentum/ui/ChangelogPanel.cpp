// The following include files are necessary to allow The Panel .cpp to compile.
#include "cbase.h"

#include "ChangelogPanel.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

// Constuctor: Initializes the Panel
CChangelogPanel::CChangelogPanel(VPANEL parent) : BaseClass(nullptr, "ChangelogPanel")
{
    V_memset(m_cOnlineVersion, 0, sizeof(m_cOnlineVersion));
    m_pwOnlineChangelog = nullptr;

    SetParent(parent);
    LoadControlSettings("resource/ui/ChangelogPanel.res");
    m_pChangeLog = FindControl<RichText>("ChangeLog", true);
    m_flScrollTime = -1.0f;

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMinimumSize(400, 250);
    SetMoveable(true);
    SetVisible(false);
    SetProportional(true);

    if (!m_pChangeLog)
    {
        Assert("Missing one more gameui controls from ui/changelogpanel.res");
    }
}

// Called when the versions don't match (there's an update)
void CChangelogPanel::Activate()
{
    // Reset the version warning to keep reminding them
    // MOM_TODO: re-enable this when the steam version is newer ConVarRef("mom_toggle_versionwarn").SetValue(0);
    g_pMomentumUtil->GetRemoteChangelog();

    BaseClass::Activate();
}

CON_COMMAND(mom_version, "Prints mod current installed version\n")
{
    Log("Mod currently installed version: %s\n", MOM_CURRENT_VERSION);
    // MOM_TODO: Do we want to check for new versions in the future?
}

CON_COMMAND(mom_show_changelog, "Shows the changelog for the mod.\n")
{
    changelogpanel->Activate();
}

// Interface this class to the rest of the DLL
static CChangelogInterface g_Changelog;
IChangelogPanel *changelogpanel = static_cast<IChangelogPanel *>(&g_Changelog);