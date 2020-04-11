#include "cbase.h"

#include "baseviewport.h"
#include "hud_comparisons.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>

#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"
#include "c_mom_replay_entity.h"

#include "c_baseplayer.h"
#include "movevars_shared.h"
#include "run/run_compare.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_CONVAR(mom_hud_speedometer_units, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Changes the units of measurement of the speedometer.\n 1 = Units per second\n 2 = "
                   "Kilometers per hour\n 3 = Miles per hour\n 4 = Energy",
                   1, 4);

static MAKE_CONVAR(mom_hud_speedometer_colorize, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles speedometer colorization. 0 = OFF, 1 = ON (Based on acceleration),"
                   " 2 = ON (Staged by relative velocity to max.)\n", 0, 2);

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_unit_labels, "0", FLAG_HUD_CVAR,
                          "Toggles showing the unit value labels (KM/H, MPH, Energy, UPS). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_showenterspeed, "1", FLAG_HUD_CVAR,
    "Toggles showing the stage/checkpoint enter speed (and comparison, if existent). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_horiz, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_lastjumpvel, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles showing player velocity at last jump (XY only). 0 = OFF, 1 = ON\n");

static MAKE_CONVAR(mom_hud_speedometer_lastjumpvel_fadeout, "3.0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Sets the fade out time for the last jump velocity.", 1.0f, 10.0f);

static MAKE_TOGGLE_CONVAR(mom_hud_velocity_type, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles the velocity type used in comparisons and map finished dialog. 0 = ABSOLUTE, 1 = HORIZONTAL\n");

// 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
#define UPS_TO_KMH_FACTOR 0.06858f
// 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
#define UPS_TO_MPH_FACTOR 0.04262f

class CHudSpeedMeter : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, EditablePanel);

  public:
    CHudSpeedMeter(const char *pElementName);
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;
    void OnThink() OVERRIDE;

  private:
    void SetLabelHeight(bool isActive, Label *label, int height);
    void SetSpeedometerLabel(bool isActive, Label *label, int height, int roundedSpeed, Color color, float alpha = -1.0f);
    void ColorBasedOnAccel(Color &color, int currentVel, int lastVel);
    void ColorRelativeToMax(Color &color, int vel);

    float m_flNextColorizeCheck, m_flLastVelocity, m_flLastHVelocity, m_flLastJumpVelocity;

    bool m_bRanFadeOutJumpSpeed;

    Color m_LastColor, m_hLastColor, m_CurrentColor, m_hCurrentColor, m_LastJumpVelColor, 
        m_NormalColor, m_IncreaseColor, m_DecreaseColor;

    Color m_MaxVelColorLevel1, m_MaxVelColorLevel2, m_MaxVelColorLevel3, m_MaxVelColorLevel4, m_MaxVelColorLevel5;

    Label *m_pUnitsLabel, *m_pAbsSpeedoLabel, *m_pHorizSpeedoLabel, *m_pLastJumpVelLabel, 
        *m_pStageEnterExitLabel, *m_pStageEnterExitComparisonLabel;

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

DECLARE_HUDELEMENT(CHudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudSpeedMeter"), 
    m_cvarTimeScale("mom_replay_timescale")
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
    m_bRanFadeOutJumpSpeed = false;
    m_flNextColorizeCheck = 0;
    m_fStageStartAlpha = 0.0f;

    m_flLastVelocity = 0;
    m_flLastHVelocity = 0;
    m_flLastJumpVelocity = 0;

    m_pUnitsLabel = new Label(this, "UnitsLabel", "");
    m_pAbsSpeedoLabel = new Label(this, "AbsSpeedoLabel", "");
    m_pHorizSpeedoLabel = new Label(this, "HorizSpeedoLabel", "");
    m_pLastJumpVelLabel = new Label(this, "LastJumpVelLabel", "");
    m_pStageEnterExitLabel = new Label(this, "StageEnterExitLabel", "");
    m_pStageEnterExitComparisonLabel = new Label(this, "StageEnterExitComparisonLabel", "");
    
    LoadControlSettings("resource/ui/Speedometer.res");
}

void CHudSpeedMeter::Init()
{
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
    m_flNextColorizeCheck = 0;
    m_fStageStartAlpha = 0.0f;
}

void CHudSpeedMeter::Reset()
{
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
    m_flNextColorizeCheck = 0;
    m_fStageStartAlpha = 0.0f;
    m_pStageEnterExitLabel->SetText("");
    m_pStageEnterExitComparisonLabel->SetText("");
    m_pLastJumpVelLabel->SetText("");
}

void CHudSpeedMeter::FireGameEvent(IGameEvent *pEvent)
{
    C_MomentumPlayer *pLocal = C_MomentumPlayer::GetLocalMomPlayer();

    if (pLocal)
    {
        const auto ent = pEvent->GetInt("ent");
        if (ent == pLocal->GetCurrentUIEntity()->GetEntIndex())
        {
            const auto bExit = FStrEq(pEvent->GetName(), "zone_exit");
            const auto bLinear = pLocal->m_iLinearTracks.Get(pLocal->m_Data.m_iCurrentTrack);

            // Logical XOR; equivalent to (bLinear && !bExit) || (!bLinear && bExit)
            if (bLinear != bExit)
            {
                // Fade the enter speed after 5 seconds (in event)
                m_fStageStartAlpha = 255.0f;
                g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutEnterSpeed");
            }
        }
    }
}

void CHudSpeedMeter::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    m_NormalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
    m_IncreaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
    m_DecreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    m_MaxVelColorLevel1 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level1", pScheme);
    m_MaxVelColorLevel2 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level2", pScheme);
    m_MaxVelColorLevel3 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level3", pScheme);
    m_MaxVelColorLevel4 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level4", pScheme);
    m_MaxVelColorLevel5 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level5", pScheme);

    SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
    SetBgColor(GetSchemeColor("Blank", pScheme));

    m_defaultAbsSpeedoLabelHeight = m_pAbsSpeedoLabel->GetTall();
    m_defaultHorizSpeedoLabelHeight = m_pHorizSpeedoLabel->GetTall();
    m_defaultLastJumpVelLabelHeight = m_pLastJumpVelLabel->GetTall();
    m_defaultUnitsLabelHeight = m_pUnitsLabel->GetTall();
    m_defaultStageEnterExitLabelHeight = m_pStageEnterExitLabel->GetTall();

    m_pStageEnterExitComparisonLabel->SetFont(m_pStageEnterExitLabel->GetFont()); // need to have same font

    m_defaultYPos = GetYPos();
}

void CHudSpeedMeter::OnThink()
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        m_pRunEntData = pPlayer->GetCurrentUIEntData();
        // Note: Velocity is also set to the player when watching first person
        Vector velocity = pPlayer->GetAbsVelocity();
        Vector horizVelocity = Vector(velocity.x, velocity.y, 0);

        if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
        {
            const float fReplayTimeScale = m_cvarTimeScale.GetFloat();
            if (fReplayTimeScale < 1.0f)
            {
                velocity /= fReplayTimeScale;
                horizVelocity /= fReplayTimeScale;
            }
        }

        m_pRunStats = pPlayer->GetCurrentUIEntStats();

        // Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
        float vel = static_cast<float>(velocity.Length());
        float hvel = static_cast<float>(horizVelocity.Length());
        float lastJumpVel = m_pRunEntData->m_flLastJumpVel;

        // reset last jump velocity when we (or a ghost ent) restart a run by entering the start zone
        if (m_pRunEntData->m_bIsInZone && m_pRunEntData->m_iCurrentZone == 1)
        {
            m_flLastJumpVelocity = 0;
        }

        if (gpGlobals->curtime - m_pRunEntData->m_flLastJumpTime > mom_hud_speedometer_lastjumpvel_fadeout.GetFloat())
        {
            if (!m_bRanFadeOutJumpSpeed)
            {
                m_bRanFadeOutJumpSpeed = g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutJumpSpeed");
            }
        }
        else
        {
            // Keep the alpha at full opaque so we can see it
            // MOM_TODO: If we want it to fade back in, we should use an event here
            m_fLastJumpVelAlpha = 255.0f;
            m_bRanFadeOutJumpSpeed = false;
        }

        switch (mom_hud_speedometer_units.GetInt())
        {
        case 2: // KM/H
            vel *= UPS_TO_KMH_FACTOR;
            hvel *= UPS_TO_KMH_FACTOR;
            lastJumpVel *= UPS_TO_KMH_FACTOR;
            m_pUnitsLabel->SetText(L"KM/H");
            break;
        case 3: // MPH
            vel *= UPS_TO_MPH_FACTOR;
            hvel *= UPS_TO_MPH_FACTOR;
            lastJumpVel *= UPS_TO_MPH_FACTOR;
            m_pUnitsLabel->SetText(L"MPH");
            break;
        case 4: // Normalized units of energy
        {
            const auto gravity = sv_gravity.GetFloat();
            vel = (velocity.LengthSqr() / 2.0f + gravity * (pPlayer->GetLocalOrigin().z - m_pRunEntData->m_flLastJumpZPos)) / gravity;
            hvel = vel;
            m_pUnitsLabel->SetText(L"Energy");
            break;
        }
        case 1: // UPS
        default:
            m_pUnitsLabel->SetText(L"UPS");
            break;
        }

        // only called if we need to update color
        if (mom_hud_speedometer_colorize.GetInt())
        {
            // speedometer coloring
            if (m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                switch (mom_hud_speedometer_colorize.GetInt())
                {
                case 1:
                    ColorBasedOnAccel(m_CurrentColor, vel, m_flLastVelocity);
                    ColorBasedOnAccel(m_hCurrentColor, hvel, m_flLastHVelocity);
                    break;
                case 2: 
                    ColorRelativeToMax(m_CurrentColor, vel);
                    ColorRelativeToMax(m_hCurrentColor, hvel);
                    break;
                case 0:
                default:
                    break;
                }
                m_LastColor = m_CurrentColor; 
                m_hLastColor = m_hCurrentColor;
                m_flLastVelocity = vel; m_flLastHVelocity = hvel;
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }

            // last jump vel coloring
            if (CloseEnough(lastJumpVel, 0.0f))
            {
                m_LastJumpVelColor = m_NormalColor;
            }
            else if (m_flLastJumpVelocity != lastJumpVel)
            {
                m_LastJumpVelColor = MomUtil::GetColorFromVariation(fabs(lastJumpVel) - fabs(m_flLastJumpVelocity), 0.0f,
                                                                    m_NormalColor, m_IncreaseColor, m_DecreaseColor);
                m_flLastJumpVelocity = lastJumpVel;
            }
        }
        else
        {
            m_LastJumpVelColor = m_CurrentColor = m_hCurrentColor = m_NormalColor;
        }

        // move panel down based on what is currently active
        int yIndent = 0; 
        yIndent += mom_hud_speedometer.GetBool()                ? 0 : m_defaultAbsSpeedoLabelHeight;
        yIndent += mom_hud_speedometer_horiz.GetBool()          ? 0 : m_defaultHorizSpeedoLabelHeight;
        yIndent += mom_hud_speedometer_lastjumpvel.GetBool()    ? 0 : m_defaultLastJumpVelLabelHeight;
        yIndent += mom_hud_speedometer_unit_labels.GetBool()    ? 0 : m_defaultUnitsLabelHeight;
        yIndent += mom_hud_speedometer_showenterspeed.GetBool() ? 0 : m_defaultStageEnterExitLabelHeight;
        SetPos(GetXPos(), m_defaultYPos + yIndent);

        SetSpeedometerLabel(mom_hud_speedometer.GetBool(), m_pAbsSpeedoLabel, m_defaultAbsSpeedoLabelHeight, RoundFloatToInt(vel), m_CurrentColor);
        SetSpeedometerLabel(mom_hud_speedometer_horiz.GetBool(), m_pHorizSpeedoLabel, m_defaultHorizSpeedoLabelHeight, RoundFloatToInt(hvel), m_hCurrentColor);
        SetSpeedometerLabel(mom_hud_speedometer_lastjumpvel.GetBool(), m_pLastJumpVelLabel, m_defaultLastJumpVelLabelHeight, RoundFloatToInt(lastJumpVel),
                            m_LastJumpVelColor, m_fLastJumpVelAlpha);

        // don't display unit labels if all other speedometers are off
        SetLabelHeight(mom_hud_speedometer_unit_labels.GetBool() &&
                           (mom_hud_speedometer.GetBool() || mom_hud_speedometer_horiz.GetBool() ||
                            mom_hud_speedometer_lastjumpvel.GetBool() || mom_hud_speedometer_showenterspeed.GetBool()),
                       m_pUnitsLabel, m_defaultUnitsLabelHeight);

        const bool isEnterSpeedActive = mom_hud_speedometer_showenterspeed.GetBool();
        SetLabelHeight(isEnterSpeedActive, m_pStageEnterExitLabel, m_defaultStageEnterExitLabelHeight);
        SetLabelHeight(isEnterSpeedActive, m_pStageEnterExitComparisonLabel, m_defaultStageEnterExitLabelHeight);
        if (isEnterSpeedActive && m_pRunEntData && m_pRunEntData->m_bTimerRunning && m_fStageStartAlpha > 0.0f)
        {
            char enterVelStr[BUFSIZELOCL], enterVelComparisonStr[BUFSIZELOCL];

            Color compareColor = Color(0, 0, 0, 0);
            Color enterColor = m_pStageEnterExitLabel->GetFgColor();

            g_pMOMRunCompare->GetComparisonString(VELOCITY_ENTER, m_pRunStats, m_pRunEntData->m_iCurrentZone,
                                                  enterVelStr, enterVelComparisonStr, &compareColor, true);

            HFont labelFont = m_pStageEnterExitLabel->GetFont();
            // compute the combined length of the enter/exit and comparison
            int combinedLength = UTIL_ComputeStringWidth(labelFont, enterVelStr);
            if (g_pMOMRunCompare->LoadedComparison()) // comparison is available
            {
                m_pStageEnterExitComparisonLabel->SetPos(0, m_pStageEnterExitLabel->GetYPos());
                m_pStageEnterExitComparisonLabel->SetFgColor(Color(compareColor.r(), compareColor.g(), compareColor.b(), m_fStageStartAlpha));
                m_pStageEnterExitComparisonLabel->SetText(enterVelComparisonStr);
                // Add the compare string to the length
                combinedLength += UTIL_ComputeStringWidth(labelFont, enterVelComparisonStr);
            }
            else
            {
                // ignore ending space as comparison is not being shown
                combinedLength -= UTIL_ComputeStringWidth(labelFont, " ");
            }

            int offsetXPos = 0 - ((GetWide() - combinedLength) / 2);
            m_pStageEnterExitLabel->SetPos(offsetXPos, m_pStageEnterExitLabel->GetYPos());
            m_pStageEnterExitLabel->SetFgColor(Color(enterColor.r(), enterColor.g(), enterColor.b(), m_fStageStartAlpha));
            m_pStageEnterExitLabel->SetText(enterVelStr);
        }
    }
}

void CHudSpeedMeter::SetLabelHeight(bool isActive, Label *label, int height)
{
    label->SetAutoTall(isActive);
    label->SetTall(isActive ? height : 0);
    if (!isActive) label->SetText("");
}

void CHudSpeedMeter::SetSpeedometerLabel(bool isActive, Label *label, int height, int roundedSpeed, Color color, float alpha)
{
    if (isActive)
    {
        char speedoValue[BUFSIZELOCL];
        Q_snprintf(speedoValue, sizeof(speedoValue), "%i", roundedSpeed);
        label->SetText(speedoValue);
        if (alpha >= 0.0f) // apply fade if specified
        {
            color = Color(color.r(), color.g(), color.b(), alpha);
        }
        label->SetFgColor(color);
    }
    SetLabelHeight(isActive, label, height);
}

void CHudSpeedMeter::ColorBasedOnAccel(Color &color, int currentVel, int lastVel)
{
    if (!CloseEnough(lastVel, 0.0f))
    {
        float variation = 0.0f;
        const float deadzone = 2.0f;

        if (mom_hud_speedometer_units.GetInt() == 4)
        {
            // For energy, if current value is larger than previous value then we've got an increase
            variation = currentVel - lastVel;
        }
        else
        {
            // Otherwise, if magnitude of value (ie. with abs) is larger then we've got an increase
            // Example: vel = -500, lastvel = -300 counts as an increase in value since magnitude of vel
            // > magnitude of lastvel
            variation = fabs(currentVel) - fabs(lastVel);
        }

        // Get colour from the variation in value
        // If variation > deadzone then color shows as increase
        // Otherwise if variation < deadzone then color shows as decrease
        color = MomUtil::GetColorFromVariation(variation, deadzone, m_NormalColor, m_IncreaseColor, m_DecreaseColor);
    }
    else
    {
        color = m_NormalColor;
    }
}

void CHudSpeedMeter::ColorRelativeToMax(Color &color, int vel)
{
    const float maxvel = sv_maxvelocity.GetFloat();
    switch (static_cast<int>(vel / maxvel * 5.0f))
    {
    case 0:
    default:
        color = m_MaxVelColorLevel1;
        break;
    case 1:
        color = m_MaxVelColorLevel2;
        break;
    case 2:
        color = m_MaxVelColorLevel3;
        break;
    case 3:
        color = m_MaxVelColorLevel4;
        break;
    case 4:
        color = m_MaxVelColorLevel5;
        break;
    }
}
