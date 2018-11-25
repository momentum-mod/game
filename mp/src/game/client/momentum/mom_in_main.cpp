#include "cbase.h"
#include <game/client/iviewport.h>
#include "TASPanel.h"
#include "c_mom_player.h"
#include "iclientmode.h"
#include "input.h"
#include "kbutton.h"
#include "momentum/mom_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static kbutton_t in_times;

//-----------------------------------------------------------------------------
// Purpose: HL Input interface
//-----------------------------------------------------------------------------
class CMOMInput : public CInput
{
    typedef CInput BaseClass;

  public:
    int GetButtonBits(int bResetState) OVERRIDE
    {
        int bits = 0;
        // First calculate all Momentum-specific toggle bits
        CalcButtonBits(bits, IN_TIMES, s_ClearInputState, &in_times, bResetState);

        // Add on the normal input bits
        bits |= BaseClass::GetButtonBits(bResetState);

        return bits;
    }

    void LevelInit() OVERRIDE
    {
        // Get buttons still down from the demo
        int stillDown = GetButtonBits(0);
        // Mark them for clearing
        ClearInputButton(stillDown);
        // Actually clear them
        GetButtonBits(2);

        BaseClass::LevelInit();
    }

    void CreateMove(int sequence_number, float input_sample_frametime, bool active) OVERRIDE
    {
        BaseClass::CreateMove(sequence_number, input_sample_frametime, active);

        auto pPlayer = reinterpret_cast<C_MomentumPlayer *>(C_BasePlayer::GetLocalPlayer());
        pPlayer->m_LastCreateMoveCmd = *input->GetUserCmd(sequence_number);

        if (vgui::g_pTASPanel != nullptr)
        {
            vgui::g_pTASPanel->m_pVisualPanel->VisPredMovements();
        }
    }
};

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
IInput *input = (IInput *)&g_Input;