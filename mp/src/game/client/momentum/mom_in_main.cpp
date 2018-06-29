#include "cbase.h"
#include "kbutton.h"
#include "input.h"
#include "iclientmode.h"
#include "momentum/mom_shareddefs.h"
#include <game/client/iviewport.h>
#include "mom_player_shared.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static	kbutton_t	in_times;

//-----------------------------------------------------------------------------
// Purpose: HL Input interface
//-----------------------------------------------------------------------------
class CMOMInput : public CInput
{
    typedef CInput BaseClass;
public:

    int GetButtonBits(int bResetState) OVERRIDE;

    void LevelInit() OVERRIDE;

    void ComputeForwardMove(CUserCmd* cmd) OVERRIDE;
    void ComputeSideMove(CUserCmd* cmd) OVERRIDE;
};

int CMOMInput::GetButtonBits(int bResetState)
{
    int bits = 0;
    // First calculate all Momentum-specific toggle bits
    CalcButtonBits(bits, IN_TIMES, s_ClearInputState, &in_times, bResetState);

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

void CMOMInput::ComputeForwardMove(CUserCmd* cmd)
{
    BaseClass::ComputeForwardMove(cmd);

    CMomentumPlayer *m_pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (m_pPlayer)
    {
        // Clamp forwards if needed
        if (m_pPlayer->m_afButtonDisabled & IN_FORWARD && cmd->forwardmove > 0)
        {
            cmd->forwardmove = 0;
        }
        // Clamp backwards if needed
        if (m_pPlayer->m_afButtonDisabled & IN_BACK && cmd->forwardmove < 0)
        {
            cmd->forwardmove = 0;
        }
    }
}

void CMOMInput::ComputeSideMove(CUserCmd* cmd)
{
    BaseClass::ComputeSideMove(cmd);

    CMomentumPlayer *m_pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (m_pPlayer)
    {
        // Clamp left if needed
        if (m_pPlayer->m_afButtonDisabled & IN_MOVELEFT && cmd->sidemove < 0)
        {
            cmd->sidemove = 0;
        }
        // Clamp right if needed
        if (m_pPlayer->m_afButtonDisabled & IN_MOVERIGHT && cmd->sidemove > 0)
        {
            cmd->sidemove = 0;
        }
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

// Expose this interface
static CMOMInput g_Input;
IInput *input = (IInput *) &g_Input;