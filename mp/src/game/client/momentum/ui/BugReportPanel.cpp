//The following include files are necessary to allow your  the panel .cpp to compile.
#include "cbase.h"
#include "IBugReportPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/pch_vgui_controls.h>
#include "momentum/util/mom_util.h"
#include "momentum/mom_shareddefs.h"
#include "tier0/memdbgon.h"

class CBugReportPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CBugReportPanel, vgui::Frame);
    //CBugReportPanel : This Class / vgui::Frame : BaseClass

    CBugReportPanel(vgui::VPANEL parent); 	// Constructor
    ~CBugReportPanel() {};				// Destructor

    void Activate() override;
    void InitPanel();
protected:
    //VGUI overrides:
    void OnTick() override;
    bool VerifyReport();
    MESSAGE_FUNC(OnSubmitReport, "SubmitReport")
    {
        if (VerifyReport())
        {
            char email[30];
            char report[1000];
            m_pEmailTextEntry->GetText(email, m_pEmailTextEntry->GetTextLength() * sizeof(char));
            m_pBugTextEntry->GetText(report, m_pBugTextEntry->GetTextLength() * sizeof(char));
            
            if (mom_UTIL->ReportBug(email, report))
            {
                InitPanel();
            }
        }
    }

private:

    Button *m_pSubmitButton;
    Label *m_pEmailLabel, *m_pDescribeBugLabel, *m_pCharsLeftLabel;
    TextEntry *m_pEmailTextEntry, *m_pBugTextEntry;
};

// Constuctor: Initializes the Panel
CBugReportPanel::CBugReportPanel(vgui::VPANEL parent)
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
    SetScheme("ClientScheme");

    LoadControlSettings("resource/ui/BugReportPanel.res");

    m_pEmailLabel = FindControl<Label>("EmailLabel", true);
    m_pDescribeBugLabel = FindControl<Label>("DescribeLabel", true);

    m_pBugTextEntry = new TextEntry(this, "BugText");
    m_pEmailTextEntry = new TextEntry(this, "EmailText");

    m_pSubmitButton = FindControl<Button>("SubmitButton", true);

    ivgui()->AddTickSignal(GetVPanel());

    InitPanel();
}

void CBugReportPanel::InitPanel()
{
#define SCALE(num) scheme()->GetProportionalScaledValueEx(GetScheme(), (num))
#define SCALEXY(x,y) SCALE(x), SCALE(y)
    if (m_pSubmitButton && m_pEmailLabel && m_pDescribeBugLabel && m_pBugTextEntry && m_pEmailTextEntry)
    {
        m_pEmailTextEntry->SetPos(SCALEXY(67, 38));
        m_pEmailTextEntry->SetDrawWidth(SCALE(550));
        m_pEmailTextEntry->SetSize(SCALEXY(200, 13));
        m_pEmailTextEntry->SetMultiline(false);
        m_pEmailTextEntry->SetMaximumCharCount(30);
        m_pEmailTextEntry->SetText("");
        m_pEmailTextEntry->SetTabPosition(1);

        m_pBugTextEntry->SetPos(SCALEXY(67, 72));
        m_pBugTextEntry->SetDrawWidth(SCALE(550));
        m_pBugTextEntry->SetSize(SCALEXY(200, 220));
        m_pBugTextEntry->SetMultiline(true);
        m_pBugTextEntry->SetMaximumCharCount(1000);
        m_pBugTextEntry->SetText("");
        m_pBugTextEntry->SetTabPosition(2);

        m_pSubmitButton->SetCommand("SubmitReport");
        m_pSubmitButton->AddActionSignalTarget(this);
        m_pSubmitButton->SetEnabled(false);
    }
    else
    {
        DevLog("Report bug has a bug (Nullptr) Oh the irony...\n");
    }
}

//Class: CBugReportPanelInterface Class. Used for construction.
class CBugReportPanelInterface : public BugReportPanel
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
    void Create(vgui::VPANEL parent)
    {
        reportubug_panel = new CBugReportPanel(parent);
    }
    void Destroy()
    {
        if (reportubug_panel)
        {
            reportubug_panel->SetParent(nullptr);
            delete reportubug_panel;
        }
    }
    void Activate(void)
    {
        if (reportubug_panel)
        {
            reportubug_panel->Activate();
            reportubug_panel->SetKeyBoardInputEnabled(true);
        }
    }
    void Close()
    {
        if (reportubug_panel)
        {
            reportubug_panel->Close();
            reportubug_panel->SetKeyBoardInputEnabled(false);
        }
    }
};
static CBugReportPanelInterface g_BugReportPanel;
BugReportPanel* bug_report = (BugReportPanel*)&g_BugReportPanel;

CON_COMMAND(mom_bugreport_show, "Shows the bug report panel.\n")
{
    bug_report->Activate();
}

void CBugReportPanel::OnTick()
{
    BaseClass::OnTick();
    vgui::GetAnimationController()->UpdateAnimations(system()->GetFrameTime());

    m_pSubmitButton->SetEnabled(IsVisible() && VerifyReport());

}

void CBugReportPanel::Activate()
{
    BaseClass::Activate();
}

bool CBugReportPanel::VerifyReport()
{
    //MOM_TODO: More verification would be needed here
    return (m_pBugTextEntry && m_pEmailTextEntry && m_pEmailTextEntry->GetTextLength() >= 6 && m_pBugTextEntry->GetTextLength() >= 15);
}