//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#pragma once

// This class manages the (persistent) list of squelched players.
class CVoiceBanMgr
{
public:
    // Init loads the list of squelched players from disk.
    bool Init();
    // Saves the state into voice_ban.dt.
    void Save();

    bool GetPlayerBan(uint32 playerID) const;
    void SetPlayerBan(uint32 playerID, bool bSquelch);

  protected:
    void Clear();

    CUtlVector<uint32> m_BannedPlayers;
};