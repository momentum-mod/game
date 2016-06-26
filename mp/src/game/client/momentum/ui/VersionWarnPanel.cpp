// The following include files are necessary to allow The Panel .cpp to compile.
#include "cbase.h"

#include "VersionWarnPanel.h"

#include "tier0/memdbgon.h"

// Constuctor: Initializes the Panel
CVersionWarnPanel::CVersionWarnPanel(VPANEL parent) : BaseClass(nullptr, "VersionWarnPanel")
{
    V_memset(m_cOnlineVersion, 0, sizeof(m_cOnlineVersion));
    SetParent(parent);
    LoadControlSettings("resource/ui/versionwarnpanel.res");
    m_pReleaseText = FindControl<URLLabel>("ReleaseText", true);
    m_pChangeLog = FindControl<RichText>("ChangeLog", true);

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

    //SetScheme(scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
    if (!m_pReleaseText || !m_pChangeLog)
    {
        Assert("Missing one more gameui controls from ui/versionwarnpanel.res");
    }
}

// Called when the versions don't match (there's an update)
void CVersionWarnPanel::Activate()
{
    //HFont m_hfReleaseFont = m_pReleaseText->GetFont();
    char m_cReleaseText[225];
    m_pReleaseText->GetText(m_cReleaseText, sizeof(m_cReleaseText));
    char m_cReleaseF[225];

    Q_snprintf(m_cReleaseF, 225, m_cReleaseText, MOM_CURRENT_VERSION, m_cOnlineVersion);
    m_pReleaseText->SetText(m_cReleaseF);
    m_pReleaseText->SetURL("https://github.com/momentum-mod/game/releases");

    BaseClass::Activate();
}

void CVersionWarnPanel::OnCommand(const char *pcCommand)
{
    BaseClass::OnCommand(pcCommand);

    if (!Q_stricmp(pcCommand, "turnoff"))
    {
        SetVisible(false);
        Close();
    }
}

CON_COMMAND(mom_version, "Prints mod current installed version")
{
    Log("Mod currently installed version: %s\n", MOM_CURRENT_VERSION);
    // MOM_TODO: Do we want to check for new versions in the future?
}

// Interface this class to the rest of the DLL
static CVersionWarnPanelInterface g_VersionWarn;
IVersionWarnPanel *versionwarnpanel = static_cast<IVersionWarnPanel *>(&g_VersionWarn);