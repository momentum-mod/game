#include "cbase.h"
#include "hud_comparisons.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_shareddefs.h"
#include "run/run_compare.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>

#include "c_baseplayer.h"
#include "movevars_shared.h"
#include "mom_player_shared.h"
#include "c_mom_replay_entity.h"
#include "momentum/util/mom_util.h"
#include "baseviewport.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(
    mom_hud_speedometer_hvel, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Toggles showing only the horizontal component of player speed. 0 = OFF (XYZ), 1 = ON (XY)\n");

static MAKE_CONVAR(mom_hud_speedometer_units, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Changes the units of measurement of the speedometer.\n 1 = Units per second\n 2 = "
                   "Kilometers per hour\n 3 = Miles per hour\n 4 = Energy",
                   1, 4);

static MAKE_CONVAR(mom_hud_speedometer_colorize, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles speedometer colorization. 0 = OFF, 1 = ON (Based on acceleration)," 
                          " 2 = ON (Staged by relative velocity to max.)\n", 0, 2);

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_showlastjumpvel, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles showing player velocity at last jump (XY only). 0 = OFF, 1 = ON\n");

static MAKE_CONVAR(mom_hud_speedometer_lastjumpvel_fadeout, "3.0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE, 
                   "Sets the fade out time for the last jump velocity.", 1.0f, 10.0f);

static MAKE_TOGGLE_CONVAR(
    mom_hud_speedometer_showenterspeed, "1", FLAG_HUD_CVAR,
    "Toggles showing the stage/checkpoint enter speed (and comparison, if existent). 0 = OFF, 1 = ON\n");

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
public:
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

    CHudSpeedMeter(const char *pElementName);

    void Init() OVERRIDE { Reset(); }
    void VidInit() OVERRIDE { Reset(); }
    void Reset() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
    void PaintNumbers(HFont font, int xpos, int ypos, int value, bool atLeast2Digits) OVERRIDE;

  private:
    float m_flNextColorizeCheck;
    float m_flLastVelocity;
    float m_flLastJumpVelocity;
    float m_flLastJumpZPos;

    int m_iRoundedVel, m_iRoundedLastJump;
    Color m_LastColor;
    Color m_CurrentColor;
    Color m_NormalColor, m_IncreaseColor, m_DecreaseColor;
    Color m_SecondaryColor;

    bool m_bRanFadeOutJumpSpeed;
    CMomRunStats *m_pRunStats;
    CMomRunEntityData *m_pRunEntData;

  protected:
    CPanelAnimationVar(Color, m_bgColor, "BgColor", "Blank");
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, m_fStageStartAlpha, "StageAlpha", "0.0"); // Used for fading
    CPanelAnimationVar(float, m_fLastJumpAlpha, "JumpAlpha", "0.0");
};

DECLARE_NAMED_HUDELEMENT(CHudSpeedMeter, HudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudSpeedMeter")
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    m_bRanFadeOutJumpSpeed = false;
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
    m_bIsTime = false;
}

void CHudSpeedMeter::Reset()
{
    SetDisplayValue(0);
    SetSecondaryValue(0);
    m_flNextColorizeCheck = 0;
    m_fStageStartAlpha = 0.0f;
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

    m_SecondaryColor = GetSchemeColor("SecondaryValueColor", pScheme);
    m_NormalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
    m_IncreaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
    m_DecreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    SetBgColor(m_bgColor);
    m_LabelColor = m_NormalColor;
}

bool CHudSpeedMeter::ShouldDraw()
{
    return mom_hud_speedometer.GetBool() && CHudElement::ShouldDraw();
}

void CHudSpeedMeter::OnThink()
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        m_pRunEntData = pPlayer->GetCurrentUIEntData();
        //Note: Velocity is also set to the player when watching first person
        Vector velocity = pPlayer->GetAbsVelocity();

        if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
        {
            const float fReplayTimeScale = ConVarRef("mom_replay_timescale").GetFloat();
            if (fReplayTimeScale < 1.0f)
                velocity /= fReplayTimeScale;
        }

        //The last jump velocity & z-coordinate
        float lastJumpVel = m_pRunEntData->m_flLastJumpVel;

        m_pRunStats = pPlayer->GetCurrentUIEntStats();

        if (gpGlobals->curtime - m_pRunEntData->m_flLastJumpTime > mom_hud_speedometer_lastjumpvel_fadeout.GetFloat())
        {
            if (!m_bRanFadeOutJumpSpeed)
            {
                m_bRanFadeOutJumpSpeed = g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutJumpSpeed");
            }
        }
        else
        {
            //Keep the alpha at full opaque so we can see it
            //MOM_TODO: If we want it to fade back in, we should use an event here
            m_fLastJumpAlpha = 255.0f;
            m_bRanFadeOutJumpSpeed = false;
        }

        int velType = mom_hud_speedometer_hvel.GetBool(); // 1 is horizontal velocity
        // Remove the vertical component if necessary
        velocity.z *= 1 - velType;

        // Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
        float vel = static_cast<float>(velocity.Length());
        switch (mom_hud_speedometer_units.GetInt())
        {
        case 2:
            // 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
            vel *= 0.06858f;
            lastJumpVel *= 0.06858f;
            SetLabelText(L"KM/H");
            break;
        case 3:
            // 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
            vel *= 0.04262f;
            lastJumpVel *= 0.04262f;
            SetLabelText(L"MPH");
            break;
        case 4:
            {
                // Normalized units of energy
                const auto gravity = sv_gravity.GetFloat();
                vel = (pPlayer->GetAbsVelocity().LengthSqr()/2.0f + 
                    gravity * (pPlayer->GetLocalOrigin().z - m_pRunEntData->m_flLastJumpZPos)) / gravity;
                SetLabelText(L"Energy");
                break;
            }
        case 1:
        default:
            // We do nothing but break out of the switch, as default vel is already in UPS
            SetLabelText(L"");
            break;
        }

        // only called if we need to update color
        if (mom_hud_speedometer_colorize.GetInt())
        {
            if (m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                if (mom_hud_speedometer_colorize.GetInt() == 1)
                {
                    if (m_flLastVelocity != 0)
                    {
                        float variation = 0.0f;
                        const float deadzone = 2.0f;

                        if (mom_hud_speedometer_units.GetInt() == 4)
                        {
                            // For energy, if current value is larger than previous value then we've got an increase
                            variation = vel - m_flLastVelocity;
                        }
                        else
                        {
                            // Otherwise, if magnitude of value (ie. with abs) is larger then we've got an increase
                            // Example: vel = -500, lastvel = -300 counts as an increase in value since magnitude of vel > magnitude of lastvel
                            variation = fabs(vel) - fabs(m_flLastVelocity);
                        }

                       // Get colour from the variation in value
                       // If variation > deadzone then color shows as increase
                       // Otherwise if variation < deadzone then color shows as decrease
                        m_CurrentColor = MomUtil::GetColorFromVariation(variation, deadzone, m_NormalColor, m_IncreaseColor, m_DecreaseColor);
                    }
                    else
                    {
                        m_CurrentColor = m_NormalColor;
                    }
                }
                else
                {
                    const float maxvel = ConVarRef("sv_maxvelocity").GetFloat();
                    switch (static_cast<int>(vel / maxvel * 5.0f))
                    {
                        case 0:
                        default:
                            m_CurrentColor = Color(255, 255, 255, 255);  // White
                            break;
                        case 1:
                            m_CurrentColor = Color(255, 255, 0, 255);  // Yellow
                            break;
                        case 2:
                            m_CurrentColor = Color(255, 165, 0, 255);  // Orange
                            break;
                        case 3:
                            m_CurrentColor = Color(255, 0, 0, 255);  // Red
                            break;
                        case 4:
                            m_CurrentColor = Color(128, 0, 128, 255);  // Purple
                            break;             
                    }
                }
                m_LastColor = m_PrimaryValueColor = m_CurrentColor;
                m_flLastVelocity = vel;
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }

            // reset last jump velocity when we (or a ghost ent) restart a run by entering the start zone
            if (m_pRunEntData->m_bIsInZone && m_pRunEntData->m_iCurrentZone == 1)
                m_flLastJumpVelocity = 0;

            if (CloseEnough(lastJumpVel, 0.0f))
            {
                m_SecondaryValueColor = m_NormalColor;
            }
            else if (m_flLastJumpVelocity != lastJumpVel)
            {
                m_SecondaryValueColor =
                    MomUtil::GetColorFromVariation(fabs(lastJumpVel) - fabs(m_flLastJumpVelocity), 0.0f,
                                                   m_NormalColor, m_IncreaseColor, m_DecreaseColor);
                m_flLastJumpVelocity = lastJumpVel;
            }
        }
        else
        {
            m_SecondaryValueColor = m_PrimaryValueColor = m_NormalColor;
        }
        // center text

        m_iRoundedVel = RoundFloatToInt(vel);
        m_iRoundedLastJump = RoundFloatToInt(lastJumpVel);
        SetDisplayValue(m_iRoundedVel);
        SetShouldDisplaySecondaryValue(mom_hud_speedometer_showlastjumpvel.GetBool());
        SetSecondaryValue(m_iRoundedLastJump);
    }
}

void CHudSpeedMeter::Paint()
{
    char speedoValue[BUFSIZELOCL], lastJumpvValue[BUFSIZELOCL];
    Q_snprintf(speedoValue, sizeof(speedoValue), "%d", m_iRoundedVel);
    Q_snprintf(lastJumpvValue, sizeof(lastJumpvValue), "%d", m_iRoundedLastJump);

    text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_LabelText) / 2;
    digit_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hNumberFont, speedoValue) / 2;
    digit2_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallNumberFont, lastJumpvValue) / 2;

    // Fade out the secondary value (last jump speed) based on the lastJumpAlpha value
    m_SecondaryValueColor =
        Color(m_SecondaryValueColor.r(), m_SecondaryValueColor.g(), m_SecondaryValueColor.b(), m_fLastJumpAlpha);

    BaseClass::Paint();

    // Draw the enter speed split, if toggled on
    if (mom_hud_speedometer_showenterspeed.GetBool() && m_pRunEntData && m_pRunEntData->m_bTimerRunning && m_fStageStartAlpha > 0.0f)
    {
        int split_xpos; // Dynamically set
        int split_ypos = mom_hud_speedometer_showlastjumpvel.GetBool()
                             ? digit2_ypos + surface()->GetFontTall(m_hSmallNumberFont)
                             : digit2_ypos;

        char enterVelANSITemp[BUFSIZELOCL], enterVelANSICompTemp[BUFSIZELOCL];
        char enterVelANSI[BUFSIZELOCL], enterVelCompareANSI[BUFSIZELOCL];
        wchar_t enterVelUnicode[BUFSIZELOCL], enterVelCompareUnicode[BUFSIZELOCL];

        Color compareColorFade = Color(0, 0, 0, 0);
        Color compareColor = Color(0, 0, 0, 0);

        Color fg = GetFgColor();
        Color actualColorFade = Color(fg.r(), fg.g(), fg.b(), m_fStageStartAlpha);


        g_pMOMRunCompare->GetComparisonString(VELOCITY_ENTER, m_pRunStats, m_pRunEntData->m_iCurrentZone, enterVelANSITemp,
                                             enterVelANSICompTemp, &compareColor);

        Q_snprintf(enterVelANSI, BUFSIZELOCL, "%i", static_cast<int>(round(atof(enterVelANSITemp))));

        ANSI_TO_UNICODE(enterVelANSI, enterVelUnicode);

        // Get the width of the split velocity
        int enterVelANSIWidth = UTIL_ComputeStringWidth(m_hSmallNumberFont, enterVelANSI);

        bool loadedComparison = g_pMOMRunCompare->LoadedComparison();
        int increaseX = 0;

        // Calculate the split comparison string
        if (loadedComparison)
        {
            // Really gross way of ripping apart this string and
            // making some sort of quasi-frankenstein string of the float as a rounded int
            char firstThree[3];
            Q_strncpy(firstThree, enterVelANSICompTemp, 3);
            const char *compFloat = enterVelANSICompTemp;
            Q_snprintf(enterVelCompareANSI, BUFSIZELOCL, " %s %i)", firstThree, RoundFloatToInt(Q_atof(compFloat + 3)));
            ANSI_TO_UNICODE(enterVelCompareANSI, enterVelCompareUnicode);

            // Update the compare color to have the alpha defined by animations (fade out if 5+ sec)
            compareColorFade.SetColor(compareColor.r(), compareColor.g(), compareColor.b(), m_fStageStartAlpha);

            increaseX = UTIL_ComputeStringWidth(m_hSmallNumberFont, enterVelCompareANSI);
        }

        // Our X pos is based on both the split and compare string
        split_xpos = GetWide() / 2 - (enterVelANSIWidth + increaseX) / 2;

        // Print the split vel
        surface()->DrawSetTextFont(m_hSmallNumberFont);
        surface()->DrawSetTextPos(split_xpos, split_ypos);
        surface()->DrawSetTextColor(actualColorFade);
        surface()->DrawPrintText(enterVelUnicode, wcslen(enterVelUnicode));

        if (loadedComparison)
        {
            // Print the comparison
            surface()->DrawSetTextPos(split_xpos + enterVelANSIWidth, split_ypos);
            surface()->DrawSetTextColor(compareColorFade);
            surface()->DrawPrintText(enterVelCompareUnicode, wcslen(enterVelCompareUnicode));
        }
    }
}

// we override this here so the last jump vel display doesnt have a double 0
void CHudSpeedMeter::PaintNumbers(HFont font, int xpos, int ypos, int value, bool atLeast2Digits)
{
    surface()->DrawSetTextFont(font);
    wchar_t unicode[6];
    V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", value);

    // adjust the position to take into account 3 characters
    int charWidth = surface()->GetCharacterWidth(font, '0');
    if (value < 100 && m_bIndent)
    {
        xpos += charWidth;
    }
    if (value < 10 && m_bIndent)
    {
        xpos += charWidth;
    }

    surface()->DrawSetTextPos(xpos, ypos);
    surface()->DrawUnicodeString(unicode);
}