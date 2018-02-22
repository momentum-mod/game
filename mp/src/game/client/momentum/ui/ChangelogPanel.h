#pragma once

#include "cbase.h"

#include "IChangelogPanel.h"
#include <vgui_controls/Frame.h>

using namespace vgui;

// CChangelogPanel class
class CChangelogPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CChangelogPanel, Frame);
    // CChangelogPanel : This Class / vgui::Frame : BaseClass

    CChangelogPanel(VPANEL parent); // Constructor
    ~CChangelogPanel();;

    void SetChangelog(const char* pChangelog);

    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;

    void Activate() OVERRIDE;
    void OnThink() OVERRIDE;

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
    CChangelogInterface();
    virtual ~CChangelogInterface() {}
    void Create(vgui::VPANEL parent) OVERRIDE;

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

    void SetChangelog(const char *pChangelog) const OVERRIDE
    {
        if (pPanel)
        {
            pPanel->SetChangelog(pChangelog);
        }
    }
};