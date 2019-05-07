//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "voice_banmgr.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BANMGR_FILEVERSION 2
#define BANMGR_FILENAME "voice_bans.dt"

bool CVoiceBanMgr::Init()
{
    CUtlBuffer buf;
    if (g_pFullFileSystem->ReadFile(BANMGR_FILENAME, "GAME", buf))
    {
        const auto version = buf.GetInt();
        if (version >= BANMGR_FILEVERSION)
        {
            const auto count = buf.GetInt();
            for (int i = 0; i < count; i++)
            {
                m_BannedPlayers.AddToTail(buf.GetUnsignedInt());
            }
        }
    }

    return true;
}

void CVoiceBanMgr::Save()
{
    // Save the file out.
    if (m_BannedPlayers.Count())
    {
        CUtlBuffer buf;
        buf.PutInt(BANMGR_FILEVERSION);
        buf.PutInt(m_BannedPlayers.Count());
        FOR_EACH_VEC(m_BannedPlayers, i)
            buf.PutUnsignedInt(m_BannedPlayers[i]);

        g_pFullFileSystem->WriteFile(BANMGR_FILENAME, "GAME", buf);
    }
}

bool CVoiceBanMgr::GetPlayerBan(uint32 playerID) const { return m_BannedPlayers.IsValidIndex(m_BannedPlayers.Find(playerID)); }

void CVoiceBanMgr::SetPlayerBan(uint32 playerID, bool bSquelch)
{
    if (bSquelch)
    {
        // Is this guy already squelched?
        if (GetPlayerBan(playerID))
            return;

        m_BannedPlayers.AddToTail(playerID);
    }
    else
    {
        m_BannedPlayers.FindAndRemove(playerID);
    }
}

void CVoiceBanMgr::Clear()
{
    m_BannedPlayers.RemoveAll();
}