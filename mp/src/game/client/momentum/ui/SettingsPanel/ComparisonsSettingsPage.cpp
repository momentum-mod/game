#include "cbase.h"

#include "ComparisonsSettingsPage.h"
#include <ienginevgui.h>

ComparisonsSettingsPage::ComparisonsSettingsPage(Panel *pParent) : BaseClass(pParent, "ComparisonsSettings")
{
    m_pCompareShow = FindControl<CvarToggleCheckButton<ConVarRef>>("CompareShow");
    m_pCompareShow->AddActionSignalTarget(this);

    m_pMaxZones = FindControl<TextEntry>("Zones");
    m_pMaxZones->AddActionSignalTarget(this);

    m_pMaxZonesLabel = FindControl<Label>("ZonesLabel");

    m_pCompareFormat = FindControl<CvarToggleCheckButton<ConVarRef>>("CompareFormat");
    m_pCompareFormat->AddActionSignalTarget(this);

    m_pTimeTypeLabel = FindControl<Label>("TimeTypeLabel");
    m_pTimeTypeLabel->GetTooltip()->SetTooltipFormatToSingleLine();

    m_pTimeType = FindControl<ComboBox>("TimeType");
    m_pTimeType->SetNumberOfEditLines(2);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_Overall", nullptr);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_PerZone", nullptr);
    m_pTimeType->AddActionSignalTarget(this);

    m_pTimeShowOverall = FindControl<CvarToggleCheckButton<ConVarRef>>("TimeShowOverall");
    m_pTimeShowOverall->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowOverall->AddActionSignalTarget(this);

    m_pTimeShowZone = FindControl<CvarToggleCheckButton<ConVarRef>>("TimeShowZone");
    m_pTimeShowZone->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowZone->AddActionSignalTarget(this);

    m_pVelocityShow = FindControl<CvarToggleCheckButton<ConVarRef>>("VelShow");
    m_pVelocityShow->AddActionSignalTarget(this);

    m_pVelocityShowAvg = FindControl<CvarToggleCheckButton<ConVarRef>>("VelShowAvg");
    m_pVelocityShowAvg->AddActionSignalTarget(this);

    m_pVelocityShowMax = FindControl<CvarToggleCheckButton<ConVarRef>>("VelShowMax");
    m_pVelocityShowMax->AddActionSignalTarget(this);

    m_pVelocityShowEnter = FindControl<CvarToggleCheckButton<ConVarRef>>("VelShowEnter");
    m_pVelocityShowEnter->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowEnter->AddActionSignalTarget(this);

    m_pVelocityShowExit = FindControl<CvarToggleCheckButton<ConVarRef>>("VelShowExit");
    m_pVelocityShowExit->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowExit->AddActionSignalTarget(this);

    m_pSyncShow = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShow");
    m_pSyncShow->AddActionSignalTarget(this);

    m_pSyncShowS1 = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShowS1");
    m_pSyncShowS1->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShow->AddActionSignalTarget(this);

    m_pSyncShowS2 = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShowS2");
    m_pSyncShowS2->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS2->AddActionSignalTarget(this);

    m_pJumpShow = FindControl<CvarToggleCheckButton<ConVarRef>>("ShowJumps");
    m_pJumpShow->AddActionSignalTarget(this);

    m_pStrafeShow = FindControl<CvarToggleCheckButton<ConVarRef>>("ShowStrafes");
    m_pStrafeShow->AddActionSignalTarget(this);

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

void ComparisonsSettingsPage::OnMainDialogClosed() const
{
    if (m_pComparisonsFrame)
        m_pComparisonsFrame->Close();
}

void ComparisonsSettingsPage::OnMainDialogShow() const
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
        m_pVelocityShowAvg->SetEnabled(bEnabled);
        m_pVelocityShowEnter->SetEnabled(bEnabled);
        m_pVelocityShowMax->SetEnabled(bEnabled);
    }

    else if (p == m_pSyncShow)
    {
        bool bEnabled = m_pSyncShow->IsSelected();

        m_pSyncShowS1->SetEnabled(bEnabled);
        m_pSyncShowS2->SetEnabled(bEnabled);
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