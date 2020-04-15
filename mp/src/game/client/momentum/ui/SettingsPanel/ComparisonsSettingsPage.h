#pragma once

#include "SettingsPage.h"

class C_RunComparisons;

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

    void OnScreenSizeChanged(int oldwide, int oldtall) override;

    //Load the settings for this panel
    void OnPageShow() OVERRIDE;

    //Overridden from PropertyPage so we can hide the comparisons frame
    void OnPageHide() OVERRIDE;

    // This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;

    MESSAGE_FUNC_PTR(CursorEnteredCallback, "OnCursorEntered", panel);
    MESSAGE_FUNC_PTR(CursorExitedCallback, "OnCursorExited", panel);
    MESSAGE_FUNC_INT_INT(OnComparisonResize, "OnSizeChange", wide, tall);


private:

    vgui::CvarToggleCheckButton *m_pCompareShow, *m_pCompareFormat, *m_pTimeShowOverall,
        *m_pTimeShowZone, *m_pVelocityShow, *m_pVelocityShowAvg, *m_pVelocityShowMax, *m_pVelocityShowEnter, 
        *m_pVelocityShowExit, *m_pSyncShow, *m_pSyncShowS1, *m_pSyncShowS2, *m_pJumpShow, *m_pStrafeShow;
    vgui::CvarTextEntry *m_pMaxZones;
    vgui::ComboBox *m_pTimeType;
    vgui::Label *m_pTimeTypeLabel, *m_pMaxZonesLabel;
    vgui::Frame *m_pComparisonsFrame;
    C_RunComparisons *m_pBogusComparisonsPanel;

    bool m_bComparisonsFrameIsFadingOut;

    //Determines what should pulse for the bogus panel
    int DetermineBogusPulse(Panel *panel) const;
};
