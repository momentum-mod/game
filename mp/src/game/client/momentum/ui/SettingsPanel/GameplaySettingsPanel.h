#pragma once

#include "SettingsPanel.h"

namespace vgui
{
    class ColorPicker;
}
class PaintPreviewPanel;

class GameplaySettingsPanel : public SettingsPanel
{
public:
    DECLARE_CLASS_SIMPLE(GameplaySettingsPanel, SettingsPanel);

    GameplaySettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;

    void OnCheckboxChecked(Panel *panel) override;

protected:
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);
    void OnCommand(const char *command) override;
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

private:
    // General gameplay settings
    vgui::CvarToggleCheckButton *m_pEnableDevConsole;
    vgui::CvarToggleCheckButton *m_pHudFastSwitch;
    vgui::CvarToggleCheckButton *m_pOverlappingKeys;
    vgui::CvarToggleCheckButton *m_pReleaseForwardOnJump;
    vgui::CvarToggleCheckButton *m_pDrawViewmodel;

    vgui::CvarToggleCheckButton *m_pPlayBlockSound;
    vgui::CvarToggleCheckButton *m_pSaveCheckpoints;
    vgui::CvarSlider *m_pYawSpeedSlider;
    vgui::CvarTextEntry *m_pYawSpeedEntry;

    // Paint
    vgui::CvarSlider *m_pPaintDecalScaleSlider;
    vgui::CvarTextEntry *m_pPaintDecalScaleEntry;

    vgui::CvarToggleCheckButton *m_pTogglePaintApplySound;
    vgui::CvarToggleCheckButton *m_pTogglePaintLimitToWorld;

    vgui::ColorPicker *m_pPaintColorPicker;
    vgui::Button *m_pPickPaintColorButton;

    vgui::DHANDLE<PaintPreviewPanel> m_pPaintPreviewPanel;

    // Run safeguards
    vgui::CvarComboBox *m_pPracticeModeSafeguards;
    vgui::CvarComboBox *m_pRestartMapSafeguards;
    vgui::CvarComboBox *m_pSavelocTeleSafeguards;
    vgui::CvarComboBox *m_pChatOpenSafeguards;
    vgui::CvarComboBox *m_pRestartStageSafeguards;

    // Gamemode
    vgui::CvarToggleCheckButton *m_pRJEnableTrailParticle, *m_pRJEnableExplosionParticle, *m_pRJEnableShootSound,
        *m_pRJToggleRocketTrailSound, *m_pRJToggleRocketExplosionSound, *m_pRJToggleRocketDecals,
        *m_pRJEnableCenterFire;

    vgui::CvarTextEntry *m_pRJRocketDrawDelayEntry;

    vgui::CvarToggleCheckButton *m_pSJEnableTrailParticle, *m_pSJEnableExplosionParticle, *m_pSJToggleStickybombDecals,
                                *m_pSJEnableExplosionSound, *m_pSJEnableDetonateSuccessSound, *m_pSJEnableDetonateFailSound,
                                *m_pSJEnableChargeSound, *m_pSJEnableShootSound, *m_pSJEnableChargeMeter, *m_pSJEnableStickyCounter, *m_pSJStickyCounterAutohide;

    vgui::CvarComboBox *m_pSJChargeMeterUnits;

    vgui::CvarTextEntry *m_pSJStickyDrawDelayEntry, *m_pSJChargedShotSoundThreshold;

    // paint cvars
    ConVarRef m_cvarPaintColor;
};
