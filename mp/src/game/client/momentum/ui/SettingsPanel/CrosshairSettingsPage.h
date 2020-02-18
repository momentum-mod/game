#pragma once

#include "SettingsPage.h"

class C_CrosshairPreview;
namespace vgui
{
    class ColorPicker;
}

class CrosshairSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(CrosshairSettingsPage, SettingsPage);

    CrosshairSettingsPage(Panel *pParent);

    ~CrosshairSettingsPage();

    // The "bogus" panel is a HUD comparisons panel initted just for this Settings Page.
    void DestroyBogusCrosshairPanel();
    void InitBogusCrosshairPanel();

    // These are used for closing/activating the bogus panel if this was the tab
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;

    // Handle custom controls
    void OnApplyChanges() OVERRIDE;

    void OnScreenSizeChanged(int oldwide, int oldtall) override;

    // Load the settings for this panel
    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;

    // Overridden from PropertyPage so we can hide the comparisons frame
    void OnPageHide() OVERRIDE;

    // This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;
    // Used for updating the max stage buffer label
    void OnTextChanged(Panel *p) OVERRIDE;

    MESSAGE_FUNC_INT_INT(OnComparisonResize, "OnSizeChange", wide, tall);

  private:
    vgui::CvarToggleCheckButton *m_pCrosshairShow, *mp_CrosshairDot, *m_pAlphaEnable, *m_pDynamicFire, *m_pDynamicMove,
        *m_pDrawT, *m_pWeaponGap, *m_pOutlineEnable, *m_pScaleEnable;
    vgui::CvarSlider *m_pOutlineThicknessSlider, *m_pThicknessSlider, *m_pScaleSlider, *m_pSizeSlider, *m_pGapSlider;
    vgui::TextEntry *m_pCustomFile;
    vgui::ComboBox *m_pCrosshairStyle;
    vgui::ColorPicker *m_pCrosshairColorPicker;
    vgui::Label *m_pTimeTypeLabel, *m_pMaxZonesLabel;
    vgui::Frame *m_pCrosshairPreviewFrame;
    C_CrosshairPreview *m_pCrosshairPreviewPanel;
};
