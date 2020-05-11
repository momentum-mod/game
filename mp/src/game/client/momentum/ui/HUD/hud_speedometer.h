#pragma once

#include "hud_speedometer_label.h"
#include "c_mom_replay_entity.h"
#include "hudelement.h"
#include "mom_player_shared.h"

#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>

// 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
#define UPS_TO_KMH_FACTOR 0.06858f
// 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
#define UPS_TO_MPH_FACTOR 0.04262f

class CHudSpeedMeter : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, vgui::EditablePanel);

  public:
    CHudSpeedMeter(const char *pElementName);
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void PerformLayout() OVERRIDE;
    void OnThink() OVERRIDE;

    // load/saves gamemode specific speedometer setups
    void LoadGamemodeData();
    void SaveGamemodeData();

  private:
    SpeedometerLabel *m_pAbsSpeedoLabel, *m_pHorizSpeedoLabel, *m_pLastJumpVelLabel, *m_pStageEnterLabel,
        *m_pStageEnterComparisonLabel;
    vgui::Label *m_pUnitsLabel;

    int m_defaultAbsSpeedoLabelHeight, m_defaultHorizSpeedoLabelHeight, m_defaultLastJumpVelLabelHeight;

    KeyValues *m_pGamemodeSetupData;

    static bool GetAbsVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext);
    static bool GetHorizVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext);
    static bool GetLastJumpVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext);
    static void LastJumpVelColorizeOverride(Color &currentColor, float currentVel, float lastVel,
                                            Color normalColor, Color increaseColor, Color decreaseColor);

    // helper functions
    static void AdjustToReplayTimeScale(float *vel);
    static void AdjustToUnits(float *vel, C_MomentumPlayer *pPlayer);

  protected:
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, m_flStageStartAlpha, "StageAlpha", "0.0f");
    CPanelAnimationVar(float, m_flLastJumpVelAlpha, "JumpAlpha", "0.0f");
};
