#pragma once

#include "hud_speedometer_label.h"
#include "c_mom_replay_entity.h"
#include "hudelement.h"
#include "mom_player_shared.h"

#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_horiz, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the horizontal speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_vert, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the vertical speedometer. 0 = OFF, 1 = ON\n");

// 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
#define UPS_TO_KMH_FACTOR 0.06858f
// 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
#define UPS_TO_MPH_FACTOR 0.04262f

// Macro to create a speedometer label that is based on axis only (absolute, horiz only, vert only, etc.)
#define MAKE_AXIS_SPEEDOMETER(name, labelVarName, labelName, cvar, Xaxis, Yaxis, Zaxis)                                \
    static bool name(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext)                       \
    {                                                                                                                  \
        Vector vecVelocity = pPlayer->GetAbsVelocity();                                                                \
        if (!Xaxis) vecVelocity.x = 0;                                                                                 \
        if (!Yaxis) vecVelocity.y = 0;                                                                                 \
        if (!Zaxis) vecVelocity.z = 0;                                                                                 \
        *pVelocity = static_cast<float>(vecVelocity.Length());                                                         \
        if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)                    \
        {                                                                                                              \
            static ConVarRef CvarTimeScale("mom_replay_timescale");                                                    \
            float timescale = CvarTimeScale.GetFloat();                                                                \
            if (timescale < 1.0f)                                                                                      \
            {                                                                                                          \
                *pVelocity /= timescale;                                                                               \
            }                                                                                                          \
        }                                                                                                              \
        AdjustToUnits(pVelocity, pPlayer);                                                                             \
        return true;                                                                                                   \
    }                                                                                                                  \
    SpeedometerLabel* labelVarName = new SpeedometerLabel(this, labelName, &cvar, SPEEDOMETER_LABEL_UPDATE_ALWAYS, name)

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
    SpeedometerLabel *m_pLastJumpVelLabel, *m_pStageEnterLabel, *m_pStageEnterComparisonLabel;
    vgui::Label *m_pUnitsLabel;

    int m_defaultAbsSpeedoLabelHeight, m_defaultHorizSpeedoLabelHeight, m_defaultVertSpeedoLabelHeight, m_defaultLastJumpVelLabelHeight;

    MAKE_AXIS_SPEEDOMETER(GetAbsVelocity, m_pAbsSpeedoLabel, "AbsSpeedoLabel", mom_hud_speedometer, true, true, true);
    MAKE_AXIS_SPEEDOMETER(GetHorizVelocity, m_pHorizSpeedoLabel, "HorizSpeedoLabel", mom_hud_speedometer_horiz, true, true, false);
    MAKE_AXIS_SPEEDOMETER(GetVertVelocity, m_pVertSpeedoLabel, "VertSpeedoLabel", mom_hud_speedometer_vert, false, false, true);

    KeyValues *m_pGamemodeSetupData;

    static bool GetLastJumpVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext);
    static void LastJumpVelColorizeOverride(Color &currentColor, float currentVel, float lastVel,
                                            Color normalColor, Color increaseColor, Color decreaseColor);

    // helper function
    static void AdjustToUnits(float *vel, C_MomentumPlayer *pPlayer);

  protected:
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, m_flStageStartAlpha, "StageAlpha", "0.0f");
    CPanelAnimationVar(float, m_flLastJumpVelAlpha, "JumpAlpha", "0.0f");
};
