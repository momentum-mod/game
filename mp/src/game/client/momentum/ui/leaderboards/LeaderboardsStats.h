#pragma once

#include "vgui_controls/EditablePanel.h"

class CAvatarImagePanel;

class CLeaderboardsStats : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CLeaderboardsStats, EditablePanel);
    CLeaderboardsStats(Panel *pParent);

    void LoadData(bool bFullUpdate);

    void NeedsUpdate() { m_bNeedsUpdate = true; }

    void OnPlayerStats(KeyValues *kv);

protected:
    void UpdatePlayerAvatarStandalone();

private:
    CAvatarImagePanel *m_pPlayerAvatar;
    vgui::Label *m_pPlayerName;
    vgui::Label *m_pPlayerMapRank;
    vgui::Label *m_pPlayerLevel;
    vgui::Label *m_pPlayerRankXP;
    vgui::Label *m_pPlayerCosXP;
    vgui::Label *m_pMapsCompleted;
    vgui::Label *m_pRunsSubmitted;
    vgui::Label *m_pTotalJumps;
    vgui::Label *m_pTotalStrafes;

    float m_flLastUpdate;
    bool m_bLoadedLocalPlayerAvatar;
    bool m_bNeedsUpdate;
};
