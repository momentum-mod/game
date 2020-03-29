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
                          "Toggles showing the unit value labels (KM/H, MPH, Energy, U/S). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_showenterspeed, "1", FLAG_HUD_CVAR,
    "Toggles showing the stage/checkpoint enter speed (and comparison, if existent). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_horiz, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_lastjumpvel, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles showing player velocity at last jump (XY only). 0 = OFF, 1 = ON\n");

static MAKE_CONVAR(mom_hud_speedometer_lastjumpvel_fadeout, "3.0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Sets the fade out time for the last jump velocity.", 1.0f, 10.0f);

static MAKE_CONVAR(mom_hud_velocity_type, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles the velocity type used in comparisons and map finished dialog. 0 = ABSOLUTE, 1 = HORIZONTAL",
                   0, 1);

class CHudSpeedMeter : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, EditablePanel);

  public:
    CHudSpeedMeter(const char *pElementName);
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;

    void Paint() OVERRIDE;
    //void PaintNumbers(HFont font, int xpos, int ypos, int value, bool atLeast2Digits) OVERRIDE;

  private:
    float m_flNextColorizeCheck;
    float m_flLastVelocity;
    float m_flLastHVelocity;
    float m_flLastJumpVelocity;

    bool m_bRanFadeOutJumpSpeed;

    int m_iRoundedVel, m_iRoundedHVel, m_iRoundedLastJumpVel;
    Color m_LastColor, m_hLastColor;
    Color m_CurrentColor, m_hCurrentColor;
    Color m_NormalColor, m_IncreaseColor, m_DecreaseColor;
    Color m_LastJumpVelColor;

    wchar_t *m_SpeedometerUnitLabelText;

    Label *m_pUnitsLabel, *m_pAbsSpeedoLabel, *m_pHorizSpeedoLabel, *m_pLastJumpVelLabel, 
        *m_pStageEnterExitLabel, *m_pStageEnterExitComparisonLabel;

    int m_defaultUnitsLabelHeight, m_defaultAbsSpeedoLabelHeight, m_defaultHorizSpeedoLabelHeight, 
        m_defaultLastJumpVelLabelHeight, m_defaultStageEnterExitLabelHeight;

    int m_defaultYPos;

    void inactivateLabel(Label *label);
    void activateLabel(Label *label, int defaultHeight);

    CMomRunStats *m_pRunStats;
    CMomRunEntityData *m_pRunEntData;
  protected:
    CPanelAnimationVar(Color, m_bgColor, "BgColor", "Blank");
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, m_fStageStartAlpha, "StageAlpha", "0.0"); // Used for fading
    CPanelAnimationVar(float, m_fLastJumpVelAlpha, "JumpAlpha", "0.0");
};

DECLARE_HUDELEMENT(CHudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudSpeedMeter")
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

    m_iRoundedVel = 0;
    m_iRoundedHVel = 0;
    m_iRoundedLastJumpVel = 0;

    m_pUnitsLabel = new Label(this, "UnitsLabel", "");
    m_pAbsSpeedoLabel = new Label(this, "AbsSpeedoLabel", "");
    m_pHorizSpeedoLabel = new Label(this, "HorizSpeedoLabel", "");
    m_pLastJumpVelLabel = new Label(this, "LastJumpVelLabel", "");
    m_pStageEnterExitLabel = new Label(this, "StageEnterExitLabel", "");
    m_pStageEnterExitComparisonLabel = new Label(this, "StageEnterExitComparisonLabel", "");
    
    LoadControlSettings("resource/ui/Speedometer.res");

    m_defaultAbsSpeedoLabelHeight = m_pAbsSpeedoLabel->GetTall();
    m_defaultHorizSpeedoLabelHeight = m_pHorizSpeedoLabel->GetTall();
    m_defaultLastJumpVelLabelHeight = m_pLastJumpVelLabel->GetTall();
    m_defaultUnitsLabelHeight = m_pUnitsLabel->GetTall();
    m_defaultStageEnterExitLabelHeight = m_pStageEnterExitLabel->GetTall();

    m_pStageEnterExitComparisonLabel->SetFont(m_pStageEnterExitLabel->GetFont()); //need to have same font

    m_defaultYPos = GetYPos();
}

void CHudSpeedMeter::inactivateLabel(Label *label)
{
    label->SetAutoTall(false);
    label->SetText("");
    label->SetTall(0);
}

void CHudSpeedMeter::activateLabel(Label *label, int defaultHeight)
{
    label->SetAutoTall(true);
    label->SetTall(defaultHeight);
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
    Panel::ApplySchemeSettings(pScheme);
    m_NormalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
    m_IncreaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
    m_DecreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
    SetBgColor(m_bgColor);

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
        Vector horizVelocity = pPlayer->GetAbsVelocity();
        horizVelocity.z = 0;

        if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
        {
            const float fReplayTimeScale = ConVarRef("mom_replay_timescale").GetFloat();
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
        // The last jump velocity & z-coordinate
        float lastJumpVel = m_pRunEntData->m_flLastJumpVel;
        if (gpGlobals->curtime - m_pRunEntData->m_flLastJumpTime > mom_hud_speedometer_lastjumpvel_fadeout.GetFloat())
        {
            if (!m_bRanFadeOutJumpSpeed)
            {
                m_bRanFadeOutJumpSpeed =
                    g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutJumpSpeed");
            }
        }
        else
        {
            // Keep the alpha at full opaque so we can see it
            // MOM_TODO: If we want it to fade back in, we should use an event here
            m_fLastJumpVelAlpha = 255.0f;
            m_bRanFadeOutJumpSpeed = false;
        }

        switch (ConVarRef("mom_hud_speedometer_units").GetInt())
        {
        case 2:
            // 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
            vel *= 0.06858f;
            hvel *= 0.06858f;
            lastJumpVel *= 0.06858f;
            m_SpeedometerUnitLabelText = L"KM/H";
            break;
        case 3:
            // 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
            vel *= 0.04262f;
            hvel *= 0.04262f;
            lastJumpVel *= 0.04262f;
            m_SpeedometerUnitLabelText = L"MPH";
            break;
        case 4:
        {
            // Normalized units of energy
            const auto gravity = sv_gravity.GetFloat();
            vel = (pPlayer->GetAbsVelocity().LengthSqr() / 2.0f +
                   gravity * (pPlayer->GetLocalOrigin().z - m_pRunEntData->m_flLastJumpZPos)) / gravity;
            hvel = vel;
            m_SpeedometerUnitLabelText = L"Energy";
            break;
        }
        case 1:
        default:
            // We do nothing but break out of the switch, as default vel is already in UPS
            m_SpeedometerUnitLabelText = L"U/S";
            break;
        }
        // if we don't show labels, remove it.
        if (!ConVarRef("mom_hud_speedometer_unit_labels").GetBool())
            m_SpeedometerUnitLabelText = L"";

        // only called if we need to update color
        if (ConVarRef("mom_hud_speedometer_colorize").GetInt())
        {
            if (m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                if (ConVarRef("mom_hud_speedometer_colorize").GetInt() == 1)
                {
                    if (m_flLastVelocity != 0)
                    {
                        float variation = 0.0f;
                        const float deadzone = 2.0f;

                        if (ConVarRef("mom_hud_speedometer_units").GetInt() == 4)
                        {
                            // For energy, if current value is larger than previous value then we've got an increase
                            variation = vel - m_flLastVelocity;
                        }
                        else
                        {
                            // Otherwise, if magnitude of value (ie. with abs) is larger then we've got an increase
                            // Example: vel = -500, lastvel = -300 counts as an increase in value since magnitude of vel
                            // > magnitude of lastvel
                            variation = fabs(vel) - fabs(m_flLastVelocity);
                        }

                        // Get colour from the variation in value
                        // If variation > deadzone then color shows as increase
                        // Otherwise if variation < deadzone then color shows as decrease
                        m_CurrentColor = MomUtil::GetColorFromVariation(variation, deadzone, m_NormalColor,
                                                                        m_IncreaseColor, m_DecreaseColor);
                    }
                    else
                    {
                        m_CurrentColor = m_NormalColor;
                    }

                    //Now the same but for the horizontal speedometer
                    if (m_flLastHVelocity != 0)
                    {
                        float hvariation = 0.0f;
                        const float deadzone = 2.0f;

                        if (ConVarRef("mom_hud_speedometer_units").GetInt() == 4)
                            hvariation = hvel - m_flLastHVelocity;
                        else
                            hvariation = fabs(hvel) - fabs(m_flLastHVelocity);
                        m_hCurrentColor = MomUtil::GetColorFromVariation(hvariation, deadzone, m_NormalColor,
                                                                         m_IncreaseColor, m_DecreaseColor);
                    }
                    else
                    {
                        m_hCurrentColor = m_NormalColor;
                    }
                }
                else //color based on relation to max vel
                {
                    const float maxvel = ConVarRef("sv_maxvelocity").GetFloat();
                    switch (static_cast<int>(vel / maxvel * 5.0f))
                    {
                    case 0:
                    default:
                        m_CurrentColor = Color(255, 255, 255, 255); // White
                        break;
                    case 1:
                        m_CurrentColor = Color(255, 255, 0, 255); // Yellow
                        break;
                    case 2:
                        m_CurrentColor = Color(255, 165, 0, 255); // Orange
                        break;
                    case 3:
                        m_CurrentColor = Color(255, 0, 0, 255); // Red
                        break;
                    case 4:
                        m_CurrentColor = Color(128, 0, 128, 255); // Purple
                        break;
                    }
                    switch (static_cast<int>(hvel / maxvel * 5.0f))
                    {
                    case 0:
                    default:
                        m_hCurrentColor = Color(255, 255, 255, 255); // White
                        break;
                    case 1:
                        m_hCurrentColor = Color(255, 255, 0, 255); // Yellow
                        break;
                    case 2:
                        m_hCurrentColor = Color(255, 165, 0, 255); // Orange
                        break;
                    case 3:
                        m_hCurrentColor = Color(255, 0, 0, 255); // Red
                        break;
                    case 4:
                        m_hCurrentColor = Color(128, 0, 128, 255); // Purple
                        break;
                    }
                }
                m_LastColor = m_CurrentColor;
                m_hLastColor = m_hCurrentColor;
                m_flLastVelocity = vel;
                m_flLastHVelocity = hvel;
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }

            // reset last jump velocity when we (or a ghost ent) restart a run by entering the start zone
            if (m_pRunEntData->m_bIsInZone && m_pRunEntData->m_iCurrentZone == 1)
                m_flLastJumpVelocity = 0;

            if (CloseEnough(lastJumpVel, 0.0f))
            {
                m_LastJumpVelColor = m_NormalColor;
            }
            else if (m_flLastJumpVelocity != lastJumpVel)
            {
                m_LastJumpVelColor =
                    MomUtil::GetColorFromVariation(fabs(lastJumpVel) - fabs(m_flLastJumpVelocity), 0.0f,
                                                                m_NormalColor, m_IncreaseColor, m_DecreaseColor);
                m_flLastJumpVelocity = lastJumpVel;
            }
        }
        else
        {
            m_LastJumpVelColor = m_CurrentColor = m_hCurrentColor = m_NormalColor;
        }

        m_iRoundedVel = RoundFloatToInt(vel);
        m_iRoundedHVel = RoundFloatToInt(hvel);
        m_iRoundedLastJumpVel = RoundFloatToInt(lastJumpVel);
        m_pAbsSpeedoLabel->SetFgColor(m_CurrentColor);
        m_pHorizSpeedoLabel->SetFgColor(m_hCurrentColor);
    }
}

void CHudSpeedMeter::Paint() 
{
    int yIndent = 0;
    if (!mom_hud_speedometer.GetBool()) yIndent += m_defaultAbsSpeedoLabelHeight;
    if (!mom_hud_speedometer_horiz.GetBool()) yIndent += m_defaultHorizSpeedoLabelHeight;
    if (!mom_hud_speedometer_lastjumpvel.GetBool()) yIndent += m_defaultLastJumpVelLabelHeight;
    if (!mom_hud_speedometer_unit_labels.GetBool()) yIndent += m_defaultUnitsLabelHeight;
    if (!mom_hud_speedometer_showenterspeed.GetBool()) yIndent += m_defaultStageEnterExitLabelHeight;
    SetPos(GetXPos(), m_defaultYPos + yIndent);

    if (mom_hud_speedometer.GetBool())
    {
        activateLabel(m_pAbsSpeedoLabel, m_defaultAbsSpeedoLabelHeight);
        char speedoValue[BUFSIZELOCL];
        Q_snprintf(speedoValue, sizeof(speedoValue), "%i", m_iRoundedVel);
        m_pAbsSpeedoLabel->SetText(speedoValue);
    }
    else
        inactivateLabel(m_pAbsSpeedoLabel);

    if (mom_hud_speedometer_horiz.GetBool())
    {
        activateLabel(m_pHorizSpeedoLabel, m_defaultHorizSpeedoLabelHeight);
        char hSpeedoValue[BUFSIZELOCL];
        Q_snprintf(hSpeedoValue, sizeof(hSpeedoValue), "%i", m_iRoundedHVel);
        m_pHorizSpeedoLabel->SetText(hSpeedoValue);
    }
    else
        inactivateLabel(m_pHorizSpeedoLabel);

    if (mom_hud_speedometer_lastjumpvel.GetBool())
    {
        activateLabel(m_pLastJumpVelLabel, m_defaultLastJumpVelLabelHeight);
        char lastJumpVelValue[BUFSIZELOCL];
        Q_snprintf(lastJumpVelValue, sizeof(lastJumpVelValue), "%i", m_iRoundedLastJumpVel);
        m_pLastJumpVelLabel->SetText(lastJumpVelValue);
        // Fade out last jump vel based on the lastJumpAlpha value
        m_LastJumpVelColor =
            Color(m_LastJumpVelColor.r(), m_LastJumpVelColor.g(), m_LastJumpVelColor.b(), m_fLastJumpVelAlpha);
        m_pLastJumpVelLabel->SetFgColor(m_LastJumpVelColor);
    }
    else
        inactivateLabel(m_pLastJumpVelLabel);

    if (mom_hud_speedometer_unit_labels.GetBool())
    {
        activateLabel(m_pUnitsLabel, m_defaultUnitsLabelHeight);
        m_pUnitsLabel->SetText(m_SpeedometerUnitLabelText);
    }
    else
        inactivateLabel(m_pUnitsLabel);
    //if every speedometer is off, don't bother drawing unit labels
    if (!mom_hud_speedometer.GetBool() && !mom_hud_speedometer_horiz.GetBool() && 
        !mom_hud_speedometer_lastjumpvel.GetBool() && !mom_hud_speedometer_showenterspeed.GetBool() && 
        mom_hud_speedometer_unit_labels.GetBool())
        inactivateLabel(m_pUnitsLabel);

    // Draw the enter speed split, if toggled on. Cannot be done in OnThink()
    if (ConVarRef("mom_hud_speedometer_showenterspeed").GetBool() && m_pRunEntData && m_pRunEntData->m_bTimerRunning &&
        m_fStageStartAlpha > 0.0f)
    {
        activateLabel(m_pStageEnterExitLabel, m_defaultStageEnterExitLabelHeight);
        activateLabel(m_pStageEnterExitComparisonLabel, m_defaultStageEnterExitLabelHeight);

        char enterVelUnrounded[BUFSIZELOCL], enterVelRounded[BUFSIZELOCL], enterVelComparisonUnrounded[BUFSIZELOCL],
            enterVelComparisonRounded[BUFSIZELOCL];

        Color compareColorFade = Color(0, 0, 0, 0);
        Color compareColor = Color(0, 0, 0, 0);

        Color fg = GetFgColor();
        Color actualColorFade = Color(fg.r(), fg.g(), fg.b(), m_fStageStartAlpha);

        g_pMOMRunCompare->GetComparisonString(VELOCITY_ENTER, m_pRunStats, m_pRunEntData->m_iCurrentZone,
                                              enterVelUnrounded, enterVelComparisonUnrounded, &compareColor);
        // Round velocity
        Q_snprintf(enterVelRounded, BUFSIZELOCL, "%i", static_cast<int>(round(atof(enterVelUnrounded))));

        bool loadedComparison = g_pMOMRunCompare->LoadedComparison();

        HFont labelFont = m_pStageEnterExitLabel->GetFont();
        int spaceBetweenLabels = UTIL_ComputeStringWidth(labelFont, " ");
        // compute the combined length of the enter/exit and comparison
        int combinedLength = UTIL_ComputeStringWidth(labelFont, enterVelRounded);
        if (loadedComparison)
        {
            // Really gross way of ripping apart this string and
            // making some sort of quasi-frankenstein string of the float as a rounded int
            char firstThree[3];
            Q_strncpy(firstThree, enterVelComparisonUnrounded, 3);
            const char *compFloat = enterVelComparisonUnrounded;
            Q_snprintf(enterVelComparisonRounded, BUFSIZELOCL, " %s %i)", firstThree,
                       RoundFloatToInt(Q_atof(compFloat + 3)));

            // Update the compare color to have the alpha defined by animations (fade out if 5+ sec)
            compareColorFade.SetColor(compareColor.r(), compareColor.g(), compareColor.b(), m_fStageStartAlpha);
            
            // Add the compare string to the length
            combinedLength += spaceBetweenLabels + UTIL_ComputeStringWidth(labelFont, enterVelComparisonRounded);
        }
        int offsetXPos = 0 - ((GetWide() - combinedLength) / 2);

        //split_xpos = GetWide() / 2 - (enterVelANSIWidth + increaseX) / 2;
        m_pStageEnterExitLabel->SetPos(offsetXPos, m_pStageEnterExitLabel->GetYPos());
        m_pStageEnterExitLabel->SetFgColor(actualColorFade);
        m_pStageEnterExitLabel->SetText(enterVelRounded);

        //print comparison as well
        if (loadedComparison)
        {
            m_pStageEnterExitComparisonLabel->SetPos(spaceBetweenLabels, m_pStageEnterExitLabel->GetYPos());
            m_pStageEnterExitComparisonLabel->SetFgColor(compareColorFade);
            m_pStageEnterExitComparisonLabel->SetText(enterVelComparisonRounded);
        }
        else //only print velocity
        {
            m_pStageEnterExitComparisonLabel->SetText("");
        }
    }
    else if (!ConVarRef("mom_hud_speedometer_showenterspeed").GetBool())
    {
        inactivateLabel(m_pStageEnterExitLabel);
        inactivateLabel(m_pStageEnterExitComparisonLabel);
    }
}
