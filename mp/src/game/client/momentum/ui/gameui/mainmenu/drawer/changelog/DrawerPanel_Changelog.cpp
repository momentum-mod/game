#include "cbase.h"

#include "DrawerPanel_Changelog.h"

#include "vgui_controls/RichText.h"
#include "vgui/ILocalize.h"

#include "fmtstr.h"
#include "mom_shareddefs.h"

#include "filesystem.h"

#include "tier0/memdbgon.h"

using namespace vgui;

CON_COMMAND(mom_version, "Prints mod current installed version\n")
{
    Log("Momentum Mod v%s\n", MOM_CURRENT_VERSION);
}

DrawerPanel_Changelog::DrawerPanel_Changelog(Panel *pParent) : BaseClass(pParent, "DrawerPanel_Changelog")
{
    SetProportional(true);

    V_memset(m_cOnlineVersion, 0, sizeof(m_cOnlineVersion));
    m_pwOnlineChangelog = nullptr;

    m_pChangeLog = new RichText(this, "ChangeLog");
    m_pChangeLog->SetText("#MOM_API_WaitingForResponse");
}

DrawerPanel_Changelog::~DrawerPanel_Changelog()
{
    m_callResult.Cancel();

    free(m_pwOnlineChangelog);
}

void DrawerPanel_Changelog::OnResetData()
{
    LoadControlSettings("resource/ui/mainmenu/DrawerPanel_Changelog.res");

    GetLocalChangelog();
}

void DrawerPanel_Changelog::OnReloadControls()
{
    BaseClass::OnReloadControls();

    if (m_pwOnlineChangelog)
    {
        m_pChangeLog->SetText(m_pwOnlineChangelog);
    }
}

void DrawerPanel_Changelog::GetLocalChangelog()
{
    CUtlBuffer changelog_buf;
    if(filesystem->ReadFile("changelog.txt", "MOD", changelog_buf))
    {
        SetChangelogText(changelog_buf.String());
    }
    changelog_buf.Purge();
}

void DrawerPanel_Changelog::GetRemoteChangelog()
{
    CHECK_STEAM_API(SteamHTTP());

    const auto handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, "https://raw.githubusercontent.com/momentum-mod/game/master/changelog.txt");
    SteamAPICall_t apiHandle;

    if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
    {
        m_callResult.Set(apiHandle, this, &DrawerPanel_Changelog::ChangelogCallback);
    }
    else
    {
        SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        Warning("Failed to send HTTP Request to get changelog!\n");
        SetChangelogText("#MOM_Drawer_Changelog_Error");
    }
}

void DrawerPanel_Changelog::SetChangelogText(const char *pChangelogText)
{
    if (m_pwOnlineChangelog)
    {
        free(m_pwOnlineChangelog);
        m_pwOnlineChangelog = nullptr;
    }

    g_pVGuiLocalize->ConvertUTF8ToUTF16(pChangelogText, &m_pwOnlineChangelog);

    m_pChangeLog->SetText(m_pwOnlineChangelog);
}

void DrawerPanel_Changelog::ChangelogCallback(HTTPRequestCompleted_t *pParam, bool bIOFailure)
{
    const char *pChangelogText = "#MOM_Drawer_Changelog_Error";
    byte *pData = nullptr;
    if (bIOFailure)
    {
        Warning("Error loading changelog; bIOFailure!\n");
    }
    else if (pParam->m_eStatusCode != k_EHTTPStatusCode200OK)
    {
        Warning("Error loading changelog, HTTP response code: %i\n", pParam->m_eStatusCode);
    }
    else
    {
        uint32 size;
        SteamHTTP()->GetHTTPResponseBodySize(pParam->m_hRequest, &size);
        if (size == 0)
        {
            Warning("MomentumUtil::ChangelogCallback: 0 body size!\n");
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

    SetChangelogText(pChangelogText);

    delete[] pData;

    SteamHTTP()->ReleaseHTTPRequest(pParam->m_hRequest);
}