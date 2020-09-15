#pragma once

#include "vgui_controls/PropertyPage.h"

#include "steam/steam_api_common.h"
#include "steam/isteamhttp.h"

class DrawerPanel_Changelog : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(DrawerPanel_Changelog, vgui::PropertyPage);
    DrawerPanel_Changelog(Panel *pParent);
    ~DrawerPanel_Changelog();

protected:
    void OnResetData() override;
    void OnReloadControls() override;

private:
    void GetRemoteChangelog();
    void SetChangelogText(const char *pChangelogText);

    CCallResult<DrawerPanel_Changelog, HTTPRequestCompleted_t> m_callResult;
    void ChangelogCallback(HTTPRequestCompleted_t *pParam, bool bIOFailure);

    vgui::RichText *m_pChangeLog;

    char m_cOnlineVersion[12];
    wchar_t *m_pwOnlineChangelog;
};