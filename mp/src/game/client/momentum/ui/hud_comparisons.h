#pragma once

#include "cbase.h"
#include "util/run_compare.h"
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

    void OnThink() override;
    void Init() override;
    void Reset() override;
    void Paint() override;
    bool ShouldDraw() override;

    void FireGameEvent(IGameEvent *event) override;

    void LoadComparisons();
    bool LoadedComparison() const
    {
        return m_bLoadedComparison;
    }
    void UnloadComparisons();
    void DrawComparisonString(ComparisonString_t, int stage, int Ypos);
    void GetComparisonString(ComparisonString_t type, int stage, char *ansiActualBufferOut, char *ansiCompareBufferOut, Color *compareColorOut);
    void GetDiffColor(float diff, Color *into, bool positiveIsGain);
    int GetMaximumTall();
    void SetMaxWide(int);

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_cGain = GetSchemeColor("MOM.Compare.Gain", pScheme);
        m_cLoss = GetSchemeColor("MOM.Compare.Loss", pScheme);
        GetSize(m_iDefaultWidth, m_iDefaultTall);//gets "wide" and "tall" from scheme .res file
        m_iMaxWide = m_iDefaultWidth;
        GetPos(m_iDefaultXPos, m_iDefaultYPos);//gets "xpos" and "ypos" from scheme .res file
    }

protected:
    CPanelAnimationVar(Color, m_cGain, "GainColor", "MOM.Compare.Gain");
    CPanelAnimationVar(Color, m_cLoss, "LossColor", "MOM.Compare.Loss");
    CPanelAnimationVar(Color, m_cTie, "TieColor", "MOM.Compare.Tie")

    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "HudHintTextSmall");

    //Number of pixels between each component of the comparison panel,
    //given mom_comparisons_format_output is 1
    CPanelAnimationVar(int, format_spacing, "format_spacing", "2");

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
    int m_iCurrentStage;
    bool m_bLoadedComparison;
    RunCompare_t *m_rcCurrentComparison;

};

static C_RunComparisons *GetComparisons()
{
    static C_RunComparisons *s_runcompare;
    if (!s_runcompare)
        s_runcompare = dynamic_cast<C_RunComparisons *>(gHUD.FindElement("CHudCompare"));
    return s_runcompare;
}

#define g_MOMRunCompare GetComparisons()