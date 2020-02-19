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

    void SetButtonColors();

    // The "bogus" panel is a HUD crosshair preview panel initted just for this Settings Page.
    void DestroyBogusCrosshairPanel();
    void InitBogusCrosshairPanel();

    // These are used for closing/activating the bogus panel if this was the tab
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;

    // Handle custom controls
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);
    void OnApplyChanges() OVERRIDE;
    void OnClose() OVERRIDE;
    void OnCommand(const char *command) OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnScreenSizeChanged(int oldwide, int oldtall) override;

    // Load the settings for this panel
    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;

    // Overridden from PropertyPage so we can hide the comparisons frame
    void OnPageHide() OVERRIDE;

    // This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;
    
    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

    MESSAGE_FUNC_INT_INT(OnCrosshairPreviewResize, "OnSizeChange", wide, tall);

private:
    void UpdateSliderEntries() const;
    void UpdateStyleToggles() const;

    vgui::CvarToggleCheckButton *m_pCrosshairShow, *m_pCrosshairDot, *m_pCrosshairAlphaEnable, *m_pDynamicFire,
        *m_pDynamicMove, *m_pCrosshairDrawT, *m_pWeaponGap, *m_pOutlineEnable, *m_pScaleEnable;
    vgui::CvarSlider *m_pOutlineThicknessSlider, *m_pCrosshairThicknessSlider, *m_pCrosshairScaleSlider,
        *m_pCrosshairSizeSlider, *m_pCrosshairGapSlider;
    vgui::TextEntry *m_pCustomFileEntry, *m_pOutlineThicknessEntry, *m_pCrosshairThicknessEntry,
        *m_pCrosshairScaleEntry, *m_pCrosshairSizeEntry, *m_pCrosshairGapEntry;
    vgui::ComboBox *m_pCrosshairStyle;
    vgui::ColorPicker *m_pCrosshairColorPicker;
    vgui::Button *m_pCrosshairColorButton;
    vgui::Frame *m_pCrosshairPreviewFrame;
    C_CrosshairPreview *m_pCrosshairPreviewPanel;

    int m_iPrevCrosshair, m_iPrevAlphaEnable, m_iPrevDot, m_iPrevDynamicFire, m_iPrevDynamicMove, m_iPrevGap,
		m_iPrevWeaponGap, m_iPrevOutlineEnable, m_iPrevOutlineThickness, m_iPrevScale, m_iPrevScaleEnable,
		m_iPrevSize, m_iPrevStyle, m_iPrevT, m_iPrevThickness;
    char *m_sPrevFile;
    Color m_prevColor, m_currentColor;
};
