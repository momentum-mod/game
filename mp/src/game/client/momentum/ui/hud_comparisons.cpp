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

#define CONVARFLAG (FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED)

#define MAKE_CONVAR(name, defaultval, desc, minVal, maxVal) \
    ConVar name(#name, defaultval, CONVARFLAG, desc, true, minVal, true, maxVal)

#define MAKE_TOGGLE_CONVAR(name, defaultval, desc) MAKE_CONVAR(name, defaultval, desc, 0, 1)

//Overall visibility
static MAKE_TOGGLE_CONVAR(mom_comparisons, "1", "Shows the run comparison panel. 0 = OFF, 1 = ON");

//Max stages
//MOM_TODO: 64 stages max is a lot, perhaps reduce it to like 10? But you know people and their customization options...
static MAKE_CONVAR(mom_comparisons_max_stages, "4", "Max number of stages to show on the comparison panel.", 0, 64);

//Time
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_type, "0",
    "Time comparison type: \n0 = overall split time (compare against time taken to get to the stage)\n1 = stage time (compare against time spent on stage)");
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_show_overall, "1", "Toggle showing comparison to overall time for the run. 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_show_perstage, "0", "Toggle showing comparison to time spent on stage. 0 = OFF, 1 = ON");

//Velocity
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show, "1", "Toggle showing velocity comparisons: 0 = OFF, 1 = ON");//Overall vis
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_type, "0",
    "Velocity comparison type: \n0 = Velocity including Z-axis (3D)\n1 = Velocity without Z axis (horizontal velocity)");//Horizontal/3D
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_avg, "1", "Toggle showing average velocity. 0 = OFF, 1 = ON");//avg vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_max, "1", "Toggle showing max stage velocity. 0 = OFF, 1 = ON");//max vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_enter, "1", "Toggle showing stage enter velocity. 0 = OFF, 1 = ON");//enter vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_exit, "1", "Toggle showing stage exit velocity. 0 = OFF, 1 = ON");//exit vel

//Sync
static MAKE_TOGGLE_CONVAR(mom_comparisons_sync_show, "0", "Toggle showing sync comparisons: 0 = OFF, 1 = ON");//Overall vis
static MAKE_TOGGLE_CONVAR(mom_comparisons_sync_show_sync1, "0",
    "Toggle showing average stage Sync1 (perfect strafe ticks / total strafe ticks). 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_sync_show_sync2, "0",
    "Toggle showing average stage Sync2 (accel ticks / total strafe ticks). 0 = OFF, 1 = ON");

//Keypress
static MAKE_TOGGLE_CONVAR(mom_comparisons_jumps_show, "1", "Toggle showing total stage jumps comparison. 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_strafe_show, "1", "Toggle showing total stage strafes comparison. 0 = OFF, 1 = ON");


class C_RunComparisons : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_RunComparisons, Panel);

private:
    struct RunCompare_t
    {
        //Name of the comparison.
        char runName[32];//MOM_TODO: determine a good size for this array. 
        //Note: we're using CUtlVectors here so we don't have to parse the stage/checkpoint number from the .tim file!
        CUtlVector<float> overallSplits,//Stage enter times (overall times)
            stageSplits,//Times spent on stages (stage time)
            stageAvgVels[2],//Average velocities for stages, 0 = 3D vels, 1 = horizontal vels
            stageMaxVels[2],//Maximum velocities for stages, 0 = 3D vels, 1 = horizontal vels
            stageEnterVels[2],//Velocity with which you enter this stage, 0 = 3D vels, 1 = horizontal vels
            stageExitVels[2],//Velocity with which you leave the stage's start, 0 = 3D vels, 1 = horizontal vels
            stageAvgSync1,//Average stage sync1
            stageAvgSync2;//Average stage sync2
        CUtlVector<int> stageJumps,//Number of jumps on this stage
            stageStrafes;//Number of strafes on this stage
    };

    enum ComparisonString_t
    {
        TIME_OVERALL = 1,
        STAGE_TIME,
        VELOCITY_AVERAGE,
        VELOCITY_MAX,
        VELOCITY_ENTER,
        VELOCITY_EXIT,
        STAGE_SYNC1,
        STAGE_SYNC2,
        STAGE_JUMPS,
        STAGE_STRAFES
    };

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
        return mom_comparisons.GetBool() && m_bLoadedComparison && CHudElement::ShouldDraw() && 
            g_MOMEventListener && g_MOMEventListener->m_bTimerIsRunning;
    }

    void FireGameEvent(IGameEvent *event) override;

    void LoadComparisons();
    void UnloadComparisons();
    void DrawComparisonString(ComparisonString_t, int stage, int Ypos);
    void GetComparisonString(ComparisonString_t type, int stage, char *ansiBufferOut, Color *compareColorOut);
    bool GetRunComparison(const char* szMapName, float tickRate, int flags, RunCompare_t *into);
    void GetDiffColor(float diff, Color *into, bool positiveIsGain);


    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_cGain = GetSchemeColor("MOM.Compare.Gain", pScheme);
        m_cLoss = GetSchemeColor("MOM.Compare.Loss", pScheme);
    }

protected:
    CPanelAnimationVar(Color, m_cGain, "GainColor", "FgColor");
    CPanelAnimationVar(Color, m_cLoss, "LossColor", "FgColor");

    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "HudHintTextSmall");

    CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "1",
        "proportional_float");
    CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2",
        "proportional_float");


private:
    char stLocalized[BUFSIZELOCL], compareLocalized[BUFSIZELOCL],
        stageTimeLocalized[BUFSIZELOCL], overallTimeLocalized[BUFSIZELOCL],
        velocityAvgLocalized[BUFSIZELOCL], velocityMaxLocalized[BUFSIZELOCL],
        velocityStartLocalized[BUFSIZELOCL], velocityExitLocalized[BUFSIZELOCL],
        sync1Localized[BUFSIZELOCL], sync2Localized[BUFSIZELOCL],
        jumpsLocalized[BUFSIZELOCL], strafesLocalized[BUFSIZELOCL];

    int m_iCurrentStage;
    bool m_bLoadedComparison;
    RunCompare_t *m_rcCurrentComparison;

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
    m_iCurrentStage = 0;
    m_bLoadedComparison = false;
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

//MOM_TODO: If this will be needed elsewhere, we will move it to mom_UTIL
bool C_RunComparisons::GetRunComparison(const char* szMapName, float tickRate, int flags, RunCompare_t *into)
{
    bool toReturn = false;
    if (into && szMapName)
    {
        char path[MAX_PATH], mapName[MAX_PATH];
        Q_snprintf(mapName, MAX_PATH, "%s%s", szMapName, EXT_TIME_FILE);
        V_ComposeFileName(MAP_FOLDER, mapName, path, MAX_PATH);
        KeyValues *kvMap = new KeyValues(szMapName);
        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            if (!kvMap->IsEmpty())
            {
                CUtlSortVector<KeyValues*, CTimeSortFunc> sortedTimes;

                FOR_EACH_SUBKEY(kvMap, kv)
                {
                    int kvflags = kv->GetInt("flags");
                    float kvrate = kv->GetFloat("rate");
                    if (kvflags == flags && mom_UTIL->FloatEquals(kvrate, tickRate, 0.001f))
                    {
                        sortedTimes.InsertNoSort(kv);
                    }
                }
                KeyValues *bestRun = nullptr;
                if (!sortedTimes.IsEmpty())
                {
                    sortedTimes.RedoSort();
                    bestRun = kvMap->FindKey(sortedTimes[0]->GetName());
                    sortedTimes.Purge();
                }

                if (bestRun)
                {
                    FOR_EACH_SUBKEY(bestRun, kv)
                    {
                        if (!Q_strnicmp(kv->GetName(), "stage", strlen("stage")))//MOM_TODO: or "checkpoint" (for linears)
                        {
                            //MOM_TODO: this may not be a PB, for now it is, but we'll load times from online.
                            //I'm thinking the name could be like "(user): (Time)"
                            Q_strcpy(into->runName, "Personal Best");
                            //Splits
                            into->overallSplits.AddToTail(kv->GetFloat("enter_time"));
                            into->stageSplits.AddToTail(kv->GetFloat("time"));
                            //Keypress
                            into->stageJumps.AddToTail(kv->GetInt("num_jumps"));
                            into->stageStrafes.AddToTail(kv->GetInt("num_strafes"));
                            //Sync
                            into->stageAvgSync1.AddToTail(kv->GetFloat("avg_sync"));
                            into->stageAvgSync2.AddToTail(kv->GetFloat("avg_sync2"));
                            //Velocity (3D and Horizontal)
                            for (int i = 0; i < 2; i++)
                            {
                                bool horizontalVel = (i == 1);
                                into->stageAvgVels[i].AddToTail(kv->GetFloat(horizontalVel ? "avg_vel_2D" : "avg_vel"));
                                into->stageMaxVels[i].AddToTail(kv->GetFloat(horizontalVel ? "max_vel_2D" : "max_vel"));
                                into->stageEnterVels[i].AddToTail(kv->GetFloat(horizontalVel ? "stage_enter_vel_2D" : "stage_enter_vel"));
                                into->stageExitVels[i].AddToTail(kv->GetFloat(horizontalVel ? "stage_exit_vel_2D" : "stage_exit_vel"));
                            }
                        }
                    }
                    DevLog("Loaded run comparisons for %s !\n", into->runName);
                    toReturn = true;
                }
            }
        }
        kvMap->deleteThis();
    }
    return toReturn;
}

void C_RunComparisons::LoadComparisons()
{
    UnloadComparisons();
    //MOM_TODO: Allow replays to compare? If so we'd need interval_per_tick and run flags to be passed here.
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    const char* szMapName = g_pGameRules ? g_pGameRules->MapName() : nullptr;
    if (szMapName && pPlayer)
    {
        m_rcCurrentComparison = new RunCompare_t();
        m_bLoadedComparison = GetRunComparison(szMapName, gpGlobals->interval_per_tick, pPlayer->m_iRunFlags, m_rcCurrentComparison);
    }
}

void C_RunComparisons::UnloadComparisons()
{
    if (m_rcCurrentComparison)
    {
        delete m_rcCurrentComparison;
        m_rcCurrentComparison = nullptr;
    }
    m_bLoadedComparison = false;
}

void C_RunComparisons::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pPlayer)
        m_iCurrentStage = pPlayer->m_iCurrentStage;
}

void C_RunComparisons::Init()
{
    //LOCALIZE STUFF HERE
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stLocalized);
    LOCALIZE_TOKEN(StageTime, "#MOM_Compare_Time_Stage", stageTimeLocalized);
    LOCALIZE_TOKEN(OverallTime, "#MOM_Compare_Time_Overall", overallTimeLocalized);
    LOCALIZE_TOKEN(Compare, "#MOM_Compare_Against", compareLocalized);
    LOCALIZE_TOKEN(VelAvg, "#MOM_Compare_Velocity_Avg", velocityAvgLocalized);
    LOCALIZE_TOKEN(VelMax, "#MOM_Compare_Velocity_Max", velocityMaxLocalized);
    LOCALIZE_TOKEN(VelEnter, "#MOM_Compare_Velocity_Enter", velocityStartLocalized);
    LOCALIZE_TOKEN(VelExit, "#MOM_Compare_Velocity_Exit", velocityExitLocalized);
    LOCALIZE_TOKEN(Sync1, "#MOM_Compare_Sync1", sync1Localized);
    LOCALIZE_TOKEN(Sync2, "#MOM_Compare_Sync2", sync2Localized);
    LOCALIZE_TOKEN(Jumps, "#MOM_Compare_Jumps", jumpsLocalized);
    LOCALIZE_TOKEN(Strafes, "#MOM_Compare_Strafes", strafesLocalized);
}

void C_RunComparisons::Reset()
{
    //I don't know what to do here, this is called each spawn?

    //MOM_TODO: UnloadComparisons() ?
}

void C_RunComparisons::GetDiffColor(float diff, Color *into, bool positiveIsGain = true)
{
    int gainColor = positiveIsGain ? m_cGain.GetRawColor() : m_cLoss.GetRawColor();
    int lossColor = positiveIsGain ? m_cLoss.GetRawColor() : m_cGain.GetRawColor();
    into->SetRawColor(diff > 0 ? gainColor : lossColor);
}


void C_RunComparisons::GetComparisonString(ComparisonString_t type, int stage, char *ansiBufferOut, Color *compareColorOut)
{
    int velType = mom_comparisons_vel_type.GetInt();//Type of velocity comparison we're making (3D vs Horizontal)
    float diff = 0.0f;//Difference between the current and the compared-to.
    char tempANSITimeOutput[BUFSIZETIME];//Only used for time comparisons, ignored otherwise.
    //Calculate diff
    switch (type)
    {
    case TIME_OVERALL: 
    case STAGE_TIME:
        //Get the time difference in seconds.
        diff = type == TIME_OVERALL ?
            g_MOMEventListener->m_flStageEnterTime[stage + 1] - m_rcCurrentComparison->overallSplits[stage] ://Read NOTE above method
            g_MOMEventListener->m_flStageTime[stage] - m_rcCurrentComparison->stageSplits[stage - 1];
        
        //Are we losing time compared to the run?
        //If diff > 0, that means you're falling behind (losing time to) your PB!
        //MOM_TODO: what if the diff == 0? (probably unlikely)

        //Format the time for displaying
        mom_UTIL->FormatTime(diff, tempANSITimeOutput);
        break;
    case VELOCITY_AVERAGE:
        //Get the vel difference
        diff = g_MOMEventListener->m_flStageVelocityAvg[stage][velType]
            - m_rcCurrentComparison->stageAvgVels[velType][stage - 1];//- 1 due to array indexing (0 is stage 1)
        break;
    case VELOCITY_EXIT:
        diff = g_MOMEventListener->m_flStageExitSpeed[stage][velType] -
            m_rcCurrentComparison->stageExitVels[velType][stage - 1];
        break;
    case VELOCITY_MAX:
        diff = g_MOMEventListener->m_flStageVelocityMax[stage][velType] -
            m_rcCurrentComparison->stageMaxVels[velType][stage - 1];
        break;
    case VELOCITY_ENTER:
        diff = g_MOMEventListener->m_flStageEnterSpeed[stage][velType] -
            m_rcCurrentComparison->stageEnterVels[velType][stage - 1];
        break;
    case STAGE_SYNC1: 
        diff = g_MOMEventListener->m_flStageStrafeSyncAvg[stage] -
            m_rcCurrentComparison->stageAvgSync1[stage - 1];
        break;
    case STAGE_SYNC2: 
        diff = g_MOMEventListener->m_flStageStrafeSync2Avg[stage] -
            m_rcCurrentComparison->stageAvgSync2[stage - 1];
        break;
    case STAGE_JUMPS: 
        diff = g_MOMEventListener->m_iStageJumps[stage] -
            m_rcCurrentComparison->stageJumps[stage - 1];
        break;
    case STAGE_STRAFES: 
        diff = g_MOMEventListener->m_iStageStrafes[stage] -
            m_rcCurrentComparison->stageStrafes[stage - 1];
        break;
    default: 
        return;
    }

    //Time, jumps, and strafe comparison are where positive is bad.
    bool positiveIsLoss = (type == TIME_OVERALL || type == STAGE_TIME || type == STAGE_JUMPS || type == STAGE_STRAFES);

    GetDiffColor(diff, compareColorOut, !positiveIsLoss);

    char diffChar = diff > 0.0f ? '+' : '-';

    diff = abs(diff);

    if (type == TIME_OVERALL || type == STAGE_TIME)
    {
        V_snprintf(ansiBufferOut, BUFSIZELOCL, "(%c %s)", diffChar, tempANSITimeOutput);
    } 
    else if (type == STAGE_JUMPS || type == STAGE_STRAFES)
    {
        V_snprintf(ansiBufferOut, BUFSIZELOCL, "(%c %i)", diffChar, static_cast<int>(diff));
    }
    else
    {
        V_snprintf(ansiBufferOut, BUFSIZELOCL, "(%c %.3f)", diffChar, diff);
    }
}

void C_RunComparisons::DrawComparisonString(ComparisonString_t string, int stage, int Ypos)
{
    Color compareColor = GetFgColor();
    char compareTypeANSI[BUFSIZELOCL], compareValueANSI[BUFSIZELOCL];
    wchar_t compareTypeUnicode[BUFSIZELOCL], compareValueUnicode[BUFSIZELOCL];
    char *localized = nullptr;
    switch (string)
    {
    case TIME_OVERALL:
    case STAGE_TIME:
        //" Overall Time: " or "  Stage Time: "
        localized = (string == TIME_OVERALL) ? overallTimeLocalized : stageTimeLocalized;
        break;
    case VELOCITY_AVERAGE: 
        localized = velocityAvgLocalized;
        break;
    case VELOCITY_MAX: 
        localized = velocityMaxLocalized;
        break;
    case VELOCITY_ENTER: 
        localized = velocityStartLocalized;
        break;
    case VELOCITY_EXIT: 
        localized = velocityExitLocalized;
        break;
    case STAGE_SYNC1: 
        localized = sync1Localized;
        break;
    case STAGE_SYNC2: 
        localized = sync2Localized;
        break;
    case STAGE_JUMPS: 
        localized = jumpsLocalized;
        break;
    case STAGE_STRAFES: 
        localized = strafesLocalized;
        break;
    default: 
        break;
    }

    if (!localized)
    {
        DevWarning("C_HudComparisons::DrawComparisonString: localized was not set!!!\n");
        return;
    }

    //Copy the value to the compareTypeANSI char, needed for the spacing.
    V_snprintf(compareTypeANSI, BUFSIZELOCL, "  %s", localized);

    //Convert compare type to Unicode
    ANSI_TO_UNICODE(compareTypeANSI, compareTypeUnicode);
    //Print the string
    surface()->DrawSetTextColor(GetFgColor());//Standard text color
    surface()->DrawSetTextPos(text_xpos, Ypos);//Standard position
    surface()->DrawPrintText(compareTypeUnicode, wcslen(compareTypeUnicode));

    //Get new X position for time comparison string
    int newXPos = text_xpos //Base starting X pos
        + UTIL_ComputeStringWidth(m_hTextFont, compareTypeANSI) //" Overall Time: " or "  Stage Time: " 
        + 2;//Padding

    //Obtain the comparison string, in this case: "(+/- XX:XX.XX)" and corresponding color
    GetComparisonString(string, stage, compareValueANSI, &compareColor);

    //Convert to unicode
    ANSI_TO_UNICODE(compareValueANSI, compareValueUnicode);
    surface()->DrawSetTextColor(compareColor);//Set the color to gain/loss color
    surface()->DrawSetTextPos(newXPos, Ypos);//Set pos to calculated width X
    surface()->DrawPrintText(compareValueUnicode, wcslen(compareValueUnicode));//print string
}

void C_RunComparisons::Paint()
{
    if (!m_rcCurrentComparison) return;

    //MOM_TODO: Determine dynamic size of panel, affected by how many stages can be shown.
    //MOM_TODO: Make panel scale to amount of stages. Linear maps will have checkpoints.

    //Get player current stage
    int currentStage = m_iCurrentStage;

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
        m_rcCurrentComparison->runName);

    wchar_t compareUnicode[BUFSIZELOCL];
    ANSI_TO_UNICODE(fullCompareString, compareUnicode);
    
    surface()->DrawSetTextPos(text_xpos, text_ypos);
    surface()->DrawPrintText(compareUnicode, wcslen(compareUnicode));


    int yToIncrementBy = surface()->GetFontTall(m_hTextFont) + 2;//+2 for padding
    int Y = text_ypos + yToIncrementBy;

    int STAGE_BUFFER = mom_comparisons_max_stages.GetInt();
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

            //print "Stage ## "
            surface()->DrawSetTextColor(GetFgColor());
            surface()->DrawSetTextPos(text_xpos, Y);
            surface()->DrawPrintText(stageStringUnicode, wcslen(stageStringUnicode));

            //char timeComparisonString[BUFSIZELOCL];
            //wchar_t timeComparisonStringUnicode[BUFSIZELOCL];
            Color comparisonColor = GetFgColor();
            ComparisonString_t timeType = mom_comparisons_time_type.GetBool() ? STAGE_TIME : TIME_OVERALL;

            if (i == (currentStage - 1))
            {
                //Very last stage, we want everything!

                //Move down a line
                Y += yToIncrementBy;

                //print Time comparison
                DrawComparisonString(timeType, i, Y);
                Y += yToIncrementBy;

                //print vel
                if (mom_comparisons_vel_show.GetBool())
                {
                    if (mom_comparisons_vel_show_avg.GetBool())
                    {
                        DrawComparisonString(VELOCITY_AVERAGE, i, Y);
                        Y += yToIncrementBy;
                    }

                    if (mom_comparisons_vel_show_max.GetBool())
                    {
                        DrawComparisonString(VELOCITY_MAX, i, Y);
                        Y += yToIncrementBy;
                    }

                    if (mom_comparisons_vel_show_exit.GetBool())
                    {
                        DrawComparisonString(VELOCITY_EXIT, i, Y);
                        Y += yToIncrementBy;
                    }

                    if (mom_comparisons_vel_show_enter.GetBool())
                    {
                        DrawComparisonString(VELOCITY_ENTER, i, Y);
                        Y += yToIncrementBy;
                    }

                }

                //print sync
                if (mom_comparisons_sync_show.GetBool())
                {
                    if (mom_comparisons_sync_show_sync1.GetBool())
                    {
                        DrawComparisonString(STAGE_SYNC1, i, Y);
                        Y += yToIncrementBy;
                    }
                    if (mom_comparisons_sync_show_sync2.GetBool())
                    {
                        DrawComparisonString(STAGE_SYNC2, i, Y);
                        Y += yToIncrementBy;
                    }
                }
                //print jumps
                if (mom_comparisons_jumps_show.GetBool())
                {
                    DrawComparisonString(STAGE_JUMPS, i, Y);
                    Y += yToIncrementBy;
                }
                //print strafes
                if (mom_comparisons_strafe_show.GetBool())
                {
                    DrawComparisonString(STAGE_STRAFES, i, Y);
                    Y += yToIncrementBy;
                }
            }
            else
            {
                //This is done here and not through DrawComparisonString because
                //we only need to get the time comparison string, nothing else.
                char timeComparisonString[BUFSIZELOCL];
                wchar_t timeComparisonStringUnicode[BUFSIZELOCL];

                //It's a stage before the very last one we've been to.         
                //print "          (+/- XX:XX.XX)" with colorization
                int newXPos = text_xpos //Base starting X pos
                    + UTIL_ComputeStringWidth(m_hTextFont, stageString)//"Stage ## "
                    + 2; //Padding
                
                GetComparisonString(timeType, i, timeComparisonString, &comparisonColor);

                //GetTimeString(i, timeComparisonString, &comparisonColor); MOM_TODO: REMOVEME
                ANSI_TO_UNICODE(timeComparisonString, timeComparisonStringUnicode);
                surface()->DrawSetTextColor(comparisonColor);
                surface()->DrawSetTextPos(newXPos, Y);
                surface()->DrawPrintText(timeComparisonStringUnicode, wcslen(timeComparisonStringUnicode));
            }
        } 
    }
}