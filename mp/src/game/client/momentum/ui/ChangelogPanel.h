#pragma once

#include "cbase.h"

#include "IChangelogPanel.h"
#include <vgui/ILocalize.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/URLLabel.h>

#include "momentum/mom_shareddefs.h"

using namespace vgui;

// CChangelogPanel class
class CChangelogPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CChangelogPanel, Frame);
    // CChangelogPanel : This Class / vgui::Frame : BaseClass

    CChangelogPanel(VPANEL parent); // Constructor
    ~CChangelogPanel()
    {
        free(m_pwOnlineChangelog);
    };

    void SetVersion(const char *version) { Q_strncpy(m_cOnlineVersion, version, 12); }

    void SetChangelog(const char *pChangelog)
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
            // Delay the scrolling to a tick or so away, thanks Valve.
            m_flScrollTime = system()->GetFrameTime() + 0.010f;
        }
    }

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        BaseClass::ApplySchemeSettings(pScheme);
        m_pChangeLog->SetFont(pScheme->GetFont("DefaultSmall"));
    }

    void Activate() OVERRIDE;

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        if (m_flScrollTime > 0.0f && system()->GetFrameTime() > m_flScrollTime)
        {
            m_pChangeLog->GotoTextStart();
            m_flScrollTime = -1.0f;
        }
    }

    void OnKillFocus() OVERRIDE
    {
        BaseClass::OnKillFocus();
        //Close();
    }

  private:
    float m_flScrollTime;
    RichText *m_pChangeLog;
    char m_cOnlineVersion[12];
    wchar_t *m_pwOnlineChangelog;
};

class CChangelogInterface : public IChangelogPanel
{
  private:
    CChangelogPanel *pPanel;

  public:
    CChangelogInterface() { pPanel = nullptr; }
    virtual ~CChangelogInterface() {}
    void Create(vgui::VPANEL parent) OVERRIDE { pPanel = new CChangelogPanel(parent); }
    void Destroy() OVERRIDE
    {
        if (pPanel)
        {
            pPanel->SetParent(nullptr);
            delete pPanel;
        }
        pPanel = nullptr;
    }
    void Activate(void) OVERRIDE
    {
        if (pPanel)
        {
            pPanel->Activate();
        }
    }
    void Close() OVERRIDE
    {
        if (pPanel)
        {
            pPanel->Close();
        }
    }

    void SetVersion(const char *pVersion) const OVERRIDE
    {
        if (pPanel)
        {
            pPanel->SetVersion(pVersion);
        }
    }

    void SetChangelog(const char *pChangelog) const OVERRIDE
    {
        if (pPanel)
        {
            pPanel->SetChangelog(pChangelog);
        }
    }
};