#pragma once

#include "c_mom_replay_entity.h"
#include "hudelement.h"

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
    void OnThink() OVERRIDE;

  private:
    void SetLabelHeight(bool isActive, vgui::Label *label, int height);
    void SetSpeedometerLabel(bool isActive, vgui::Label *label, int height, int roundedSpeed, Color color,
                             float alpha = -1.0f);
    void ColorBasedOnAccel(Color &color, int currentVel, int lastVel);
    void ColorRelativeToMax(Color &color, int vel);

    float m_flNextColorizeCheck, m_flLastVelocity, m_flLastHVelocity, m_flLastJumpVelocity;

    bool m_bRanFadeOutJumpSpeed;

    Color m_LastColor, m_hLastColor, m_CurrentColor, m_hCurrentColor, m_LastJumpVelColor, m_NormalColor,
        m_IncreaseColor, m_DecreaseColor;

    Color m_MaxVelColorLevel1, m_MaxVelColorLevel2, m_MaxVelColorLevel3, m_MaxVelColorLevel4, m_MaxVelColorLevel5;

    vgui::Label *m_pUnitsLabel, *m_pAbsSpeedoLabel, *m_pHorizSpeedoLabel, *m_pLastJumpVelLabel, *m_pStageEnterExitLabel,
        *m_pStageEnterExitComparisonLabel;

    int m_defaultUnitsLabelHeight, m_defaultAbsSpeedoLabelHeight, m_defaultHorizSpeedoLabelHeight,
        m_defaultLastJumpVelLabelHeight, m_defaultStageEnterExitLabelHeight, m_defaultYPos;

    CMomRunStats *m_pRunStats;
    CMomRunEntityData *m_pRunEntData;

    ConVarRef m_cvarTimeScale;

  protected:
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, m_fStageStartAlpha, "StageAlpha", "0.0"); // Used for fading
    CPanelAnimationVar(float, m_fLastJumpVelAlpha, "JumpAlpha", "0.0");
};