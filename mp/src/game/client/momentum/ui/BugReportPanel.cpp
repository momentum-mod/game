//The following include files are necessary to allow your  the panel .cpp to compile.
#include "cbase.h"

#include "IBugReportPanel.h"
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/pch_vgui_controls.h>
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class CBugReportPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(CBugReportPanel, vgui::Frame);
    //CBugReportPanel : This Class / vgui::Frame : BaseClass

    CBugReportPanel(VPANEL parent); 	// Constructor
    ~CBugReportPanel() {};				// Destructor

    void OnThink() override;
    void Activate() override;
    void InitPanel();
protected:
    MESSAGE_FUNC_CHARPTR(OnURLChange, "OnFinishRequest", URL)
    {
        DevLog("URL FINISHED LOADING %s\n", URL);
        //MOM_TODO: Do we want to have anything custom when they submit a contact form?
    }

private:
    HTML *m_pWebPage;
};

// Constuctor: Initializes the Panel
CBugReportPanel::CBugReportPanel(VPANEL parent)
    : BaseClass(nullptr, "CBugReportPanel")
{
    SetParent(parent);
    SetPaintBackgroundType(1);
    SetRoundedCorners(PANEL_ROUND_CORNER_ALL);
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetProportional(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMoveable(true);
    SetVisible(false);
    AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/BugReportPanel.res");
    m_pWebPage = new HTML(this, "HTMLForm", true);
    m_pWebPage->AddActionSignalTarget(this);

    InitPanel();
}

void CBugReportPanel::InitPanel()
{
#define SCALE(num) scheme()->GetProportionalScaledValue(num)
#define SCALEXY(x,y) SCALE(x), SCALE(y)

    m_pWebPage->SetPos(SCALEXY(0, 25));
    m_pWebPage->SetSize(GetWide(), GetTall() - SCALE(25));
    m_pWebPage->OpenURL("http://momentum-mod.org/contact", nullptr);
}

//Class: CBugReportPanelInterface Class. Used for construction.
class CBugReportPanelInterface : public IBugReportPanel
{
private:
    CBugReportPanel *reportubug_panel;
public:
    CBugReportPanelInterface()
    {
        reportubug_panel = nullptr;
    }
    ~CBugReportPanelInterface()
    {
        reportubug_panel = nullptr;
    }
    void Create(VPANEL parent) override
    {
        reportubug_panel = new CBugReportPanel(parent);
    }
    void Destroy() override
    {
        if (reportubug_panel)
        {
            reportubug_panel->SetParent(nullptr);
            delete reportubug_panel;
        }
    }
    void Activate(void) override
    {
        if (reportubug_panel)
        {
            reportubug_panel->Activate();
            reportubug_panel->SetKeyBoardInputEnabled(true);
        }
    }
    void Close() override
    {
        if (reportubug_panel)
        {
            reportubug_panel->Close();
            reportubug_panel->SetKeyBoardInputEnabled(false);
        }
    }
};
static CBugReportPanelInterface g_BugReportPanel;
IBugReportPanel* bug_report = static_cast<IBugReportPanel*>(&g_BugReportPanel);

CON_COMMAND(mom_bugreport_show, "Shows the bug report panel.\n")
{
    bug_report->Activate();
}

void CBugReportPanel::OnThink()
{
    BaseClass::OnThink();
    GetAnimationController()->UpdateAnimations(system()->GetFrameTime());
}

void CBugReportPanel::Activate()
{
    BaseClass::Activate();
    InitPanel();
}