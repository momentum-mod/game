#include "cbase.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_player_shared.h"
#include "momentum/util/mom_util.h"
#include "vphysics_interface.h"
#include <math.h>

using namespace vgui;

static ConVar speedometer_hvel("mom_speedometer_hvel", "0",
                               FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                               "If set to 1, doesn't take the vertical velocity component into account.\n", true, 0,
                               true, 1);

static ConVar speedometer_units("mom_speedometer_units", "1",
                                FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                "Changes the units of measure of the speedmeter.\n 1: Units per second. \n 2: "
                                "Kilometers per hour. \n 3: Milles per hour.\n",
                                true, 1, true, 3);

static ConVar speedometer_draw("mom_drawspeedometer", "1",
                               FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                               "Toggles displaying the speedometer.\n", true, 0, true, 1);

static ConVar speedometer_colorize("mom_speedometer_colorize", "1",
                                   FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                   "Toggles speedometer colorization based on acceleration.\n", true, 0, true, 1);

static ConVar speedometer_lastjump("mom_speedometer_showlastjumpvel", "1",
                                   FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                   "Toggles showing player velocity at last jump (XY only). \n", true, 0, true, 1);

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

  public:
    CHudSpeedMeter(const char *pElementName);
    virtual void Init() { Reset(); }
    virtual void VidInit() { Reset(); }
    virtual void Reset()
    {
        // We set the proper LabelText based on mom_speedmeter_units value
        switch (speedometer_units.GetInt())
        {
        case 2:
            SetLabelText(L"KM/H");
            break;
        case 3:
            SetLabelText(L"MPH");
            break;
        case 1:
        default:
            SetLabelText(L"UPS");
            break;
        }
        SetDisplayValue(0);
        SetSecondaryValue(0);
        m_flNextColorizeCheck = 0;
    }
    virtual void OnThink();
    virtual bool ShouldDraw() { return speedometer_draw.GetBool() && CHudElement::ShouldDraw(); }
    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
        m_LabelColor = normalColor;
    }
    bool ShouldColorize() { return speedometer_colorize.GetBool(); }
  private:
    float m_flNextColorizeCheck;
    float m_flLastVelocity;
    float m_flLastJumpVelocity;

    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;
};

DECLARE_HUDELEMENT(CHudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudSpeedMeter")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void CHudSpeedMeter::OnThink()
{
    Vector velocity = vec3_origin;
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        velocity = pPlayer->GetLocalVelocity();

        // Remove the vertical component if necessary
        if (!speedometer_hvel.GetBool())
        {
            velocity.z = 0;
        }

        // Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
        float vel = (float)velocity.Length();
        switch (speedometer_units.GetInt())
        {
        case 2:
            // 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
            vel *= 0.06858;
            SetLabelText(L"KM/H");
            break;
        case 3:
            // 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
            vel *= 0.04262;
            SetLabelText(L"MPH");
            break;
        case 1:
        default:
            // We do nothing but break out of the switch, as default vel is already in UPS
            SetLabelText(L"UPS");
            break;
        }

        // only called if we need to update color
        if (ShouldColorize())
        {
            if (m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                if (m_flLastVelocity != 0)
                {
                    m_currentColor = mom_UTIL.GetColorFromVariation(abs(vel) - abs(m_flLastVelocity), 2.0f, normalColor,
                                                                    increaseColor, decreaseColor);
                }
                else
                {
                    m_currentColor = normalColor;
                }
                m_lastColor = m_PrimaryValueColor = m_currentColor;
                m_flLastVelocity = vel;
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }
            //reset last jump velocity when we restart a run by entering the start zone 
            if (pPlayer->m_bPlayerInsideStartZone)
                m_flLastJumpVelocity = 0;
            if (pPlayer->m_flLastJumpVel == 0)
            {
                m_SecondaryValueColor = normalColor;
            }
            else if (m_flLastJumpVelocity != pPlayer->m_flLastJumpVel)
            {
                m_SecondaryValueColor =
                    mom_UTIL.GetColorFromVariation(abs(pPlayer->m_flLastJumpVel) - abs(m_flLastJumpVelocity), 0.0f,
                                                   normalColor, increaseColor, decreaseColor);
                m_flLastJumpVelocity = pPlayer->m_flLastJumpVel;
            }
        }
        else
        {
            m_SecondaryValueColor = m_PrimaryValueColor = normalColor;
        }

        SetDisplayValue(round(vel));
        SetShouldDisplaySecondaryValue(speedometer_lastjump.GetBool());
        SetSecondaryValue(round(pPlayer->m_flLastJumpVel));
    }
}
