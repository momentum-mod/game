#include "cbase.h"

#include "ComparisonsSettingsPage.h"
#include <ienginevgui.h>
#include "clientmode.h"
#include "hud_comparisons.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/CvarComboBox.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/AnimationController.h>

#include <tier0/memdbgon.h>

using namespace vgui;

ComparisonsSettingsPage::ComparisonsSettingsPage(Panel *pParent)
    : BaseClass(pParent, "ComparisonsSettings"), m_bComparisonsFrameIsFadingOut(false)
{
    m_pCompareShow = new CvarToggleCheckButton(this, "CompareShow", "#MOM_Settings_Compare_Show", "mom_comparisons");
    m_pCompareShow->AddActionSignalTarget(this);
    m_pMaxZones = new CvarTextEntry(this, "Zones", "mom_comparisons_max_zones");
    m_pMaxZones->SetAllowNumericInputOnly(true);
    m_pMaxZones->AddActionSignalTarget(this);
    m_pMaxZonesLabel = new Label(this, "ZonesLabel", "#MOM_Settings_Zones_Label");

    m_pCompareFormat = new CvarToggleCheckButton(this, "CompareFormat", "#MOM_Settings_Compare_Format", "mom_comparisons_format_output");
    m_pCompareFormat->AddActionSignalTarget(this);

    m_pTimeTypeLabel = new Label(this, "TimeTypeLabel", "#MOM_Settings_Compare_Time_Type_Label");
    m_pTimeTypeLabel->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeType = new CvarComboBox(this, "TimeType", 2, false, "mom_comparisons_time_type");
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_Overall", nullptr);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_PerZone", nullptr);
    m_pTimeType->AddActionSignalTarget(this);

    m_pTimeShowOverall = new CvarToggleCheckButton(this, "TimeShowOverall", "#MOM_Settings_Compare_Show_Overall", "mom_comparisons_time_show_overall");
    m_pTimeShowOverall->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowOverall->AddActionSignalTarget(this);
    m_pTimeShowZone = new CvarToggleCheckButton(this, "TimeShowZone", "#MOM_Settings_Compare_Show_Zone", "mom_comparisons_time_show_perzone");
    m_pTimeShowZone->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowZone->AddActionSignalTarget(this);

    m_pVelocityShow = new CvarToggleCheckButton(this, "VelShow", "#MOM_Settings_Compare_Show_Velocity", "mom_comparisons_vel_show");
    m_pVelocityShow->AddActionSignalTarget(this);
    m_pVelocityShowAvg = new CvarToggleCheckButton(this, "VelShowAvg", "#MOM_Settings_Compare_Show_Velocity_Avg", "mom_comparisons_vel_show_avg");
    m_pVelocityShowAvg->AddActionSignalTarget(this);
    m_pVelocityShowMax = new CvarToggleCheckButton(this, "VelShowMax", "#MOM_Settings_Compare_Show_Velocity_Max", "mom_comparisons_vel_show_max");
    m_pVelocityShowMax->AddActionSignalTarget(this);
    m_pVelocityShowEnter = new CvarToggleCheckButton(this, "VelShowEnter", "#MOM_Settings_Compare_Show_Velocity_Enter", "mom_comparisons_vel_show_enter");
    m_pVelocityShowEnter->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowEnter->AddActionSignalTarget(this);
    m_pVelocityShowExit = new CvarToggleCheckButton(this, "VelShowExit", "#MOM_Settings_Compare_Show_Velocity_Exit", "mom_comparisons_vel_show_exit");
    m_pVelocityShowExit->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowExit->AddActionSignalTarget(this);

    m_pSyncShow = new CvarToggleCheckButton(this, "SyncShow", "#MOM_Settings_Compare_Show_Sync", "mom_comparisons_sync_show");
    m_pSyncShow->AddActionSignalTarget(this);
    m_pSyncShowS1 = new CvarToggleCheckButton(this, "SyncShowS1", "#MOM_Settings_Compare_Show_Sync1", "mom_comparisons_sync_show_sync1");
    m_pSyncShowS1->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS1->AddActionSignalTarget(this);
    m_pSyncShowS2 = new CvarToggleCheckButton(this, "SyncShowS2", "#MOM_Settings_Compare_Show_Sync2", "mom_comparisons_sync_show_sync2");
    m_pSyncShowS2->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS2->AddActionSignalTarget(this);

    m_pJumpShow = new CvarToggleCheckButton(this, "ShowJumps", "#MOM_Settings_Compare_Show_Jumps", "mom_comparisons_jumps_show");
    m_pJumpShow->AddActionSignalTarget(this);
    m_pStrafeShow = new CvarToggleCheckButton(this, "ShowStrafes", "#MOM_Settings_Compare_Show_Strafes", "mom_comparisons_strafe_show");
    m_pStrafeShow->AddActionSignalTarget(this);

    m_pComparisonsFrame = nullptr;
    m_pBogusComparisonsPanel = nullptr;

    LoadControlSettings("resource/ui/SettingsPanel_ComparisonsSettings.res");
}

ComparisonsSettingsPage::~ComparisonsSettingsPage()
{
}

void ComparisonsSettingsPage::DestroyBogusComparePanel()
{
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
    m_pComparisonsFrame->SetCloseButtonVisible(false);
    m_pComparisonsFrame->SetMinimizeButtonVisible(false);
    m_pComparisonsFrame->SetMaximizeButtonVisible(false);
    m_pComparisonsFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);
    m_pComparisonsFrame->InvalidateLayout(true);

    //Initialize a bogus version of the HUD element
    m_pBogusComparisonsPanel = new C_RunComparisons("BogusComparisonsPanel", m_pComparisonsFrame);
    m_pBogusComparisonsPanel->AddActionSignalTarget(this);
    m_pBogusComparisonsPanel->SetPaintBackgroundEnabled(true);
    m_pBogusComparisonsPanel->SetPaintBackgroundType(2);
    m_pBogusComparisonsPanel->Init();
    m_pBogusComparisonsPanel->Reset();
    int x, y, wid, tal;
    m_pComparisonsFrame->GetClientArea(x, y, wid, tal);
    m_pBogusComparisonsPanel->SetBounds(x, y, wid, tal);
    m_pBogusComparisonsPanel->LoadBogusComparisons();
    m_pBogusComparisonsPanel->SetScheme(scheme()->GetScheme("ClientScheme"));
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

void ComparisonsSettingsPage::OnScreenSizeChanged(int oldwide, int oldtall)
{
    BaseClass::OnScreenSizeChanged(oldwide, oldtall);

    DestroyBogusComparePanel();
}

void ComparisonsSettingsPage::OnPageShow()
{
    BaseClass::OnPageShow();

    if (!m_pComparisonsFrame)
        InitBogusComparePanel();
    else if (!m_pComparisonsFrame->IsVisible() || m_bComparisonsFrameIsFadingOut)
    {
        m_pComparisonsFrame->Activate();
        m_bComparisonsFrameIsFadingOut = false;
    }
}

void ComparisonsSettingsPage::OnPageHide()
{
    if (m_pComparisonsFrame)
    {
        m_pComparisonsFrame->Close();
        m_bComparisonsFrameIsFadingOut = true;
    }
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