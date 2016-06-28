#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/pch_vgui_controls.h>

using namespace vgui;

class ComparisonsSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ComparisonsSettingsPage, SettingsPage);

    ComparisonsSettingsPage(Panel *pParent) : BaseClass(pParent, "ComparisonsSettings")
    {
        m_pCompareShow = FindControl<CvarToggleCheckButton<ConVarRef>>("CompareShow");
        m_pCompareShow->AddActionSignalTarget(this);

        m_pMaxZones = FindControl<TextEntry>("Zones");
        m_pMaxZones->AddActionSignalTarget(this);

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

        LoadSettings();
    }

    ~ComparisonsSettingsPage() {}

    void OnApplyChanges() override
    {
        BaseClass::OnApplyChanges();

        if (m_pMaxZones)
        {
            ConVarRef stages("mom_comparisons_max_stages");
            char buf[64];
            m_pMaxZones->GetText(buf, sizeof(buf));
            int stagesNum = atoi(buf);
            //MOM_TODO: This needs changing if mom_comparisons_max_stages has a new caps
            if (stagesNum > 0 && stagesNum < 65)
            {
                stages.SetValue(stagesNum);
            }
        }

        if (m_pTimeType)
        {
            ConVarRef timeType("mom_comparisons_time_type");
            timeType.SetValue(m_pTimeType->GetActiveItem());
        }
    }

    void LoadSettings() override
    {
        if (m_pMaxZones)
        {
            ConVarRef stages("mom_comparisons_max_stages");
            m_pMaxZones->SetText(stages.GetString());
        }

        if (m_pTimeType)
        {
            ConVarRef timeType("mom_comparisons_time_type");
            m_pTimeType->ActivateItemByRow(timeType.GetInt());
        }
    }

    //This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) override
    {
        BaseClass::OnCheckboxChecked(p);

        if (p == m_pCompareShow)
        {
            //Turn everything on/off
            bool bEnabled = m_pCompareShow->IsSelected();

            //MOM_TODO: Turn everything off
            m_pMaxZones->SetEnabled(bEnabled);
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

    void OnTextChanged(Panel *p) override
    {
        BaseClass::OnTextChanged(p);

        if (p == m_pMaxZones)
        {
            ConVarRef maxStages("mom_comparisons_max_stages");
            char buf[64];
            m_pMaxZones->GetText(buf, 64);
            int input = Q_atoi(buf);
            //MOM_TODO: If the max stages clamp changes, this needs to as well!
            if (input > 0 && input < 65)
            {
                maxStages.SetValue(input);
            }
            else // Out of range, 
            {
                const char *pDefault = maxStages.GetDefault();
                maxStages.SetValue(pDefault);
                m_pMaxZones->SetText(pDefault);
            }
        }
    }

private:

    CvarToggleCheckButton<ConVarRef> *m_pCompareShow, *m_pCompareFormat, *m_pTimeShowOverall,
        *m_pTimeShowZone, *m_pVelocityShow, *m_pVelocityShowAvg, *m_pVelocityShowMax, *m_pVelocityShowEnter, 
        *m_pVelocityShowExit, *m_pSyncShow, *m_pSyncShowS1, *m_pSyncShowS2, *m_pJumpShow, *m_pStrafeShow;
    TextEntry *m_pMaxZones;
    ComboBox *m_pTimeType;
    Label *m_pTimeTypeLabel;
};