#pragma once

#include <vgui_controls/Frame.h>
#include "steam/steam_api_common.h"
#include "steam/isteamhttp.h"

class CChangelogPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CChangelogPanel, Frame);

    CChangelogPanel();
    ~CChangelogPanel();

    static void Init();

    void SetChangelogText(const char* pChangelogText);

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

    void Activate() OVERRIDE;

  private:
    void GetRemoteChangelog();
    CCallResult<CChangelogPanel, HTTPRequestCompleted_t> m_callResult;
    void ChangelogCallback(HTTPRequestCompleted_t *pParam, bool bIOFailure);

    vgui::RichText *m_pChangeLog;
    char m_cOnlineVersion[12];
    wchar_t *m_pwOnlineChangelog;
};