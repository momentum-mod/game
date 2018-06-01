#include "cbase.h"

#include "ComparisonsSettingsPage.h"
#include <ienginevgui.h>
#include "clientmode.h"
#include "hud_comparisons.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/AnimationController.h>

#include <tier0/memdbgon.h>

using namespace vgui;

ComparisonsSettingsPage::ComparisonsSettingsPage(Panel *pParent) : BaseClass(pParent, "ComparisonsSettings")
{
    m_pCompareShow = FindControl<CvarToggleCheckButton>("CompareShow");

    m_pMaxZones = FindControl<TextEntry>("Zones");
    m_pMaxZonesLabel = FindControl<Label>("ZonesLabel");

    m_pCompareFormat = FindControl<CvarToggleCheckButton>("CompareFormat");

    m_pTimeTypeLabel = FindControl<Label>("TimeTypeLabel");
    m_pTimeTypeLabel->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeType = FindControl<ComboBox>("TimeType");
    m_pTimeType->SetNumberOfEditLines(2);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_Overall", nullptr);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_PerZone", nullptr);
    m_pTimeShowOverall = FindControl<CvarToggleCheckButton>("TimeShowOverall");
    m_pTimeShowOverall->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowZone = FindControl<CvarToggleCheckButton>("TimeShowZone");
    m_pTimeShowZone->GetTooltip()->SetTooltipFormatToSingleLine();

    m_pVelocityShow = FindControl<CvarToggleCheckButton>("VelShow");
    m_pVelocityShowAvg = FindControl<CvarToggleCheckButton>("VelShowAvg");
    m_pVelocityShowMax = FindControl<CvarToggleCheckButton>("VelShowMax");
    m_pVelocityShowEnter = FindControl<CvarToggleCheckButton>("VelShowEnter");
    m_pVelocityShowEnter->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowExit = FindControl<CvarToggleCheckButton>("VelShowExit");
    m_pVelocityShowExit->GetTooltip()->SetTooltipFormatToSingleLine();

    m_pSyncShow = FindControl<CvarToggleCheckButton>("SyncShow");
    m_pSyncShowS1 = FindControl<CvarToggleCheckButton>("SyncShowS1");
    m_pSyncShowS1->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS2 = FindControl<CvarToggleCheckButton>("SyncShowS2");
    m_pSyncShowS2->GetTooltip()->SetTooltipFormatToSingleLine();

    m_pJumpShow = FindControl<CvarToggleCheckButton>("ShowJumps");
    m_pStrafeShow = FindControl<CvarToggleCheckButton>("ShowStrafes");

    m_pComparisonsFrame = nullptr;
}

ComparisonsSettingsPage::~ComparisonsSettingsPage()
{
}

void ComparisonsSettingsPage::DestroyBogusComparePanel()
{
    if (m_pBogusComparisonsPanel)
        m_pBogusComparisonsPanel->DeletePanel();

    if (m_pComparisonsFrame)
        m_pComparisonsFrame->DeletePanel();

    m_pComparisonsFrame = nullptr;
    m_pBogusComparisonsPanel = nullptr;
}

void ComparisonsSettingsPage::InitBogusComparePanel()
{
    //Initialize the frame we're putting this into
    m_pComparisonsFrame = new Frame(nullptr, "ComparisonsFrame");
    m_pComparisonsFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pComparisonsFrame->SetSize(350, scheme()->GetProportionalScaledValue(275));
    m_pComparisonsFrame->SetMoveable(false);
    m_pComparisonsFrame->MoveToFront();
    m_pComparisonsFrame->SetSizeable(false);
    m_pComparisonsFrame->SetTitle("#MOM_Settings_Compare_Bogus_Run", false);
    m_pComparisonsFrame->SetTitleBarVisible(true);
    m_pComparisonsFrame->SetMenuButtonResponsive(false);
    m_pComparisonsFrame->SetCloseButtonVisible(true);
    m_pComparisonsFrame->SetMinimizeButtonVisible(false);
    m_pComparisonsFrame->SetMaximizeButtonVisible(false);
    m_pComparisonsFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);

    //Initialize a bogus version of the HUD element
    m_pBogusComparisonsPanel = new C_RunComparisons("BogusComparisonsPanel");
    m_pBogusComparisonsPanel->SetParent(m_pComparisonsFrame);
    m_pBogusComparisonsPanel->AddActionSignalTarget(this);
    m_pBogusComparisonsPanel->SetPaintBackgroundEnabled(true);
    m_pBogusComparisonsPanel->SetPaintBackgroundType(2);
    m_pBogusComparisonsPanel->Init();
    m_pBogusComparisonsPanel->SetSize(200, 150);
    m_pBogusComparisonsPanel->SetPos(14, 30);
    m_pBogusComparisonsPanel->LoadBogusComparisons();
    IScheme *pClientScheme = scheme()->GetIScheme(scheme()->GetScheme("ClientScheme"));
    if (!pClientScheme)
    {
        //Only load this if we haven't already
        scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
        pClientScheme = scheme()->GetIScheme(scheme()->GetScheme("ClientScheme"));
    }
    m_pBogusComparisonsPanel->ApplySchemeSettings(pClientScheme);
    m_pBogusComparisonsPanel->SetVisible(true);
    m_pBogusComparisonsPanel->MakeReadyForUse();

    //Finally, set the frame visible (after the bogus panel is loaded and dandy)
    m_pComparisonsFrame->SetVisible(true);
}

void ComparisonsSettingsPage::OnMainDialogClosed()
{
    if (m_pComparisonsFrame)
        m_pComparisonsFrame->Close();
}

void ComparisonsSettingsPage::OnMainDialogShow()
{
    if (m_pComparisonsFrame)
        m_pComparisonsFrame->SetVisible(true);
}

void ComparisonsSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    if (m_pMaxZones)
    {
        ConVarRef zones("mom_comparisons_max_zones");
        char buf[64];
        m_pMaxZones->GetText(buf, sizeof(buf));
        int zonesNum = atoi(buf);
        if (zonesNum < 0) // Less than min
        {
            zones.SetValue(1);
        }
        else if (zonesNum > 10) // Greater than max
        {
            zones.SetValue(10);
        }
        else // In range
        {
            zones.SetValue(zonesNum);
        }
    }

    if (m_pTimeType)
    {
        ConVarRef("mom_comparisons_time_type").SetValue(m_pTimeType->GetActiveItem());
    }
}

void ComparisonsSettingsPage::LoadSettings()
{
    if (m_pMaxZones)
    {
        m_pMaxZones->SetText(ConVarRef("mom_comparisons_max_zones").GetString());
    }

    if (m_pTimeType)
    {
        m_pTimeType->ActivateItemByRow(ConVarRef("mom_comparisons_time_type").GetInt());
    }
}

void ComparisonsSettingsPage::OnPageShow()
{
    BaseClass::OnPageShow();

    if (!m_pComparisonsFrame)
        InitBogusComparePanel();
    else if (!m_pComparisonsFrame->IsVisible())
        m_pComparisonsFrame->Activate();
}

void ComparisonsSettingsPage::OnPageHide()
{
    if (m_pComparisonsFrame)
        m_pComparisonsFrame->Close();
}

void ComparisonsSettingsPage::OnCheckboxChecked(Panel *p)
{
    BaseClass::OnCheckboxChecked(p);

    if (p == m_pCompareShow)
    {
        //Turn everything on/off
        bool bEnabled = m_pCompareShow->IsSelected();

        m_pMaxZones->SetEnabled(bEnabled);
        m_pMaxZonesLabel->SetEnabled(bEnabled);
        m_pCompareFormat->SetEnabled(bEnabled);

        //Time
        m_pTimeType->SetEnabled(bEnabled);
        m_pTimeTypeLabel->SetEnabled(bEnabled);
        m_pTimeShowOverall->SetEnabled(bEnabled);
        m_pTimeShowZone->SetEnabled(bEnabled);

        //Velocity
        m_pVelocityShow->SetEnabled(bEnabled);
        m_pVelocityShowExit->SetEnabled(bEnabled);
        m_pVelocityShowAvg->SetEnabled(bEnabled);
        m_pVelocityShowEnter->SetEnabled(bEnabled);
        m_pVelocityShowMax->SetEnabled(bEnabled);

        //Sync
        m_pSyncShow->SetEnabled(bEnabled);
        m_pSyncShowS1->SetEnabled(bEnabled);
        m_pSyncShowS2->SetEnabled(bEnabled);

        //Keypress
        m_pJumpShow->SetEnabled(bEnabled);
        m_pStrafeShow->SetEnabled(bEnabled);

        //Bogus panel
        if (m_pComparisonsFrame)
            m_pComparisonsFrame->SetVisible(bEnabled);
    }

    else if (p == m_pVelocityShow)
    {
        bool bEnabled = m_pVelocityShow->IsSelected();

        m_pVelocityShowExit->SetEnabled(bEnabled);
        m_pVelocityShowExit->SetSelected(bEnabled);
        m_pVelocityShowAvg->SetEnabled(bEnabled);
        m_pVelocityShowAvg->SetSelected(bEnabled);
        m_pVelocityShowEnter->SetEnabled(bEnabled);
        m_pVelocityShowEnter->SetSelected(bEnabled);
        m_pVelocityShowMax->SetEnabled(bEnabled);
        m_pVelocityShowMax->SetSelected(bEnabled);
    }

    else if (p == m_pSyncShow)
    {
        bool bEnabled = m_pSyncShow->IsSelected();

        m_pSyncShowS1->SetEnabled(bEnabled);
        m_pSyncShowS1->SetSelected(bEnabled);
        m_pSyncShowS2->SetEnabled(bEnabled);
        m_pSyncShowS2->SetSelected(bEnabled);
    }
}

void ComparisonsSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    if (p == m_pMaxZones)
    {
        char buf[64];
        m_pMaxZones->GetText(buf, 64);
        int input = Q_atoi(buf);
        if (input > 0 && input < 11)
        {
            ConVarRef("mom_comparisons_max_zones").SetValue(input);
        }
    }
}

int ComparisonsSettingsPage::DetermineBogusPulse(Panel *panel) const
{
    int bogusPulse = 0;
    if (panel == m_pJumpShow)
    {
        bogusPulse |= ZONE_JUMPS;
    }
    else if (panel == m_pStrafeShow)
    {
        bogusPulse |= ZONE_STRAFES;
    }
    else if (panel == m_pSyncShow)
    {
        bogusPulse |= ZONE_SYNC1;
        bogusPulse |= ZONE_SYNC2;
    }
    else if (panel == m_pSyncShowS1)
    {
        bogusPulse |= ZONE_SYNC1;
    }
    else if (panel == m_pSyncShowS2)
    {
        bogusPulse |= ZONE_SYNC2;
    }
    else if (panel == m_pVelocityShow)
    {
        bogusPulse |= VELOCITY_AVERAGE;
        bogusPulse |= VELOCITY_MAX;
        bogusPulse |= VELOCITY_ENTER;
        bogusPulse |= VELOCITY_EXIT;
    }
    else if (panel == m_pVelocityShowAvg)
    {
        bogusPulse |= VELOCITY_AVERAGE;
    }
    else if (panel == m_pVelocityShowMax)
    {
        bogusPulse |= VELOCITY_MAX;
    }
    else if (panel == m_pVelocityShowEnter)
    {
        bogusPulse |= VELOCITY_ENTER;
    }
    else if (panel == m_pVelocityShowExit)
    {
        bogusPulse |= VELOCITY_EXIT;
    }
    else if (panel == m_pTimeShowOverall)
    {
        bogusPulse |= TIME_OVERALL;
    }
    else if (panel == m_pTimeShowZone)
    {
        bogusPulse |= ZONE_TIME;
    }
    else if (panel == m_pMaxZones || panel == m_pMaxZonesLabel)
    {
        bogusPulse |= ZONE_LABELS;
    }
    else if (panel == m_pTimeType || panel == m_pTimeTypeLabel)
    {
        bogusPulse |= ZONE_LABELS_COMP;
    }
    else if (panel == m_pCompareFormat)
    {
        //Just fade everything since formatting affects them all
        bogusPulse |= ZONE_TIME;
        bogusPulse |= TIME_OVERALL;
        bogusPulse |= VELOCITY_AVERAGE;
        bogusPulse |= VELOCITY_MAX;
        bogusPulse |= VELOCITY_ENTER;
        bogusPulse |= VELOCITY_EXIT;
        bogusPulse |= ZONE_SYNC1;
        bogusPulse |= ZONE_SYNC2;
        bogusPulse |= ZONE_JUMPS;
        bogusPulse |= ZONE_STRAFES;
    }

    return bogusPulse;
}

void ComparisonsSettingsPage::CursorEnteredCallback(vgui::Panel *panel)
{
    int bogusPulse = DetermineBogusPulse(panel);

    m_pBogusComparisonsPanel->SetBogusPulse(bogusPulse);
    if (bogusPulse > 0)
    {
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_pComparisonsFrame,
                                                                                "PulseComparePanel");
    }
}

void ComparisonsSettingsPage::CursorExitedCallback(vgui::Panel *panel)
{
    if (DetermineBogusPulse(panel) > 0)
    {
        m_pBogusComparisonsPanel->ClearBogusPulse();
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_pComparisonsFrame,
                                                                                "StopPulseComparePanel");
    }
}

void ComparisonsSettingsPage::OnComparisonResize(int wide, int tall)
{
    int scaledPad = scheme()->GetProportionalScaledValue(15);
    m_pComparisonsFrame->SetSize(wide + scaledPad, tall + float(scaledPad) * 1.5f);
    m_pBogusComparisonsPanel->SetPos(m_pComparisonsFrame->GetXPos() + scaledPad / 2,
                                     m_pComparisonsFrame->GetYPos() + scaledPad);
}