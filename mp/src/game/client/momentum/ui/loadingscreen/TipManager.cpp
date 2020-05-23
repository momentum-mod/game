#include "cbase.h"

#include "TipManager.h"

#include "filesystem.h"

#include "tier0/memdbgon.h"

struct GameModeTips
{
    CUtlStringList m_vecTips;
    const char *GetRandomTip()
    {
        if (m_vecTips.IsEmpty())
            return "#MOM_Tip_Not_Found";

        return m_vecTips.Random();
    }
};

CTipManager::CTipManager()
{
    KeyValuesAD kvTips("Tips");
    if (kvTips->LoadFromFile(g_pFullFileSystem, "resource/MomentumTips.res"))
    {
        m_vecTips.EnsureCount(GAMEMODE_COUNT);

        for (int i = 0; i < GAMEMODE_COUNT; i++)
        {
            GameModeTips *pTips = nullptr;

            const auto pGameModeTips = kvTips->FindKey(g_szGameModes[i]);
            if (pGameModeTips)
            {
                pTips = new GameModeTips;

                FOR_EACH_VALUE(pGameModeTips, kvTip)
                {
                    pTips->m_vecTips.CopyAndAddToTail(kvTip->GetString());
                }
            }

            m_vecTips[i] = pTips;
        }
    }
    else
    {
        const auto pTips = new GameModeTips;
        pTips->m_vecTips.CopyAndAddToTail("#MOM_Tip_Verify_Cache");
        m_vecTips.AddToTail(pTips);
    }
}

const char *CTipManager::GetTipForGamemode(GameMode_t gameMode)
{
    if (RandomInt(0, 1) || gameMode == GAMEMODE_UNKNOWN || m_vecTips.Count() == 1)
    {
        return m_vecTips[GAMEMODE_UNKNOWN]->GetRandomTip();
    }

    const auto pGameModeTips = m_vecTips[gameMode];
    return pGameModeTips ? pGameModeTips->GetRandomTip() : "#MOM_Tip_Not_Found";
}