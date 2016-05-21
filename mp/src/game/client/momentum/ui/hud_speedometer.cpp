#include "cbase.h"
#include "hud_comparisons.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "iinput.h"
#include "in_buttons.h"
#include "mom_shareddefs.h"
#include "util/run_compare.h"
#include "vgui_helpers.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "c_mom_replay_entity.h"
#include "momentum/util/mom_util.h"
#include "vphysics_interface.h"
#include <math.h>

using namespace vgui;

static MAKE_TOGGLE_CONVAR(
    mom_speedometer_hvel, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Toggles showing only the horizontal component of player speed. 0 = OFF (XYZ), 1 = ON (XY)\n");

static MAKE_CONVAR(mom_speedometer_units, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Changes the units of measurement of the speedometer.\n 1 = Units per second\n 2 = "
                   "Kilometers per hour\n 3 = Miles per hour.",
                   1, 3);

static MAKE_TOGGLE_CONVAR(mom_speedometer, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_speedometer_colorize, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles speedometer colorization based on acceleration. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_speedometer_showlastjumpvel, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles showing player velocity at last jump (XY only). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(
    mom_speedometer_showenterspeed, "1", FLAG_HUD_CVAR,
    "Toggles showing the stage/checkpoint enter speed (and comparison, if existent). 0 = OFF, 1 = ON\n");

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

  public:
    CHudSpeedMeter(const char *pElementName);

    void Init() override { Reset(); }

    void VidInit() override { Reset(); }

    void Paint() override;
    void PaintNumbers(HFont font, int xpos, int ypos, int value, bool atLeast2Digits) override;

    void Reset() override
    {
        // We set the proper LabelText based on mom_speedmeter_units value
        switch (mom_speedometer_units.GetInt())
        {
        case 2:
            SetLabelText(L"KM/H");
            break;
        case 3:
            SetLabelText(L"MPH");
            break;
        case 1:
        default:
            SetLabelText(L""); // don't draw label if we are on UPS mode (it's a bit redundant)
            break;
        }
        SetDisplayValue(0);
        SetSecondaryValue(0);
        m_flNextColorizeCheck = 0;
        stageStartAlpha = 255;
    }

    void OnThink() override;

    bool ShouldDraw() override { return mom_speedometer.GetBool() && CHudElement::ShouldDraw(); }

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        secondaryColor = GetSchemeColor("SecondaryValueColor", pScheme);
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
        SetBgColor(_bgColor);
        m_LabelColor = normalColor;
    }
    bool ShouldColorize() const { return mom_speedometer_colorize.GetBool(); }

    void FireGameEvent(IGameEvent *pEvent) override
    {
        if (!Q_strcmp(pEvent->GetName(), "stage_exit"))
        {
            // Fade the enter speed after 5 seconds (in event)
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutEnterSpeed");
        }
        else if (!Q_strcmp(pEvent->GetName(), "stage_enter"))
        {
            // Reset the alpha if we hit a stage enter again
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ResetEnterSpeed");
        }
    }

  private:
    float m_flNextColorizeCheck;
    float m_flLastVelocity;
    float m_flLastJumpVelocity;

    int m_iRoundedVel, m_iRoundedLastJump;
    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;
    Color secondaryColor;

    bool m_bRanFadeOutJumpSpeed;

  protected:
    CPanelAnimationVar(Color, _bgColor, "BgColor", "Blank");
    // NOTE: These need to be floats because of animations (thanks volvo)
    CPanelAnimationVar(float, stageStartAlpha, "StageAlpha", "255.0"); // Used for fading
    CPanelAnimationVar(float, lastJumpAlpha, "JumpAlpha", "255.0");
};

DECLARE_NAMED_HUDELEMENT(CHudSpeedMeter, HudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudSpeedMeter")
{
    ListenForGameEvent("stage_exit");
    ListenForGameEvent("stage_enter");
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_bRanFadeOutJumpSpeed = false;
}

void CHudSpeedMeter::OnThink()
{
    Vector velocity = vec3_origin;
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        //This will be null if the player is not watching a replay first person
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();

        //Note: Velocity is also set to the player when watching first person
        velocity = pPlayer->GetLocalVelocity();

        //The last jump velocity
        float lastJumpVel = (pGhost ? pGhost->m_RunData.m_flLastJumpVel : 
            pPlayer->m_RunData.m_flLastJumpVel);

        //The last jump time is also important if the player is watching a replay
        float lastJumpTime = (pGhost ? pGhost->m_RunData.m_flLastJumpTime :
            pPlayer->m_RunData.m_flLastJumpTime);

        int velType = mom_speedometer_hvel.GetBool(); // 1 is horizontal velocity

        if (gpGlobals->curtime - lastJumpTime > 5.0f)
        {
            if (!m_bRanFadeOutJumpSpeed)
                m_bRanFadeOutJumpSpeed =
                    g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutJumpSpeed");
        }
        else
        {
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ResetJumpSpeed");
            m_bRanFadeOutJumpSpeed = false;
        }
        // Remove the vertical component if necessary
        if (velType)
        {
            velocity.z = 0;
        }

        // Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
        float vel = static_cast<float>(velocity.Length());
        switch (mom_speedometer_units.GetInt())
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
        case 1:
        default:
            // We do nothing but break out of the switch, as default vel is already in UPS
            SetLabelText(L"");
            break;
        }

        // only called if we need to update color
        if (ShouldColorize())
        {
            if (m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                if (m_flLastVelocity != 0)
                {
                    m_currentColor = mom_UTIL->GetColorFromVariation(abs(vel) - abs(m_flLastVelocity), 2.0f,
                                                                     normalColor, increaseColor, decreaseColor);
                }
                else
                {
                    m_currentColor = normalColor;
                }
                m_lastColor = m_PrimaryValueColor = m_currentColor;
                m_flLastVelocity = vel;
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }
            // reset last jump velocity when we (or a ghost ent) restart a run by entering the start zone
            if (pGhost)
            {
                if (pGhost->m_RunData.m_bIsInZone && pGhost->m_RunData.m_iCurrentZone == 1)
                    m_flLastJumpVelocity = 0;
            } 
            else
            {
                if (pPlayer->m_RunData.m_bIsInZone && pPlayer->m_RunData.m_iCurrentZone == 1)
                    m_flLastJumpVelocity = 0;
            }


            if (lastJumpVel == 0)
            {
                m_SecondaryValueColor = normalColor;
            }
            else if (m_flLastJumpVelocity != lastJumpVel)
            {
                m_SecondaryValueColor =
                    mom_UTIL->GetColorFromVariation(abs(lastJumpVel) - abs(m_flLastJumpVelocity), 0.0f,
                                                    normalColor, increaseColor, decreaseColor);
                m_flLastJumpVelocity = lastJumpVel;
            }
        }
        else
        {
            m_SecondaryValueColor = m_PrimaryValueColor = normalColor;
        }
        // center text

        m_iRoundedVel = round(vel);
        m_iRoundedLastJump = round(lastJumpVel);
        SetDisplayValue(m_iRoundedVel);
        SetShouldDisplaySecondaryValue(mom_speedometer_showlastjumpvel.GetBool());
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
    Color newSecondary =
        Color(m_SecondaryValueColor.r(), m_SecondaryValueColor.g(), m_SecondaryValueColor.b(), lastJumpAlpha);
    m_SecondaryValueColor = newSecondary;

    BaseClass::Paint();

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());

    // Draw the enter speed split, if toggled on
    if (mom_speedometer_showenterspeed.GetBool() && pPlayer && !pPlayer->m_RunData.m_bIsInZone &&
        g_MOMEventListener->m_bTimerIsRunning)
    {
        int split_xpos; // Dynamically set
        int split_ypos = mom_speedometer_showlastjumpvel.GetBool()
                             ? digit2_ypos + surface()->GetFontTall(m_hSmallNumberFont)
                             : digit2_ypos;

        char enterVelANSITemp[BUFSIZELOCL], enterVelANSICompTemp[BUFSIZELOCL];
        char enterVelANSI[BUFSIZELOCL], enterVelCompareANSI[BUFSIZELOCL];
        wchar_t enterVelUnicode[BUFSIZELOCL], enterVelCompareUnicode[BUFSIZELOCL];

        Color compareColorFade = Color(0, 0, 0, 0);
        Color compareColor = Color(0, 0, 0, 0);

        Color fg = GetFgColor();
        Color actualColorFade = Color(fg.r(), fg.g(), fg.b(), stageStartAlpha);

        g_MOMRunCompare->GetComparisonString(VELOCITY_ENTER, pPlayer->m_RunData.m_iCurrentZone, enterVelANSITemp,
                                             enterVelANSICompTemp, &compareColor);

        Q_snprintf(enterVelANSI, BUFSIZELOCL, "%i", static_cast<int>(round(atof(enterVelANSITemp))));

        ANSI_TO_UNICODE(enterVelANSI, enterVelUnicode);

        // Get the width of the split velocity
        int enterVelANSIWidth = UTIL_ComputeStringWidth(m_hSmallNumberFont, enterVelANSI);

        bool loadedComparison = g_MOMRunCompare->LoadedComparison();
        int increaseX = 0;

        // Calculate the split comparison string
        if (loadedComparison)
        {
            // Really gross way of ripping apart this string and
            // making some sort of quasi-frankenstein string of the float as a rounded int
            char firstThree[3];
            Q_strncpy(firstThree, enterVelANSICompTemp, 3);
            const char *compFloat = enterVelANSICompTemp;
            Q_snprintf(enterVelCompareANSI, BUFSIZELOCL, " %s %i)", firstThree,
                       static_cast<int>(round(atof(compFloat + 3))));
            ANSI_TO_UNICODE(enterVelCompareANSI, enterVelCompareUnicode);

            // Update the compare color to have the alpha defined by animations (fade out if 5+ sec)
            compareColorFade.SetColor(compareColor.r(), compareColor.g(), compareColor.b(), stageStartAlpha);

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
    if (!m_bIsTime)
    {
        if (atLeast2Digits && value < 10)
        {
            V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", value);
        }
        else
        {
            V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", value);
        }
    }
    else
    {
        int iMinutes = value / 60;
        int iSeconds = value - iMinutes * 60;

        if (iSeconds < 10)
            V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d`0%d", iMinutes, iSeconds);
        else
            V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d`%d", iMinutes, iSeconds);
    }

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