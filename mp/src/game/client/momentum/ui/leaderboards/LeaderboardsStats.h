#pragma once

#include "vgui_controls/EditablePanel.h"

class CLeaderboardsStats : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CLeaderboardsStats, EditablePanel);
    CLeaderboardsStats(Panel *pParent);

    void LoadData(bool bFullUpdate);

    void NeedsUpdate() { m_bNeedsUpdate = true; }

protected:
    void UpdatePlayerAvatarStandalone();

private:
    vgui::ImagePanel *m_pPlayerAvatar;
    vgui::Label *m_pPlayerName;
    vgui::Label *m_pPlayerMapRank;
    vgui::Label *m_pPlayerPersonalBest;
    vgui::Label *m_pPlayerGlobalRank;
    vgui::Label *m_pPlayerExperience;

    float m_flLastUpdate;
    bool m_bLoadedLocalPlayerAvatar;
    bool m_bNeedsUpdate;
};
