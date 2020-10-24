#pragma once

struct UserData
{
    int m_iID;
    uint64 m_uSteamID;
    char m_szAlias[32];

    // Stats
    int m_iCurrentLevel;
    int m_iCurrentXP;

    // Game Stats
    int m_iTotalJumps;
    int m_iTotalStrafes;
    int m_iMapsCompleted;
    int m_iRunsSubmitted;

    UserData();
    bool ParseFromKV(KeyValues *pUserKv);
    void SaveToKV(KeyValues *pKvOut) const;
};

class MomentumUserData : public CAutoGameSystem, public CGameEventListener
{
public:
    MomentumUserData();

    const UserData &GetLocalUserData() const { return m_LocalUserData; }

    void AddUserDataChangeListener(vgui::VPANEL hPanel);
    void RemoveUserDataChangeListener(vgui::VPANEL hPanel);

    void FireUserDataUpdate();
protected:
    void PostInit() override;
    void Shutdown() override;

    void FireGameEvent(IGameEvent *event) override;

private:
    bool SaveLocalUserData();

    void OnUserDataReceived(KeyValues *pDataKV);

    UserData m_LocalUserData;
    CUtlVector<vgui::VPANEL> m_vecUserDataChangeListeners;
};

extern MomentumUserData *g_pUserData;