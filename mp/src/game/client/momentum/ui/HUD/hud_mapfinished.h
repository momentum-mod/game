#pragma once

#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include "mom_shareddefs.h"
#include "run/run_stats.h"

class C_MomRunEntityData;
class C_MomentumPlayer;

class CHudMapFinishedDialog : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudMapFinishedDialog, vgui::EditablePanel);

public:
    CHudMapFinishedDialog(const char *pElementName);
    ~CHudMapFinishedDialog();

    bool ShouldDraw() OVERRIDE;
    void Reset() OVERRIDE;
    void SetVisible(bool) OVERRIDE;
    void FireGameEvent(IGameEvent*) OVERRIDE;
    void LevelShutdown() OVERRIDE;
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;

    void SetMouseInputEnabled(bool state) OVERRIDE;

    bool IsBuildGroupEnabled() OVERRIDE { return false; }

    void OnMousePressed(vgui::MouseCode code) OVERRIDE;

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

    void SetCurrentPage(int pageNum);

    void SetRunSaved(bool bState);
    void SetRunUploaded(bool bState);
    void SetRunSubmitted(RunSubmitState_t state);

    bool ClosePanel();

protected:
    CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVar(float, m_fPanelAlpha, "PanelAlpha", "0.0"); // Used for fading

private:
    void FirePanelClosedEvent(bool bRestartingMap);

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
    wchar_t m_wXPGainCos[BUFSIZELOCL], m_wXPGainRank[BUFSIZELOCL], m_wLevelGain[BUFSIZELOCL];

    char m_pszEndRunTime[BUFSIZETIME];

    vgui::ImagePanel *m_pPlayReplayButton;
    vgui::ImagePanel *m_pClosePanelButton;
    vgui::ImagePanel *m_pRepeatButton;
    vgui::ImagePanel *m_pNextZoneButton;
    vgui::ImagePanel *m_pPrevZoneButton;
    vgui::Label *m_pMouseStateLabel;
    vgui::Label *m_pCurrentZoneLabel;
    vgui::Label *m_pZoneOverallTime;//Also known as "Zone Time"
    vgui::Label *m_pZoneEnterTime;
    vgui::Label *m_pZoneJumps;
    vgui::Label *m_pZoneStrafes;
    vgui::Label *m_pZoneVelEnter;
    vgui::Label *m_pZoneVelExit;
    vgui::Label *m_pZoneVelAvg;
    vgui::Label *m_pZoneVelMax;
    vgui::Label *m_pZoneSync1;
    vgui::Label *m_pZoneSync2;
    vgui::Label *m_pRunSaveStatus;
    vgui::Label *m_pRunUploadStatus;
    vgui::Label *m_pXPGainCosmetic, *m_pXPGainRank, *m_pLevelGain;

    CMomRunStats* m_pRunStats;
    C_MomRunEntityData *m_pRunData;

    bool m_bIsGhost;
    bool m_bCanClose;

    int m_iCurrentPage;

    ConVarRef m_cvarVelType;
};