#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hud_macros.h"
#include "utlvector.h"
#include "KeyValues.h"
#include "iclientmode.h"
#include "steam/steam_api.h"
#include "view.h"
#include "menu.h"
#include "vgui_helpers.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "mom_event_listener.h"
#include "momentum/util/mom_util.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"


using namespace vgui;

static ConVar mom_comparisons("mom_hud_comparisons", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED, 
    "Shows the run comparison panel. 0 = OFF, 1 = ON");

class C_RunComparisons : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_RunComparisons, Panel);

public:
    C_RunComparisons();
    C_RunComparisons(const char* pElementName);
    //~C_RunComparisons();

    void OnThink() override;
    void Init() override;
    void Reset() override;
    void Paint() override;
    bool ShouldDraw() override
    {
        return mom_comparisons.GetBool() && CHudElement::ShouldDraw();//MOM_TODO: && !mapFinished
    }

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_TimeGain = GetSchemeColor("MOM.Timer.Gain", pScheme);
        m_TimeLoss = GetSchemeColor("MOM.Timer.Loss", pScheme);
    }

protected:
    CPanelAnimationVar(Color, m_TimeGain, "TimeGainColor", "FgColor");
    CPanelAnimationVar(Color, m_TimeLoss, "TimeLossColor", "FgColor");

};

DECLARE_HUDELEMENT(C_RunComparisons);


C_RunComparisons::C_RunComparisons(const char* pElementName) : CHudElement(pElementName),
Panel(g_pClientMode->GetViewport(), "CHudCompare")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);//MOM_TODO: will we want key
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void C_RunComparisons::OnThink()
{
}

void C_RunComparisons::Init()
{
    //LOCALIZE STUFF HERE
}

void C_RunComparisons::Reset()
{
    //I don't know what to do here, this is called each spawn?
}

void C_RunComparisons::Paint()
{
    //MOM_TODO: Determine the max number of stages to be shown on the panel.
    //MOM_TODO: Determine size of panel, affects above.
    //MOM_TODO: Make panel scale to amount of stages. Linear maps will have checkpoints.

    //Get player current stage
    g_MOMEventListener->m_iCurrentStage;
    //We want to create a "buffer" of stages. The very last stage should show
    //full comparisons, and be the most bottom one. However, the stages before that need
    //to show time comparisons next to them. How I think it should go:

    //Comparing against: (run comparing against: usually PB or WR, could be any run?)
    //Stage 1 (+/- XX:XX.XX)
    //Stage 2 (+/- XX:XX.XX)
    //Stage 3 (+/- XX:XX.XX)
    //Stage 4
    //  Time  (+/- XX:XX.XX)
    //  Vel   (+/- XXX.XX)
    //  Sync? etc

    for (int i = 1; i < g_MOMEventListener->m_iCurrentStage; i++)
    {
        if (i == g_MOMEventListener->m_iCurrentStage - 1)
        {
            //everything:
            //print Stage (i)
            //print time
            //print vel
            //print sync?
            //print jumps/strafes?
        } else
        {
            //print "Stage (i) (stageTime[i])" with colorization etc
        }
    }

}