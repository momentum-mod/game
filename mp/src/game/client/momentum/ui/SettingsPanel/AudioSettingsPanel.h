#pragma once

#include "SettingsPanel.h"
#include "language.h"

class AudioSettingsPanel : public SettingsPanel
{
public:
    DECLARE_CLASS_SIMPLE(AudioSettingsPanel, SettingsPanel);

    AudioSettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;
    static const char *GetUpdatedAudioLanguage() { return m_pchUpdatedAudioLanguage; }

    void OnTextChanged(Panel *panel, const char *text) override { OnControlModified(panel); }

private:
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);

    vgui::ComboBox *m_pSpeakerSetupCombo;
    vgui::ComboBox *m_pSoundQualityCombo;
    vgui::CvarSlider *m_pVolumeSlider;
    vgui::CvarTextEntry *m_pVolumeEntry;
    vgui::CvarSlider *m_pMusicSlider;
    vgui::CvarTextEntry *m_pMusicVolumeEntry;
    vgui::ComboBox *m_pCloseCaptionCombo;

    vgui::ComboBox *m_pSpokenLanguageCombo;
    ELanguage m_nCurrentAudioLanguage;
    static const char *m_pchUpdatedAudioLanguage;

    vgui::CvarToggleCheckButton *m_pMuteLoseFocus;

    ConVarRef m_cvarSubtitles, m_cvarCloseCaption, m_cvarSndSurroundSpeakers, m_cvarSndPitchQuality, m_cvarDSPSlowCPU, m_cvarDSPEnhanceStereo;

    // helper functions
    void ResetCloseCaption();
    void ResetSpeakerSetup();
    void ResetSoundQuality();
    void ResetSpokenLanguage();

    void ApplyCloseCaption();
    void ApplySpeakerSetup();
    void ApplySoundQuality();
    void ApplySpokenLanguage();
    void ApplyEnhanceStereo();
};