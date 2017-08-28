#pragma once

#include "cbase.h"
#include "run/run_compare.h"
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

using namespace vgui;

class C_RunComparisons : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_RunComparisons, Panel);

public:
    C_RunComparisons();
    C_RunComparisons(const char* pElementName);
    ~C_RunComparisons();

    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void Paint() OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void OnTick() OVERRIDE;

    void FireGameEvent(IGameEvent *event) OVERRIDE;

    void LoadComparisons();
    void LoadBogusComparisons();
    bool LoadedComparison() const
    {
        return m_bLoadedComparison;
    }
    void UnloadComparisons();
    void UnloadBogusComparisons();
    void DrawComparisonString(ComparisonString_t, int stage, int Ypos);
    void GetComparisonString(ComparisonString_t type, CMomRunStats *pStats, int zone, char *ansiActualBufferOut, char *ansiCompareBufferOut, Color *compareColorOut);
    void GetDiffColor(float diff, Color *into, bool positiveIsGain);
    int GetMaximumTall();
    void SetMaxWide(int);

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        Panel::ApplySchemeSettings(pScheme);
        m_hTextFont = pScheme->GetFont("HudHintTextSmall", true);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_cGain = GetSchemeColor("MOM.Compare.Gain", pScheme);
        m_cLoss = GetSchemeColor("MOM.Compare.Loss", pScheme);
        m_cTie = GetSchemeColor("MOM.Compare.Tie", pScheme);
        GetSize(m_iDefaultWidth, m_iDefaultTall);//gets "wide" and "tall" from scheme .res file
        m_iMaxWide = m_iDefaultWidth;
        GetPos(m_iDefaultXPos, m_iDefaultYPos);//gets "xpos" and "ypos" from scheme .res file
    }

    //Bogus Pulse is a flag-based variable, using the run_compare enum. Allows for multiple parsing.
    void SetBogusPulse(int i)
    {
        m_nCurrentBogusPulse |= i;
    }

    void SetPanelSize(int wide, int tall)
    {
        SetSize(wide, tall);
        PostActionSignal(new KeyValues("OnSizeChange", "wide", wide, "tall", tall));
    }

    int GetCurrentZone() const
    {
        return m_bLoadedBogusComparison ? MAX_STAGES - 1 : m_iCurrentZone;
    }

    void ClearBogusPulse()
    {
        m_nCurrentBogusPulse = 0;
    }

    bool LoadedComparison()
    {
        return m_bLoadedComparison || m_bLoadedBogusComparison;
    }

    RunCompare_t *GetRunComparisons() const
    {
        return m_bLoadedBogusComparison ? m_rcBogusComparison : m_rcCurrentComparison;
    }

    CMomRunStats *GetRunStats() const
    {
        return m_bLoadedBogusComparison ? m_pBogusRunStats : m_pRunStats;
    }

protected:
    Color m_cGain, m_cLoss, m_cTie;
    HFont m_hTextFont;

    //Number of pixels between each component of the comparison panel,
    //given mom_comparisons_format_output is 1
    CPanelAnimationVar(int, format_spacing, "format_spacing", "2");

    CPanelAnimationVar(float, bogus_alpha, "BogusAlpha", "255");

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

    int m_iDefaultWidth, m_iDefaultTall, m_iDefaultXPos, m_iDefaultYPos;
    int m_iMaxWide, m_iWidestLabel, m_iWidestValue;
    int m_iCurrentZone, m_iCurrentEntIndex;
    bool m_bLoadedComparison, m_bLoadedBogusComparison;
    RunCompare_t *m_rcCurrentComparison, *m_rcBogusComparison;
    //m_pRunStats points to the player's/bot's CMomRunStats::data member, but the bogus one needs its own data.
    CMomRunStats *m_pRunStats, *m_pBogusRunStats;
    CMomRunStats::data m_bogusData;
    int m_nCurrentBogusPulse;

};

//Really hacky way to interface this hud element, as opposed to calling the gHUD.FindElement everywhere
static C_RunComparisons *GetComparisons()
{
    static C_RunComparisons *s_runcompare;
    if (!s_runcompare)
        s_runcompare = dynamic_cast<C_RunComparisons *>(gHUD.FindElement("CHudCompare"));
    return s_runcompare;
}

#define g_MOMRunCompare GetComparisons()