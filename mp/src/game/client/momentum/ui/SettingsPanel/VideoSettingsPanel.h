#pragma once

#include "SettingsPanel.h"

class VideoSettingsPanel : public SettingsPanel
{
    DECLARE_CLASS_SIMPLE(VideoSettingsPanel, SettingsPanel);

    VideoSettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;

    void OnTextChanged(Panel *panel, const char *text) override;

private:
    void SetCurrentResolutionComboItem();
    void PrepareResolutionList();

    MESSAGE_FUNC(OnDataChanged, "ControlModified");

    int m_nSelectedMode; // -1 if we are running in a nonstandard mode

    // Main video settings
    vgui::ComboBox *m_pMode;
    vgui::ComboBox *m_pWindowed;
    vgui::ComboBox *m_pAspectRatio;

    // Advanced dialog
    void ApplyAdvancedChanges();

    void SetComboItemAsRecommended(vgui::ComboBox *combo, int iItem);
    int FindMSAAMode(int nAASamples, int nAAQuality);
    void MarkDefaultSettingsAsRecommended();


    vgui::ComboBox *m_pModelDetail, *m_pTextureDetail, *m_pAntialiasingMode, *m_pFilteringMode;
    vgui::ComboBox *m_pShadowDetail, *m_pWaterDetail, *m_pVSync, *m_pShaderDetail;
    vgui::ComboBox *m_pColorCorrection;
    vgui::ComboBox *m_pMotionBlur;

    vgui::ComboBox *m_pMulticore;

    vgui::CvarSlider *m_pFOVSlider;
    vgui::CvarTextEntry *m_pFOVEntry;

    int m_nNumAAModes;
    struct AAMode_t
    {
        int m_nNumSamples;
        int m_nQualityLevel;
    };
    AAMode_t m_nAAModes[16];

    vgui::ComboBox *m_pTonemap, *m_pBloom;

    ConVarRef m_cvarPicmip, m_cvarForceaniso, m_cvarTrilinear, m_cvarAntialias, m_cvarAAQuality,
        m_cvarShadowRenderToTexture, m_cvarFlashlightDepthTexture, m_cvarWaterForceExpensive,
        m_cvarWaterForceReflectEntities, m_cvarVSync, m_cvarRootlod, m_cvarReduceFillrate, m_cvarColorCorreciton,
        m_cvarMotionBlur, m_cvarQueueMode, m_cvarDisableBloom, m_cvarDynamicTonemapping;

    // Gamma section
    vgui::CvarSlider *m_pGammaSlider;
    vgui::CvarTextEntry *m_pGammaEntry;
    float m_flOriginalGamma;

    ConVarRef m_cvarGamma;
};