#pragma once

#include "SettingsPanel.h"

class SpeedometerLabel;
class C_RunComparisons;

class HUDSettingsPanel : public SettingsPanel
{
public:
    DECLARE_CLASS_SIMPLE(HUDSettingsPanel, SettingsPanel);

    HUDSettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;

protected:
    void OnCheckboxChecked(Panel *panel) override;
    void OnTextChanged(Panel *panel, const char *text) override;
    void OnMainDialogClosed() override;

    // Comparisons panel; listening for when we hover a comparison setting for blinking
    MESSAGE_FUNC_PTR(CursorEnteredCallback, "OnCursorEntered", panel);
    MESSAGE_FUNC_PTR(CursorExitedCallback, "OnCursorExited", panel);

private:
    vgui::CvarComboBox *m_pSyncType, *m_pSyncColorize;

    vgui::CvarToggleCheckButton *m_pHUDSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion, *m_pTimerShow, *m_pTimerSoundFailEnable,
                                *m_pTimerSoundStartEnable, *m_pTimerSoundStopEnable, *m_pTimerSoundFinishEnable, *m_pShowMapName, *m_pShowMapAuthor, *m_pShowMapDifficulty, *m_pShowMapStatus;

    // speedo controls
    vgui::ComboBox *m_pSpeedometerGameType, *m_pSpeedometerType, *m_pSpeedometerUnits, *m_pSpeedometerColorize;
    vgui::CheckButton *m_pSpeedometerShow;

    // Comparisons panel
    vgui::CvarToggleCheckButton *m_pCompareShow, *m_pCompareFormat, *m_pTimeShowOverall,
        *m_pTimeShowZone, *m_pVelocityShow, *m_pVelocityShowAvg, *m_pVelocityShowMax, *m_pVelocityShowEnter,
        *m_pVelocityShowExit, *m_pCompareSyncShow, *m_pSyncShowS1, *m_pSyncShowS2, *m_pJumpShow, *m_pStrafeShow;
    vgui::CvarTextEntry *m_pMaxZones;
    vgui::CvarComboBox *m_pTimeType;
    vgui::Label *m_pTimeTypeLabel, *m_pMaxZonesLabel;

    vgui::DHANDLE<C_RunComparisons> m_pBogusComparisonsPanel;

    //Determines what should pulse for the bogus panel
    int DetermineBogusPulse(Panel *panel) const;
};
