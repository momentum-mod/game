#pragma once

#include "cbase.h"

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "menu.h"
#include "time.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/pch_vgui_controls.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "vgui_helpers.h"
#include "mom_shareddefs.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "mom_event_listener.h"
#include "util/mom_util.h"

using namespace vgui;

class CHudMapFinishedDialog : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudMapFinishedDialog, EditablePanel);

public:
    CHudMapFinishedDialog(const char *pElementName);
    ~CHudMapFinishedDialog();

    bool ShouldDraw() OVERRIDE;
    void Paint() OVERRIDE;
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void SetVisible(bool) OVERRIDE;
    void FireGameEvent(IGameEvent*) OVERRIDE;

    bool IsBuildGroupEnabled() OVERRIDE { return false; }

    void OnMousePressed(MouseCode code) OVERRIDE;

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;

protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

private:
    wchar_t m_pwCurrentPageOverall[BUFSIZELOCL];
    wchar_t m_pwCurrentPageZoneNum[BUFSIZELOCL];
    wchar_t m_pwOverallTime[BUFSIZELOCL];
    wchar_t m_pwZoneTime[BUFSIZELOCL];
    wchar_t m_pwZoneEnterTime[BUFSIZELOCL];
    wchar_t m_pwVelAvg[BUFSIZELOCL];//Used for both overall and zones
    wchar_t m_pwVelMax[BUFSIZELOCL];//Used for both overall and zones
    wchar_t m_pwVelZoneEnter[BUFSIZELOCL];//MOM_TODO: This may not exist for linear maps
    wchar_t m_pwVelZoneExit[BUFSIZELOCL];
    wchar_t m_pwSync1Overall[BUFSIZELOCL];
    wchar_t m_pwSync1Zone[BUFSIZELOCL];
    wchar_t m_pwSync2Overall[BUFSIZELOCL];
    wchar_t m_pwSync2Zone[BUFSIZELOCL];
    wchar_t m_pwJumpsOverall[BUFSIZELOCL];
    wchar_t m_pwJumpsZone[BUFSIZELOCL];
    wchar_t m_pwStrafesOverall[BUFSIZELOCL];
    wchar_t m_pwStrafesZone[BUFSIZELOCL];
    wchar_t m_pwRunSavedLabel[BUFSIZELOCL];
    wchar_t m_pwRunNotSavedLabel[BUFSIZELOCL];
    wchar_t m_pwRunUploadedLabel[BUFSIZELOCL];
    wchar_t m_pwRunNotUploadedLabel[BUFSIZELOCL];

    char m_pszEndRunTime[BUFSIZETIME];
    char m_pszRepeatToolTipMap[BUFSIZELOCL];
    char m_pszRepeatToolTipReplay[BUFSIZELOCL];
    char m_pszPlayReplayToolTip[BUFSIZELOCL];
    char m_pszRightArrowToolTip[BUFSIZELOCL];
    char m_pszLeftArrowToolTip[BUFSIZELOCL];

    ImagePanel *m_pPlayReplayButton;
    ImagePanel *m_pClosePanelButton;
    ImagePanel *m_pRepeatButton;
    ImagePanel *m_pNextZoneButton;
    ImagePanel *m_pPrevZoneButton;
    Label *m_pDetachMouseLabel;
    Label *m_pCurrentZoneLabel;
    Label *m_pZoneOverallTime;//Also known as "Zone Time"
    Label *m_pZoneEnterTime;
    Label *m_pZoneJumps;
    Label *m_pZoneStrafes;
    Label *m_pZoneVelEnter;
    Label *m_pZoneVelExit;
    Label *m_pZoneVelAvg;
    Label *m_pZoneVelMax;
    Label *m_pZoneSync1;
    Label *m_pZoneSync2;
    Label *m_pRunSaveStatus;
    Label *m_pRunUploadStatus;

    CMomRunStats* m_pRunStats;
    CMOMRunEntityData *m_pRunData;

    bool m_bRunSaved, m_bRunUploaded, m_bIsGhost;

    int m_iCurrentPage, m_iVelocityType;
    int m_iMaxPageTitleWidth, m_iCurrentZoneOrigX;
};