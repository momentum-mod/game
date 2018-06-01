#include "cbase.h"

#include "ChangelogPanel.h"
#include "mom_shareddefs.h"
#include "vgui_controls/RichText.h"
#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

using namespace vgui;

// Constuctor: Initializes the Panel
CChangelogPanel::CChangelogPanel(VPANEL parent) : BaseClass(nullptr, "ChangelogPanel")
{
    V_memset(m_cOnlineVersion, 0, sizeof(m_cOnlineVersion));
    m_pwOnlineChangelog = nullptr;

    SetProportional(true);
    SetParent(parent);
    SetTitle("#MOM_ChangeLog", false);
    m_pChangeLog = new RichText(this, "ChangeLog");

    LoadControlSettings("resource/ui/ChangelogPanel.res");

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(true);
    SetMinimumSize(400, 250);
    SetMoveable(true);
    SetVisible(false);
}

CChangelogPanel::~CChangelogPanel()
{
    if (m_callResult.IsActive())
        m_callResult.Cancel();

    free(m_pwOnlineChangelog);
}

void CChangelogPanel::SetChangelog(const char* pChangelog)
{
    if (m_pChangeLog)
    {
        if (m_pwOnlineChangelog)
        {
            free(m_pwOnlineChangelog);
            m_pwOnlineChangelog = nullptr;
        }

        g_pVGuiLocalize->ConvertUTF8ToUTF16(pChangelog, &m_pwOnlineChangelog);

        m_pChangeLog->SetText(m_pwOnlineChangelog);
    }
}

void CChangelogPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    m_pChangeLog->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
} 

// Called when the versions don't match (there's an update)
void CChangelogPanel::Activate()
{
    // Reset the version warning to keep reminding them
    // MOM_TODO: re-enable this when the steam version is newer ConVarRef("mom_toggle_versionwarn").SetValue(0);
    GetRemoteChangelog();

    BaseClass::Activate();
}

void CChangelogPanel::GetRemoteChangelog()
{
    if (SteamHTTP())
    {
        HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, "http://raw.githubusercontent.com/momentum-mod/game/master/changelog.txt");
        SteamAPICall_t apiHandle;

        if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            m_callResult.Set(apiHandle, this, &CChangelogPanel::ChangelogCallback);
        }
        else
        {
            Warning("Failed to send HTTP Request to post scores online!\n");
            SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("Steampicontext failure.\n");
        Warning("Could not find Steam Api Context active\n");
    }
}

void CChangelogPanel::ChangelogCallback(HTTPRequestCompleted_t* pParam, bool bIOFailure)
{
    const char *pChangelogText = "Error loading changelog!";
    byte *pData = nullptr;
    if (bIOFailure)
    {
        pChangelogText = "Error loading changelog; bIOFailure!\n";
    }
    else if (pParam->m_eStatusCode != k_EHTTPStatusCode200OK)
    {
        pChangelogText = CFmtStr("Error loading changelog, HTTP response code: %i\n", pParam->m_eStatusCode).Get();
    }
    else
    {
        uint32 size;
        SteamHTTP()->GetHTTPResponseBodySize(pParam->m_hRequest, &size);
        if (size == 0)
        {
            pChangelogText = "MomentumUtil::ChangelogCallback: 0 body size!\n";
        }
        else
        {
            // Right now "size" is the content body size, not in string terms where the end is marked
            // with a null terminator.
            pData = new uint8[size + 1];
            SteamHTTP()->GetHTTPResponseBodyData(pParam->m_hRequest, pData, size);
            pData[size] = 0;
            pChangelogText = reinterpret_cast<const char *>(pData);
        }
    }

    changelogpanel->SetChangelog(pChangelogText);

    if (pData)
        delete[] pData;

    SteamHTTP()->ReleaseHTTPRequest(pParam->m_hRequest);
}


CChangelogInterface::CChangelogInterface()
{
    pPanel = nullptr;
}

void CChangelogInterface::Create(vgui::VPANEL parent)
{
    pPanel = new CChangelogPanel(parent);
}

CON_COMMAND(mom_version, "Prints mod current installed version\n")
{
    Log("Mod currently installed version: %s\n", MOM_CURRENT_VERSION);
}

CON_COMMAND(mom_show_changelog, "Shows the changelog for the mod.\n")
{
    changelogpanel->Activate();
}

// Interface this class to the rest of the DLL
static CChangelogInterface g_Changelog;
IChangelogPanel *changelogpanel = &g_Changelog;