// The following include files are necessary to allow The Panel .cpp to compile.
#include "cbase.h"

#include "ChangelogPanel.h"

#include "tier0/memdbgon.h"

// Constuctor: Initializes the Panel
CChangelogPanel::CChangelogPanel(VPANEL parent) : BaseClass(nullptr, "ChangelogPanel")
{
    V_memset(m_cOnlineVersion, 0, sizeof(m_cOnlineVersion));
    V_memset(m_pwOnlineChangelog, 0, sizeof(m_pwOnlineChangelog));

    SetParent(parent);
    LoadControlSettings("resource/ui/ChangelogPanel.res");
    m_pReleaseText = FindControl<URLLabel>("ReleaseText", true);
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

    if (!m_pReleaseText || !m_pChangeLog)
    {
        Assert("Missing one more gameui controls from ui/changelogpanel.res");
    }
}

// Called when the versions don't match (there's an update)
void CChangelogPanel::Activate()
{
    char m_cReleaseText[225];
    m_pReleaseText->GetText(m_cReleaseText, sizeof(m_cReleaseText));
    char m_cReleaseF[225];

    Q_snprintf(m_cReleaseF, 225, m_cReleaseText, MOM_CURRENT_VERSION, m_cOnlineVersion);
    m_pReleaseText->SetText(m_cReleaseF);
    m_pReleaseText->SetURL("https://github.com/momentum-mod/game/releases");

    BaseClass::Activate();
}

CON_COMMAND(mom_version, "Prints mod current installed version")
{
    Log("Mod currently installed version: %s\n", MOM_CURRENT_VERSION);
    // MOM_TODO: Do we want to check for new versions in the future?
}

// Interface this class to the rest of the DLL
static CChangelogInterface g_Changelog;
IChangelogPanel *changelogpanel = static_cast<IChangelogPanel *>(&g_Changelog);