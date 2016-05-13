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

#define STAGE_BUFFER 4 //The amount of stages that the panel should show. MOM_TODO: Maybe make this a convar?

using namespace vgui;

static ConVar mom_comparisons("mom_hud_comparisons", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED, 
    "Shows the run comparison panel. 0 = OFF, 1 = ON");

class C_RunComparisons : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_RunComparisons, Panel);

public:
    C_RunComparisons();
    C_RunComparisons(const char* pElementName);
    ~C_RunComparisons();

    void OnThink() override;
    void Init() override;
    void Reset() override;
    void Paint() override;
    bool ShouldDraw() override
    {
        return mom_comparisons.GetBool() && CHudElement::ShouldDraw() && 
            g_MOMEventListener && g_MOMEventListener->m_bTimerIsRunning;
    }

    void FireGameEvent(IGameEvent *event);

    void LoadComparisons();
    void UnloadComparisons();
    void GetTimeString(int, char*, Color *);

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_cTimeGain = GetSchemeColor("MOM.Timer.Gain", pScheme);
        m_cTimeLoss = GetSchemeColor("MOM.Timer.Loss", pScheme);
    }

protected:
    CPanelAnimationVar(Color, m_cTimeGain, "TimeGainColor", "FgColor");
    CPanelAnimationVar(Color, m_cTimeLoss, "TimeLossColor", "FgColor");

    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "HudHintTextSmall");

    CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "1",
        "proportional_float");
    CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2",
        "proportional_float");


private:
    KeyValues *m_kvBestTimeBuffer, *m_kvBestTime;
    CUtlVector<float> m_vecBestTimes, m_vecBestSpeeds;

    char stLocalized[BUFSIZELOCL], timeLocalized[BUFSIZELOCL], compareLocalized[BUFSIZELOCL];

};

DECLARE_HUDELEMENT(C_RunComparisons);


C_RunComparisons::C_RunComparisons(const char* pElementName) : CHudElement(pElementName),
Panel(g_pClientMode->GetViewport(), "CHudCompare")
{
    ListenForGameEvent("timer_state");
    SetProportional(true);
    SetKeyBoardInputEnabled(false);//MOM_TODO: will we want keybinds? Hotkeys?
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_kvBestTime = nullptr;
    m_kvBestTimeBuffer = nullptr;
}

C_RunComparisons::~C_RunComparisons()
{
    UnloadComparisons();
}

void C_RunComparisons::FireGameEvent(IGameEvent* event)
{
    if (!Q_strcmp(event->GetName(), "timer_state"))//This is insuring, even though we register for only this event...?
    {
        bool started = event->GetBool("is_running", false);
        if (started)
        {
            LoadComparisons();
        } else
        {
            UnloadComparisons();
        }
    }
}

void C_RunComparisons::LoadComparisons()
{
    //MOM_TODO: Allow replays to compare? If so we'd need interval_per_tick and run flags to be passed here.
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    const char* szMapName = g_pGameRules ? g_pGameRules->MapName() : nullptr;
    if (szMapName && pPlayer)
    {
        m_kvBestTimeBuffer = new KeyValues(szMapName);
        m_kvBestTime = mom_UTIL->GetBestTime(m_kvBestTimeBuffer, szMapName, gpGlobals->interval_per_tick, pPlayer->m_iRunFlags);
        if (m_kvBestTime)
        {
            mom_UTIL->GetBestStageTimes(m_kvBestTime, &m_vecBestTimes);
            mom_UTIL->GetBestStageSpeeds(m_kvBestTime, &m_vecBestSpeeds);
        }
    }
}

void C_RunComparisons::UnloadComparisons()
{
    if (m_kvBestTimeBuffer) m_kvBestTimeBuffer->deleteThis();
    m_kvBestTime = nullptr;
    m_kvBestTimeBuffer = nullptr;
    m_vecBestTimes.RemoveAll();
    m_vecBestSpeeds.RemoveAll();
}

void C_RunComparisons::OnThink()
{
}

void C_RunComparisons::Init()
{
    if (gameeventmanager)
        gameeventmanager->AddListener(this, "timer_state", false);

    //LOCALIZE STUFF HERE
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stLocalized);
    LOCALIZE_TOKEN(Time, "#MOM_RunTime", timeLocalized);
    LOCALIZE_TOKEN(Compare, "#MOM_Compare_Against", compareLocalized);
}

void C_RunComparisons::Reset()
{
    //I don't know what to do here, this is called each spawn?

    //MOM_TODO: UnloadComparisons() ?
}

//NOTE: Since we're looking BACKWARDS at stages, the int stage passed here is the stage IN QUESTION,
//NOT THE CURRENT STAGE!!!! So let's say we pass stage "3" as the stage we want the time comparison for.
//We'll be comparing using the StageEnterTime for stage 4 (perhaps the current stage, perhaps not) for both the PB and 
//the current run to compare. If this gets changed to StageTime, the calculation shifts one int left!
//We're going to look at the enter time for stage 4 (stage + 1) and subtract it from the enter time for stage 4 in PB,
//and since m_vecBestTimes does not store anything special in index 0, we keep it as stage to get the enter time for stage 4.
void C_RunComparisons::GetTimeString(int stage, char *ansiBufferOut, Color *timeCompareColorOut)
{
    //Local PB comparison
    if (!m_vecBestTimes.IsEmpty())
    {
        //MOM_TODO: Check the "Compare against time spent on stage (StageTime) OR time spent on map (StageEnterTime)" convar
        //Get the time difference in seconds.
        float diff = g_MOMEventListener->m_flStageEnterTime[stage + 1] - m_vecBestTimes[stage];//Read NOTE above method

        //Are we losing time compared to the run? 
        //If diff > 0, that means you're falling behind (losing time to) your PB!
        bool losingTime = diff > 0;//MOM_TODO: what if the diff == 0? (probably unlikely)

        //Set text color based on the results
        timeCompareColorOut->SetRawColor((losingTime ? m_cTimeLoss : m_cTimeGain).GetRawColor());
        
        //Get ready for string building
        char tempANSITimeOutput[BUFSIZETIME];// , tempANSIStringOutput[BUFSIZELOCL];

        //Format the time for displaying
        mom_UTIL->FormatTime(diff, tempANSITimeOutput);

        //Add the + or - into the time, and put into the ANSI output array
        Q_snprintf(ansiBufferOut, BUFSIZELOCL,
            "(%c %s)",
            losingTime ? '+' : '-', //Self explanatory
            tempANSITimeOutput);//Stage comparison time
    }
}

void C_RunComparisons::Paint()
{
    //MOM_TODO: Determine the max number of stages to be shown on the panel.
    //MOM_TODO: Determine size of panel, affects above.
    //MOM_TODO: Make panel scale to amount of stages. Linear maps will have checkpoints.

    //Get player current stage
    int currentStage = g_MOMEventListener->m_iCurrentStage;

    //We want to create a "buffer" of stages. The very last stage should show
    //full comparisons, and be the most bottom one. However, the stages before that need
    //to show time comparisons next to them. How I think it should go:

    //Comparing against: (run comparing against: usually PB or WR, could be any run?)
    //Stage 1 (+/- XX:XX.XX)     ----->  Stage X - 4
    //Stage 2 (+/- XX:XX.XX)     ----->  Stage X - 3
    //Stage 3 (+/- XX:XX.XX)     ----->  Stage X - 2
    //Stage 4                    ----->  Stage X - 1     where "X" is the current stage
    //  Time  (+/- XX:XX.XX)
    //  Vel   (+/- XXX.XX)
    //  Sync? etc

    surface()->DrawSetTextFont(m_hTextFont);

    //Print "Comparing against: X"
    char fullCompareString[BUFSIZELOCL];
    Q_snprintf(fullCompareString, BUFSIZELOCL, "%s%s", 
        compareLocalized, //"Compare against: "
        "PB");//MOM_TODO: Change this to a class variable determined in LoadComparisons

    wchar_t compareUnicode[BUFSIZELOCL];
    ANSI_TO_UNICODE(fullCompareString, compareUnicode);
    
    surface()->DrawSetTextPos(text_xpos, text_ypos);
    surface()->DrawPrintText(compareUnicode, wcslen(compareUnicode));


    int yToIncrementBy = surface()->GetFontTall(m_hTextFont) + 2;//+2 for padding
    int Y = text_ypos + yToIncrementBy;


    for (int i = 1; i < currentStage; i++, Y += yToIncrementBy)
    {
        //We need a buffer. We only want the last STAGE_BUFFER amount of
        //stages. (So if there's 20 stages, we only show the last X stages, not all.)
        if (i > (currentStage - STAGE_BUFFER))
        {
            char stageString[BUFSIZELOCL];
            wchar_t stageStringUnicode[BUFSIZELOCL];

            Q_snprintf(stageString, BUFSIZELOCL, "%s %i ",
                stLocalized, // "Stage" localization
                i); // Stage number

            ANSI_TO_UNICODE(stageString, stageStringUnicode);

            char timeComparisonString[BUFSIZELOCL];
            wchar_t timeComparisonStringUnicode[BUFSIZELOCL];
            Color comparisonColor = GetFgColor();

            if (i == (currentStage - 1))
            {
                //Very last stage, we want everything:

                //print "Stage ## "
                surface()->DrawSetTextColor(GetFgColor());
                surface()->DrawSetTextPos(text_xpos, Y);
                surface()->DrawPrintText(stageStringUnicode, wcslen(stageStringUnicode));
                //Move down a line
                Y += yToIncrementBy;

                //print " (\t) Time: "
                char timeStringANSI[BUFSIZELOCL];
                wchar_t timeStringUnicode[BUFSIZELOCL];

                Q_snprintf(timeStringANSI, BUFSIZELOCL, "    %s",
                    timeLocalized, // "Stage" localization
                    i); // Stage number

                ANSI_TO_UNICODE(timeStringANSI, timeStringUnicode);

                surface()->DrawSetTextPos(text_xpos, Y);
                //Color is still the same
                surface()->DrawPrintText(timeStringUnicode, wcslen(timeStringUnicode));

                //print "            (+/- XX:XX.XX)" with color
                int newXPos = text_xpos //Base starting X pos
                    + UTIL_ComputeStringWidth(m_hTextFont, timeStringANSI) //"    Time: "
                    + 2;//Padding


                //MOM_TODO: update this to be "GetComparisonForStage()" which
                //returns a struct full of the data (strings, colors) we're going to need to display,
                //and only display based on the convar setting
                GetTimeString(i, timeComparisonString, &comparisonColor);
                ANSI_TO_UNICODE(timeComparisonString, timeComparisonStringUnicode);
                surface()->DrawSetTextColor(comparisonColor);
                surface()->DrawSetTextPos(newXPos, Y);
                surface()->DrawPrintText(timeComparisonStringUnicode, wcslen(timeComparisonStringUnicode));

                //print vel
                //MOM_TODO
                //print sync?
                //MOM_TODO
                //print jumps/strafes?
                //MOM_TODO
            }
            else
            {
                //It's a stage before the very last one we've been to.
                //print "Stage ## "
                surface()->DrawSetTextColor(GetFgColor());
                surface()->DrawSetTextPos(text_xpos, Y);
                surface()->DrawPrintText(stageStringUnicode, wcslen(stageStringUnicode));

                //print "          (+/- XX:XX.XX)" with colorization
                int newXPos = text_xpos //Base starting X pos
                    + UTIL_ComputeStringWidth(m_hTextFont, stageString)//"Stage ## "
                    + 2; //Padding
                
                GetTimeString(i, timeComparisonString, &comparisonColor);
                ANSI_TO_UNICODE(timeComparisonString, timeComparisonStringUnicode);
                surface()->DrawSetTextColor(comparisonColor);
                surface()->DrawSetTextPos(newXPos, Y);
                surface()->DrawPrintText(timeComparisonStringUnicode, wcslen(timeComparisonStringUnicode));
            }
        } 
    }
}