#pragma once

#include <vgui_controls/Panel.h>
#include <hudelement.h>
#include "run/run_compare.h"

class C_MomentumPlayer;
class C_MomRunEntityData;

class C_RunComparisons : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(C_RunComparisons, Panel);

public:
    C_RunComparisons(const char* pElementName, Panel *pParent = nullptr);
    ~C_RunComparisons();

    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void Paint() OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdown() OVERRIDE;
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
    void GetDiffColor(float diff, Color *into, bool positiveIsGain = true);
    int GetMaximumTall();
    void SetMaxWide(int);

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

    //Bogus Pulse is a flag-based variable, using the run_compare enum. Allows for multiple parsing.
    void SetBogusPulse(int i);

    void SetPanelSize(int wide, int tall);

    int GetCurrentZone() const;

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
    vgui::HFont m_hTextFont;

    //Number of pixels between each component of the comparison panel,
    //given mom_comparisons_format_output is 1
    CPanelAnimationVar(int, format_spacing, "format_spacing", "2");
    CPanelAnimationVar(float, bogus_alpha, "BogusAlpha", "255");
    CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "1", "proportional_float");
    CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");


private:
    wchar_t m_wStage[BUFSIZELOCL], m_wCheckpoint[BUFSIZELOCL];
    char compareLocalized[BUFSIZELOCL],
        stageTimeLocalized[BUFSIZELOCL], overallTimeLocalized[BUFSIZELOCL],
        velocityAvgLocalized[BUFSIZELOCL], velocityMaxLocalized[BUFSIZELOCL],
        velocityStartLocalized[BUFSIZELOCL], velocityExitLocalized[BUFSIZELOCL],
        sync1Localized[BUFSIZELOCL], sync2Localized[BUFSIZELOCL],
        jumpsLocalized[BUFSIZELOCL], strafesLocalized[BUFSIZELOCL];

    int m_iDefaultWidth, m_iDefaultTall, m_iDefaultXPos, m_iDefaultYPos;
    int m_iMaxWide, m_iWidestLabel, m_iWidestValue;
    bool m_bLoadedComparison, m_bLoadedBogusComparison;
    RunCompare_t *m_rcCurrentComparison, *m_rcBogusComparison;
    //m_pRunStats points to the player's/bot's CMomRunStats::data member, but the bogus one needs its own data.
    CMomRunStats *m_pRunStats, *m_pBogusRunStats;
    C_MomRunEntityData *m_pRunData;
    int m_nCurrentBogusPulse;

    int m_iLoadedRunFlags; // Loaded comparison's run flags
    float m_fLoadedTickRate; // Loaded comparison's tick rate

    ConVarRef m_cvarVelType;
};

extern C_RunComparisons *g_pMOMRunCompare;