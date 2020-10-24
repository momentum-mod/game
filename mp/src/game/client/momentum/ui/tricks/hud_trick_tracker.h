#pragma once

#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Label.h"

class TrickStepLabel : public vgui::Label
{
public:
    DECLARE_CLASS_SIMPLE(TrickStepLabel, Label);

    TrickStepLabel(Panel *pParent, const char *pszStepName);

    void SetCompleted(bool bComplete) { m_bCompleted = bComplete; }

protected:
    void PerformLayout() override;
    void ApplySchemeSettings(vgui::IScheme* pScheme) override;
    void OnThink() override;

private:
    bool m_bCompleted;

    Color m_cCompletedColor, m_cDefaultColor;
};

class TrickTrackerHUD : public CHudElement, public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(TrickTrackerHUD, EditablePanel);
    TrickTrackerHUD(const char *pElementName);

protected:
    void Reset() override;

    void FireGameEvent(IGameEvent* event) override;
    void PerformLayout() override;
    bool ShouldDraw() override;
    void LevelShutdown() override;

private:
    void PopulateTrickStepLabels();
    void UpdateTrickStepLabels();
    void ClearTrickStepLabels();

    int m_iTrackedTrick;
    vgui::Label *m_pPathNameLabel;

    CUtlVector<TrickStepLabel*> m_vecTrickSteps;
};