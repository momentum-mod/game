#include "cbase.h"
#include <game/client/iviewport.h>
#include "iclientmode.h"
#include "in_buttons.h"
#include "input.h"
#include "kbutton.h"
#include "mom_player_shared.h"
#include "momentum/mom_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar thirdperson_platformer;
extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;
extern ConVar thirdperson_screenspace;
extern ConVar cam_idealyaw;
extern kbutton_t in_left;
extern kbutton_t in_right;
extern kbutton_t in_klook;

static kbutton_t in_times;
static kbutton_t in_lobby_members;
static kbutton_t in_paint;

//-----------------------------------------------------------------------------
// Purpose: HL Input interface
//-----------------------------------------------------------------------------
class CMOMInput : public CInput
{
    typedef CInput BaseClass;

  public:
    int GetButtonBits(int bResetState) OVERRIDE;

    void LevelInit() OVERRIDE;

    void ComputeForwardMove(CUserCmd *cmd) OVERRIDE;
    void ComputeSideMove(CUserCmd *cmd) OVERRIDE;
};

int CMOMInput::GetButtonBits(int bResetState)
{
    int bits = 0;
    // First calculate all Momentum-specific toggle bits
    CalcButtonBits(bits, IN_SCORE, s_ClearInputState, &in_times, bResetState);
    CalcButtonBits(bits, IN_PAINT, s_ClearInputState, &in_paint, bResetState);

    // Add on the normal input bits
    bits |= BaseClass::GetButtonBits(bResetState);

    return bits;
}

void CMOMInput::LevelInit()
{
    // Get buttons still down from the demo
    int stillDown = GetButtonBits(0);
    // Mark them for clearing
    ClearInputButton(stillDown);
    // Actually clear them
    GetButtonBits(2);

    BaseClass::LevelInit();
}

// Sometimes sidemove/forward can be not in the right direction because of the sample time used for the keyboard if when
// you press simultaneously keys so we better check keys so it's more instantaneous, also a problem could occur if the
// player used +strafe too. (Could be a funny way to make the player guess what to use for movements when both are
// blocked)

void CMOMInput::ComputeForwardMove(CUserCmd *cmd)
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        // thirdperson platformer movement
        if (CAM_IsThirdPerson() && thirdperson_platformer.GetInt())
        {
            // movement is always forward in this mode
            float movement =
                KeyState(&in_forward) || KeyState(&in_moveright) || KeyState(&in_back) || KeyState(&in_moveleft);

            cmd->forwardmove += cl_forwardspeed.GetFloat() * movement;

            return;
        }

        // thirdperson screenspace movement
        if (CAM_IsThirdPerson() && thirdperson_screenspace.GetInt())
        {
            float ideal_yaw = cam_idealyaw.GetFloat();
            float ideal_sin = sin(DEG2RAD(ideal_yaw));
            float ideal_cos = cos(DEG2RAD(ideal_yaw));

            float movement = ideal_cos * KeyState(&in_forward) + ideal_sin * KeyState(&in_moveright) +
                             -ideal_cos * KeyState(&in_back) + -ideal_sin * KeyState(&in_moveleft);

            cmd->forwardmove += cl_forwardspeed.GetFloat() * movement;

            return;
        }

        if (!(in_klook.state & 1))
        {
            if ((pPlayer->m_afButtonDisabled & IN_FORWARD) == 0)
                cmd->forwardmove += cl_forwardspeed.GetFloat() * KeyState(&in_forward);
            if ((pPlayer->m_afButtonDisabled & IN_BACK) == 0)
                cmd->forwardmove -= cl_backspeed.GetFloat() * KeyState(&in_back);
        }
    }
}

void CMOMInput::ComputeSideMove(CUserCmd *cmd)
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        // thirdperson platformer movement
        if (CAM_IsThirdPerson() && thirdperson_platformer.GetInt())
        {
            // no sideways movement in this mode
            return;
        }

        // thirdperson screenspace movement
        if (CAM_IsThirdPerson() && thirdperson_screenspace.GetInt())
        {
            float ideal_yaw = cam_idealyaw.GetFloat();
            float ideal_sin = sin(DEG2RAD(ideal_yaw));
            float ideal_cos = cos(DEG2RAD(ideal_yaw));

            float movement = ideal_cos * KeyState(&in_moveright) + ideal_sin * KeyState(&in_back) +
                             -ideal_cos * KeyState(&in_moveleft) + -ideal_sin * KeyState(&in_forward);

            cmd->sidemove += cl_sidespeed.GetFloat() * movement;

            return;
        }

        // If strafing, check left and right keys and act like moveleft and moveright keys
        if (in_strafe.state & 1)
        {
            cmd->sidemove += cl_sidespeed.GetFloat() * KeyState(&in_right);
            cmd->sidemove -= cl_sidespeed.GetFloat() * KeyState(&in_left);
        }

        // Otherwise, check strafe keys
        if ((pPlayer->m_afButtonDisabled & IN_MOVERIGHT) == 0)
            cmd->sidemove += cl_sidespeed.GetFloat() * KeyState(&in_moveright);
        if ((pPlayer->m_afButtonDisabled & IN_MOVELEFT) == 0)
            cmd->sidemove -= cl_sidespeed.GetFloat() * KeyState(&in_moveleft);
    }
}

void IN_TimesDown(const CCommand &args)
{
    KeyDown(&in_times, args[1]);
    if (gViewPortInterface)
    {
        gViewPortInterface->ShowPanel(PANEL_TIMES, true);
    }
}

void IN_TimesUp(const CCommand &args)
{
    KeyUp(&in_times, args[1]);
    if (gViewPortInterface)
    {
        gViewPortInterface->ShowPanel(PANEL_TIMES, false);
    }
}

static ConCommand startshowtimes("+showtimes", IN_TimesDown);
static ConCommand endshowtimes("-showtimes", IN_TimesUp);

void IN_PaintDown(const CCommand &args)
{
    KeyDown(&in_paint, args[1]);
}

void IN_PaintUp(const CCommand &args)
{
    KeyUp(&in_paint, args[1]);
}

static ConCommand startpaint("+paint", IN_PaintDown);
static ConCommand endpaint("-paint", IN_PaintUp);

// Expose this interface
static CMOMInput g_Input;
IInput *input = (IInput *)&g_Input;
