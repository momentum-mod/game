#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "hud_comparisons.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/pch_vgui_controls.h>
#include <vgui_controls/AnimationController.h>

using namespace vgui;

class ComparisonsSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ComparisonsSettingsPage, SettingsPage);

    ComparisonsSettingsPage(Panel *pParent);

    ~ComparisonsSettingsPage();

    //The "bogus" panel is a HUD comparisons panel initted just for this Settings Page.
    void DestroyBogusComparePanel();
    void InitBogusComparePanel();

    //These are used for closing/activating the bogus panel if this was the tab
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;

    //Handle custom controls
    void OnApplyChanges() OVERRIDE;

    //Load the settings for this panel
    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;

    //Overridden from PropertyPage so we can hide the comparisons frame
    void OnPageHide() OVERRIDE;

    // This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;
    // Used for updating the max stage buffer label
    void OnTextChanged(Panel *p) OVERRIDE;

    MESSAGE_FUNC_PTR(CursorEnteredCallback, "OnCursorEntered", panel)
    {
        int bogusPulse = DetermineBogusPulse(panel);

        m_pBogusComparisonsPanel->SetBogusPulse(bogusPulse);
        if (bogusPulse > 0)
        {
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_pComparisonsFrame, "PulseComparePanel");
        }
    }

    MESSAGE_FUNC_PTR(CursorExitedCallback, "OnCursorExited", panel)
    {
        if (DetermineBogusPulse(panel) > 0)
        {
            m_pBogusComparisonsPanel->ClearBogusPulse();
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_pComparisonsFrame, "StopPulseComparePanel");
        }
    }

    MESSAGE_FUNC_INT_INT(OnComparisonResize, "OnSizeChange", wide, tall)
    {
        int scaledPad = scheme()->GetProportionalScaledValue(15);
        m_pComparisonsFrame->SetSize(wide + scaledPad, tall + float(scaledPad) * 1.5f);
        m_pBogusComparisonsPanel->SetPos(m_pComparisonsFrame->GetXPos() + scaledPad/2, m_pComparisonsFrame->GetYPos() + scaledPad);
    }

private:

    CvarToggleCheckButton *m_pCompareShow, *m_pCompareFormat, *m_pTimeShowOverall,
        *m_pTimeShowZone, *m_pVelocityShow, *m_pVelocityShowAvg, *m_pVelocityShowMax, *m_pVelocityShowEnter, 
        *m_pVelocityShowExit, *m_pSyncShow, *m_pSyncShowS1, *m_pSyncShowS2, *m_pJumpShow, *m_pStrafeShow;
    TextEntry *m_pMaxZones;
    ComboBox *m_pTimeType;
    Label *m_pTimeTypeLabel, *m_pMaxZonesLabel;
    Frame *m_pComparisonsFrame;
    C_RunComparisons *m_pBogusComparisonsPanel;

    //Determines what should pulse for the bogus panel
    int DetermineBogusPulse(Panel *panel) const;
};
