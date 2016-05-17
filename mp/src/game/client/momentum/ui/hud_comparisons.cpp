#include "cbase.h"
#include "hud_comparisons.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

// Overall visibility
static MAKE_TOGGLE_CONVAR(mom_comparisons, "1", FLAG_HUD_CVAR, "Shows the run comparison panel. 0 = OFF, 1 = ON");

// Max stages
// MOM_TODO: 64 stages max is a lot, perhaps reduce it to like 10? But you know people and their customization
// options...
static MAKE_CONVAR(mom_comparisons_max_stages, "4", FLAG_HUD_CVAR,
                   "Max number of stages to show on the comparison panel.", 0, 64);

// Format the output to look pretty?
static MAKE_TOGGLE_CONVAR(mom_comparisons_format_output, "1", FLAG_HUD_CVAR,
                          "Toggles formatting the comparisons panel to look nice. 0 = OFF, 1 = ON");

// Time
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_type, "0", FLAG_HUD_CVAR,
                          "Time comparison type: \n0 = overall split time (compare "
                          "against time taken to get to the stage)\n1 = stage time "
                          "(compare against time spent on stage)");
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_show_overall, "1", FLAG_HUD_CVAR,
                          "Toggle showing comparison to overall time for the run. 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_time_show_perstage, "0", FLAG_HUD_CVAR,
                          "Toggle showing comparison to time spent on stage. 0 = OFF, 1 = ON");

// Velocity
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show, "1", FLAG_HUD_CVAR,
                          "Toggle showing velocity comparisons: 0 = OFF, 1 = ON"); // Overall vis
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_type, "0", FLAG_HUD_CVAR,
                          "Velocity comparison type: \n0 = Velocity including Z-axis (3D)\n1 = Velocity without Z axis "
                          "(horizontal velocity)"); // Horizontal/3D
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_avg, "1", FLAG_HUD_CVAR,
                          "Toggle showing average velocity. 0 = OFF, 1 = ON"); // avg vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_max, "1", FLAG_HUD_CVAR,
                          "Toggle showing max stage velocity. 0 = OFF, 1 = ON"); // max vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_enter, "1", FLAG_HUD_CVAR,
                          "Toggle showing stage enter velocity. 0 = OFF, 1 = ON"); // enter vel
static MAKE_TOGGLE_CONVAR(mom_comparisons_vel_show_exit, "1", FLAG_HUD_CVAR,
                          "Toggle showing stage exit velocity. 0 = OFF, 1 = ON"); // exit vel

// Sync
static MAKE_TOGGLE_CONVAR(mom_comparisons_sync_show, "0", FLAG_HUD_CVAR,
                          "Toggle showing sync comparisons: 0 = OFF, 1 = ON"); // Overall vis
static MAKE_TOGGLE_CONVAR(
    mom_comparisons_sync_show_sync1, "0", FLAG_HUD_CVAR,
    "Toggle showing average stage Sync1 (perfect strafe ticks / total strafe ticks). 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_sync_show_sync2, "0", FLAG_HUD_CVAR,
                          "Toggle showing average stage Sync2 (accel ticks / total strafe ticks). 0 = OFF, 1 = ON");

// Keypress
static MAKE_TOGGLE_CONVAR(mom_comparisons_jumps_show, "1", FLAG_HUD_CVAR,
                          "Toggle showing total stage jumps comparison. 0 = OFF, 1 = ON");
static MAKE_TOGGLE_CONVAR(mom_comparisons_strafe_show, "1", FLAG_HUD_CVAR,
                          "Toggle showing total stage strafes comparison. 0 = OFF, 1 = ON");

DECLARE_NAMED_HUDELEMENT(C_RunComparisons, CHudCompare);

C_RunComparisons::C_RunComparisons(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudCompare")
{
    ListenForGameEvent("timer_state");
    SetProportional(true);
    SetKeyBoardInputEnabled(false); // MOM_TODO: will we want keybinds? Hotkeys?
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_iCurrentStage = 0;
    m_bLoadedComparison = false;
    m_iWidestLabel = 0;
    m_iWidestValue = 0;
}

C_RunComparisons::~C_RunComparisons() { UnloadComparisons(); }

void C_RunComparisons::Init()
{
    // LOCALIZE STUFF HERE
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

bool C_RunComparisons::ShouldDraw()
{
    return mom_comparisons.GetBool() && m_bLoadedComparison && CHudElement::ShouldDraw() && g_MOMEventListener &&
           g_MOMEventListener->m_bTimerIsRunning;
}

void C_RunComparisons::Reset()
{
    UnloadComparisons();
    m_iMaxWide = m_iDefaultWidth;
    m_iWidestLabel = 0;
    m_iWidestValue = 0;
}

void C_RunComparisons::FireGameEvent(IGameEvent *event)
{
    if (!Q_strcmp(event->GetName(), "timer_state")) // This is insuring, even though we register for only this event...?
    {
        bool started = event->GetBool("is_running", false);
        if (started)
        {
            LoadComparisons();
        }
        else
        {
            UnloadComparisons();
        }
    }
}

void C_RunComparisons::LoadComparisons()
{
    UnloadComparisons();
    // MOM_TODO: Allow replays to compare? If so we'd need interval_per_tick and run flags to be passed here.
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    const char *szMapName = g_pGameRules ? g_pGameRules->MapName() : nullptr;
    if (szMapName && pPlayer)
    {
        m_rcCurrentComparison = new RunCompare_t();
        m_bLoadedComparison = mom_UTIL->GetRunComparison(szMapName, gpGlobals->interval_per_tick, pPlayer->m_iRunFlags,
                                                         m_rcCurrentComparison);
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

    if (!mom_comparisons_time_show_overall.GetBool() && !mom_comparisons_time_show_perstage.GetBool())
    {
        // Uh oh, both overall and perstage were turned off, let's turn back on the one they want to compare
        bool showStage = mom_comparisons_time_type.GetBool(); // 0 = overall, 1 = perstage
        if (showStage)
        {
            mom_comparisons_time_show_perstage.SetValue(1);
        }
        else
        {
            mom_comparisons_time_show_overall.SetValue(1);
        }
    }
}

// Gets the maximum tall that is currently possible. (Dynamic sizing)
int C_RunComparisons::GetMaximumTall()
{
    int toReturn = 0;
    int fontTall = surface()->GetFontTall(m_hTextFont) + 2; // font tall and padding
    toReturn += fontTall;                                   // Comparing against: (run)
    int stageBuffer = mom_comparisons_max_stages.GetInt();
    int lowerBound = m_iCurrentStage - stageBuffer;

    for (int i = 1; i < m_iCurrentStage; i++)
    {
        // Note: Say our current stage is 5 and our buffer is 4. We don't look at stage 5,
        // and the lower bound becomes 1. If the check was i > lowerBound, stage 1 would be ignored,
        // and the panel would thus only show 3 stages (2, 3, and 4). So it must be >=.
        if (i >= lowerBound)
        {
            if (i == (m_iCurrentStage - 1))
            {
                // Add everything that the user compares.
                // Time
                // Note: They actually could show both, but one is always on
                if (mom_comparisons_time_show_overall.GetBool())
                    toReturn += fontTall;
                if (mom_comparisons_time_show_perstage.GetBool())
                    toReturn += fontTall;
                // Vel
                if (mom_comparisons_vel_show.GetBool())
                {
                    if (mom_comparisons_vel_show_avg.GetBool())
                        toReturn += fontTall;
                    if (mom_comparisons_vel_show_max.GetBool())
                        toReturn += fontTall;
                    if (mom_comparisons_vel_show_enter.GetBool())
                        toReturn += fontTall;
                    if (mom_comparisons_vel_show_exit.GetBool())
                        toReturn += fontTall;
                }
                // Sync
                if (mom_comparisons_sync_show.GetBool())
                {
                    if (mom_comparisons_sync_show_sync1.GetBool())
                        toReturn += fontTall;
                    if (mom_comparisons_sync_show_sync2.GetBool())
                        toReturn += fontTall;
                }
                // Keypress
                if (mom_comparisons_jumps_show.GetBool())
                    toReturn += fontTall;
                if (mom_comparisons_strafe_show.GetBool())
                    toReturn += fontTall;
            }
            // Stage ## (every stage on the panel has this)
            toReturn += fontTall;
        }
    }

    return toReturn + 2;
}

void C_RunComparisons::GetDiffColor(float diff, Color *into, bool positiveIsGain = true)
{
    int gainColor = positiveIsGain ? m_cGain.GetRawColor() : m_cLoss.GetRawColor();
    int lossColor = positiveIsGain ? m_cLoss.GetRawColor() : m_cGain.GetRawColor();
    int rawColor;
    if (mom_UTIL->FloatEquals(diff, 0.0f))
        rawColor = m_cTie.GetRawColor();
    else if (diff > 0.0f)
        rawColor = gainColor;
    else
        rawColor = lossColor;

    into->SetRawColor(rawColor);
}

// If you pass null to any of the pointer args, they will not be touched. This allows for
// only obtaining the actual, only obtaining the comparison, or only obtaining the color.
void C_RunComparisons::GetComparisonString(ComparisonString_t type, int stage, char *ansiActualBufferOut,
                                           char *ansiCompareBufferOut, Color *compareColorOut)
{
    int velType = mom_comparisons_vel_type.GetInt(); // Type of velocity comparison we're making (3D vs Horizontal)
    float diff = 0.0f;                               // Difference between the current and the compared-to.
    float act = 0.0f;                                // Actual value that the player has for this stage.
    char tempANSITimeOutput[BUFSIZETIME],
        tempANSITimeActual[BUFSIZETIME]; // Only used for time comparisons, ignored otherwise.
    char diffChar = '\0';                // The character used for showing the diff: + or -
    // Calculate diff only if we loaded a comparison
    if (m_bLoadedComparison)
    {
        switch (type)
        {
        case TIME_OVERALL:
        case STAGE_TIME:
            // Get the time difference in seconds.
            act = type == TIME_OVERALL ? g_MOMEventListener->m_flStageEnterTime[stage + 1]
                                       : g_MOMEventListener->m_flStageTime[stage];

            diff = act - (type == TIME_OVERALL ? m_rcCurrentComparison->overallSplits[stage]
                                               : m_rcCurrentComparison->stageSplits[stage - 1]);

            // Are we losing time compared to the run?
            // If diff > 0, that means you're falling behind (losing time to) your PB!

            // Format the time for displaying
            mom_UTIL->FormatTime(act, tempANSITimeActual);
            mom_UTIL->FormatTime(diff, tempANSITimeOutput);
            break;
        case VELOCITY_AVERAGE:
            // Get the vel difference
            act = g_MOMEventListener->m_flStageVelocityAvg[stage][velType];
            diff = act -
                   m_rcCurrentComparison->stageAvgVels[velType][stage - 1]; //- 1 due to array indexing (0 is stage 1)
            break;
        case VELOCITY_EXIT:
            act = g_MOMEventListener->m_flStageExitSpeed[stage][velType];
            diff = act - m_rcCurrentComparison->stageExitVels[velType][stage - 1];
            break;
        case VELOCITY_MAX:
            act = g_MOMEventListener->m_flStageVelocityMax[stage][velType];
            diff = act - m_rcCurrentComparison->stageMaxVels[velType][stage - 1];
            break;
        case VELOCITY_ENTER:
            act = g_MOMEventListener->m_flStageEnterSpeed[stage][velType];
            diff = act - m_rcCurrentComparison->stageEnterVels[velType][stage - 1];
            break;
        case STAGE_SYNC1:
            act = g_MOMEventListener->m_flStageStrafeSyncAvg[stage];
            diff = act - m_rcCurrentComparison->stageAvgSync1[stage - 1];
            break;
        case STAGE_SYNC2:
            act = g_MOMEventListener->m_flStageStrafeSync2Avg[stage];
            diff = act - m_rcCurrentComparison->stageAvgSync2[stage - 1];
            break;
        case STAGE_JUMPS:
            act = g_MOMEventListener->m_iStageJumps[stage];
            diff = act - m_rcCurrentComparison->stageJumps[stage - 1];
            break;
        case STAGE_STRAFES:
            act = g_MOMEventListener->m_iStageStrafes[stage];
            diff = act - m_rcCurrentComparison->stageStrafes[stage - 1];
            break;
        default:
            return;
        }

        // Time and jump comparison are where positive is bad.
        bool positiveIsLoss = (type == TIME_OVERALL || type == STAGE_TIME || type == STAGE_JUMPS);

        diffChar = diff > 0.0f ? '+' : '-';

        if (compareColorOut)
        {
            if (type == STAGE_STRAFES)// Since strafes aren't really important to be above/below on
                compareColorOut->SetRawColor(m_cTie.GetRawColor());
            else
                GetDiffColor(diff, compareColorOut, !positiveIsLoss);
        }

        diff = abs(diff);
    }

    if (type == TIME_OVERALL || type == STAGE_TIME)
    {
        if (ansiActualBufferOut)
            V_snprintf(ansiActualBufferOut, BUFSIZELOCL, "%s ", tempANSITimeActual);
        if (m_bLoadedComparison && ansiCompareBufferOut)
            V_snprintf(ansiCompareBufferOut, BUFSIZELOCL, "(%c %s)", diffChar, tempANSITimeOutput);
    }
    else if (type == STAGE_JUMPS || type == STAGE_STRAFES)
    {
        if (ansiActualBufferOut)
            V_snprintf(ansiActualBufferOut, BUFSIZELOCL, "%i ", static_cast<int>(act));
        if (m_bLoadedComparison && ansiCompareBufferOut)
            V_snprintf(ansiCompareBufferOut, BUFSIZELOCL, "(%c %i)", diffChar, static_cast<int>(diff));
    }
    else
    {
        if (ansiActualBufferOut)
            V_snprintf(ansiActualBufferOut, BUFSIZELOCL, "%.3f ", act);
        if (m_bLoadedComparison && ansiCompareBufferOut)
            V_snprintf(ansiCompareBufferOut, BUFSIZELOCL, "(%c %.3f)", diffChar, diff);
    }
}

void C_RunComparisons::DrawComparisonString(ComparisonString_t string, int stage, int Ypos)
{
    Color compareColor = Color(GetFgColor());
    char actualValueANSI[BUFSIZELOCL], //The actual value of the run 
        compareTypeANSI[BUFSIZELOCL], //The label of the comparison "Velocity: " etc
        compareValueANSI[BUFSIZELOCL];// The comparison string (+/- XX)
    wchar_t actualValueUnicode [BUFSIZELOCL], //Unicode of actual value
        compareTypeUnicode[BUFSIZELOCL], //Unicode of the label of the comparison type
        compareValueUnicode[BUFSIZELOCL];//Unicode of the comparison value
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

    // Obtain the actual value, comparison string, and corresponding color
    GetComparisonString(string, stage, actualValueANSI, compareValueANSI, &compareColor);
    
    //Pad the compare type with a couple spaces in front.
    V_snprintf(compareTypeANSI, BUFSIZELOCL, "  %s", localized);

    // Convert compare type to Unicode
    ANSI_TO_UNICODE(compareTypeANSI, compareTypeUnicode);

    // Print the compare type "Velocity:"/"Stage Time:" etc
    surface()->DrawSetTextColor(GetFgColor());  // Standard text color
    surface()->DrawSetTextPos(text_xpos, Ypos); // Standard position
    surface()->DrawPrintText(compareTypeUnicode, wcslen(compareTypeUnicode));

    //Convert actual value ANSI to unicode
    ANSI_TO_UNICODE(actualValueANSI, actualValueUnicode);

    //Find the x position for the actual value and comparison value
    int newXPosActual, newXPosComparison;
    int widthOfCompareType = UTIL_ComputeStringWidth(m_hTextFont, compareTypeANSI);
    int widthOfActualValue = UTIL_ComputeStringWidth(m_hTextFont, actualValueANSI);

    //Now we need to decide if we're formatting or not.
    if (mom_comparisons_format_output.GetBool())
    {
        //We want to space the strings on the same X pos, which
        //is the highest X pos possible by normal printing standards.
        
        if (widthOfCompareType > m_iWidestLabel)
            m_iWidestLabel = widthOfCompareType;
        if (widthOfActualValue > m_iWidestValue)
            m_iWidestValue = widthOfActualValue;

        newXPosActual = text_xpos + m_iWidestLabel + format_spacing;//padding
        newXPosComparison = newXPosActual + m_iWidestValue + format_spacing;
    }
    else
    {
        newXPosActual = widthOfCompareType + 2;//default padding
        newXPosComparison = newXPosActual + widthOfActualValue + 2;
    }

    //Draw the actual value
    surface()->DrawSetTextPos(newXPosActual, Ypos);
    surface()->DrawPrintText(actualValueUnicode, wcslen(actualValueUnicode));

    //Draw the comparison value
    //But first see if this changes our max width for the panel
    SetMaxWide(newXPosComparison //X pos for the comparison 
        + UTIL_ComputeStringWidth(m_hTextFont, compareValueANSI) //Width of the compare string
        + 2);//Padding

    //Convert the comparison value to unicode
    ANSI_TO_UNICODE(compareValueANSI, compareValueUnicode);
    
    //Print the 
    surface()->DrawSetTextColor(compareColor);                                  // Set the color to gain/loss color
    surface()->DrawSetTextPos(newXPosComparison, Ypos);                                   // Set pos to calculated width X
    surface()->DrawPrintText(compareValueUnicode, wcslen(compareValueUnicode)); // print string
}

void C_RunComparisons::SetMaxWide(int newWide)
{
    if (newWide > m_iMaxWide)
        m_iMaxWide = newWide;
}

void C_RunComparisons::Paint()
{
    if (!m_rcCurrentComparison)
        return;
    int maxTall = GetMaximumTall();
    int newY = m_iDefaultYPos + (m_iDefaultTall - maxTall);
    SetPos(m_iDefaultXPos, newY); // Dynamic placement
    SetSize(m_iMaxWide, maxTall); // Dynamic sizing
    //MOM_TODO: Linear maps will have checkpoints, which rid the exit velocity stat?

    // Get player current stage
    int currentStage = m_iCurrentStage;

    // We want to create a "buffer" of stages. The very last stage should show
    // full comparisons, and be the most bottom one. However, the stages before that need
    // to show time comparisons next to them. How I think it should go:

    // Comparing against: (run comparing against: usually PB or WR, could be any run?)
    // Stage 1 (+/- XX:XX.XX)     ----->  Stage X - 4
    // Stage 2 (+/- XX:XX.XX)     ----->  Stage X - 3
    // Stage 3 (+/- XX:XX.XX)     ----->  Stage X - 2
    // Stage 4                    ----->  Stage X - 1     where "X" is the current stage
    //  Time  (+/- XX:XX.XX)
    //  Vel   (+/- XXX.XX)
    //  Sync? etc

    surface()->DrawSetTextFont(m_hTextFont);

    // Print "Comparing against: X"
    char fullCompareString[BUFSIZELOCL];
    Q_snprintf(fullCompareString, BUFSIZELOCL, "%s%s",
               compareLocalized, //"Compare against: "
               m_rcCurrentComparison->runName);

    // Check to see if this updates max width
    SetMaxWide(UTIL_ComputeStringWidth(m_hTextFont, fullCompareString));

    wchar_t compareUnicode[BUFSIZELOCL];
    ANSI_TO_UNICODE(fullCompareString, compareUnicode);

    surface()->DrawSetTextPos(text_xpos, text_ypos);
    surface()->DrawPrintText(compareUnicode, wcslen(compareUnicode));

    int yToIncrementBy = surface()->GetFontTall(m_hTextFont) + 2; //+2 for padding
    int Y = text_ypos + yToIncrementBy;

    int STAGE_BUFFER = mom_comparisons_max_stages.GetInt();
    for (int i = 1; i < currentStage; i++, Y += yToIncrementBy)
    {
        // We need a buffer. We only want the last STAGE_BUFFER amount of
        // stages. (So if there's 20 stages, we only show the last X stages, not all.)
        if (i > (currentStage - STAGE_BUFFER))
        {
            char stageString[BUFSIZELOCL];
            wchar_t stageStringUnicode[BUFSIZELOCL];

            Q_snprintf(stageString, BUFSIZELOCL, "%s %i ",
                       stLocalized, // "Stage" localization
                       i);          // Stage number

            ANSI_TO_UNICODE(stageString, stageStringUnicode);

            // print "Stage ## "
            surface()->DrawSetTextColor(GetFgColor());
            surface()->DrawSetTextPos(text_xpos, Y);
            surface()->DrawPrintText(stageStringUnicode, wcslen(stageStringUnicode));

            Color comparisonColor = Color(GetFgColor());

            if (i == (currentStage - 1))
            {
                // Very last stage, we want everything!

                // Move down a line
                Y += yToIncrementBy;

                // print Time comparison
                if (mom_comparisons_time_show_overall.GetBool())
                {
                    DrawComparisonString(TIME_OVERALL, i, Y);
                    Y += yToIncrementBy;
                }
                if (mom_comparisons_time_show_perstage.GetBool())
                {
                    DrawComparisonString(STAGE_TIME, i, Y);
                    Y += yToIncrementBy;
                }

                // print vel
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

                // print sync
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
                // print jumps
                if (mom_comparisons_jumps_show.GetBool())
                {
                    DrawComparisonString(STAGE_JUMPS, i, Y);
                    Y += yToIncrementBy;
                }
                // print strafes
                if (mom_comparisons_strafe_show.GetBool())
                {
                    DrawComparisonString(STAGE_STRAFES, i, Y);
                    Y += yToIncrementBy;
                }
            }
            else
            {
                // This is done here and not through DrawComparisonString because
                // we only need to get the time comparison string, nothing else.
                ComparisonString_t timeType = mom_comparisons_time_type.GetBool() ? STAGE_TIME : TIME_OVERALL;
                char timeComparisonString[BUFSIZELOCL];
                wchar_t timeComparisonStringUnicode[BUFSIZELOCL];

                // It's a stage before the very last one we've been to.
                // print "          (+/- XX:XX.XX)" with colorization
                int newXPos = text_xpos                                           // Base starting X pos
                              + UTIL_ComputeStringWidth(m_hTextFont, stageString) //"Stage ## "
                              + 2;                                                // Padding
                
                //Get just the comparison value, no actual value needed as it clutters up the panel
                GetComparisonString(timeType, i, nullptr, timeComparisonString, &comparisonColor);

                // See if this updates our max width.
                SetMaxWide(newXPos + UTIL_ComputeStringWidth(m_hTextFont, timeComparisonString) + 2);

                ANSI_TO_UNICODE(timeComparisonString, timeComparisonStringUnicode);
                surface()->DrawSetTextColor(comparisonColor);
                surface()->DrawSetTextPos(newXPos, Y);
                surface()->DrawPrintText(timeComparisonStringUnicode, wcslen(timeComparisonStringUnicode));
            }
        }
    }
}