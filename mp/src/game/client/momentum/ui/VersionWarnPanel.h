#pragma once

#include "cbase.h"

#include "IVersionWarnPanel.h"
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/URLLabel.h>

#include "momentum/mom_shareddefs.h"

using namespace vgui;

// CVersionWarnPanel class
class CVersionWarnPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CVersionWarnPanel, Frame);
    // CVersionWarnPanel : This Class / vgui::Frame : BaseClass

    CVersionWarnPanel(VPANEL parent); // Constructor
    ~CVersionWarnPanel(){};           // Destructor

    void SetVersion(const char *version) { Q_strncpy(m_cOnlineVersion, version, 12); }

    void SetChangelog(char *pChangelog)
    {
        Q_strncpy(m_cOnlineChangelog, pChangelog, sizeof(m_cOnlineChangelog));
        if (m_pChangeLog)
        {
            m_pChangeLog->InsertString(m_cOnlineChangelog);
            m_pChangeLog->GotoTextStart();
        }
    }

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        BaseClass::ApplySchemeSettings(pScheme);
        m_pChangeLog->SetFont(pScheme->GetFont("DefaultSmall"));
    }

    void Activate() override;

  protected:
    // VGUI overrides:
    void OnCommand(const char *pcCommand) override;

  private:
    // Other used VGUI control Elements:
    URLLabel *m_pReleaseText;
    RichText *m_pChangeLog;
    char m_cOnlineVersion[12];
    char m_cOnlineChangelog[4056]; // MOM_TODO: Determine a better size
};

class CVersionWarnPanelInterface : public IVersionWarnPanel
{
  private:
    CVersionWarnPanel *pPanel;

  public:
    CVersionWarnPanelInterface() { pPanel = nullptr; }
    ~CVersionWarnPanelInterface() { }
    void Create(vgui::VPANEL parent) override { pPanel = new CVersionWarnPanel(parent); }
    void Destroy() override
    {
        if (pPanel)
        {
            pPanel->SetParent(nullptr);
            delete pPanel;
        }
        pPanel = nullptr;
    }
    void Activate(void) override
    {
        if (pPanel)
        {
            pPanel->Activate();
        }
    }
    void Close() override
    {
        if (pPanel)
        {
            pPanel->Close();
        }
    }

    void SetVersion(const char *pVersion) const override
    {
        if (pPanel)
        {
            pPanel->SetVersion(pVersion);
        }
    }

    void SetChangelog(char *pChangelog) const override
    {
        if (pPanel)
        {
            pPanel->SetChangelog(pChangelog);
        }
    }
};