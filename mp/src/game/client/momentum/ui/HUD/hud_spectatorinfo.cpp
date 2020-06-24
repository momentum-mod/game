#include "cbase.h"

#include "hud_spectatorinfo.h"

#include "c_mom_player.h"
#include "c_mom_online_ghost.h"

#include "clientmode.h"
#include "mom_shareddefs.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "steam/steam_api.h"
#include "baseviewport.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudSpectatorInfo);

static MAKE_CONVAR(mom_hud_spectator_info_show, "1", FLAG_HUD_CVAR, "Toggles showing the spectator panel. 0 = OFF, 1 = ON (when there are spectators), 2 = ALWAYS ON\n", 0, 2);
static MAKE_TOGGLE_CONVAR(mom_hud_spectator_info_show_names, "1", FLAG_HUD_CVAR, "Toggles showing the names of who is spectating you.\n");
static MAKE_CONVAR(mom_hud_spectator_info_name_count, "5", FLAG_HUD_CVAR, "Controls the max number of names to print of who is spectating you.\n0 = unlimited (as many as the panel can handle)\n", 0, 100);

CHudSpectatorInfo::CHudSpectatorInfo(const char *pName) : CHudElement(pName), BaseClass(g_pClientMode->GetViewport(), pName)
{
    SetProportional(true);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetDefLessFunc(m_mapTargetToSpectateList);
    SetDefLessFunc(m_mapSpectatorToTargetMap);

    m_CurrentSpecTargetSteamID = 0;
    m_iSpecCount = 0;
}

CHudSpectatorInfo::~CHudSpectatorInfo()
{
}

bool CHudSpectatorInfo::ShouldDraw()
{
    m_iSpecCount = 0;
    m_CurrentSpecTargetSteamID = 0;

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return false;

    const auto pUIEntity = pPlayer->GetCurrentUIEntity();
    if (pUIEntity->GetEntType() == RUN_ENT_PLAYER || pUIEntity->GetEntType() == RUN_ENT_ONLINE)
    {
        m_CurrentSpecTargetSteamID = pUIEntity->GetSteamID();

        const auto index = m_mapTargetToSpectateList.Find(m_CurrentSpecTargetSteamID);
        if (m_mapTargetToSpectateList.IsValidIndex(index))
        {
            m_iSpecCount = m_mapTargetToSpectateList[index]->m_vecSpectators.Count();
        }
    }

    const int showVal = mom_hud_spectator_info_show.GetInt();
    const bool showFromCount = showVal == 2 || (showVal == 1 && m_iSpecCount > 0);

    return showFromCount && CHudElement::ShouldDraw();
}

void CHudSpectatorInfo::Paint()
{
    char spectatorCountANSI[128];
    Q_snprintf(spectatorCountANSI, 128, "Spectators: %i", m_iSpecCount);

    wchar_t spectatorCountUnicode[128];
    ANSI_TO_UNICODE(spectatorCountANSI, spectatorCountUnicode);

    int yPos = 2;
    int fontTall = surface()->GetFontTall(m_hTextFont);
    
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(COLOR_WHITE);
    // MOM_TODO: Allow customizing this text position
    surface()->DrawSetTextPos(2, yPos);
    surface()->DrawPrintText(spectatorCountUnicode, Q_wcslen(spectatorCountUnicode));

    if (mom_hud_spectator_info_show_names.GetBool() && m_iSpecCount > 0)
    {
        const auto index = m_mapTargetToSpectateList.Find(m_CurrentSpecTargetSteamID);
        if (m_mapTargetToSpectateList.IsValidIndex(index))
        {
            CUtlVector<uint64> *pVecSpec = &m_mapTargetToSpectateList[index]->m_vecSpectators;
            int desiredCount = mom_hud_spectator_info_name_count.GetInt() == 0 ? INT_MAX : mom_hud_spectator_info_name_count.GetInt();
            for (int i = 0; i < pVecSpec->Count() && i < desiredCount; i++)
            {
                yPos += fontTall + 2;

                //MOM_TODO: Allow customizing this text position

                surface()->DrawSetTextPos(2, yPos);
                const auto specUser = pVecSpec->Element(i);

                const char *pName = SteamFriends()->GetFriendPersonaName(CSteamID(specUser));
                wchar_t pNameUnicode[MAX_PLAYER_NAME_LENGTH];
                ANSI_TO_UNICODE(pName, pNameUnicode);

                surface()->DrawPrintText(pNameUnicode, Q_wcslen(pNameUnicode));
            }
        }
    }
}

void CHudSpectatorInfo::LevelShutdown()
{
    m_mapTargetToSpectateList.PurgeAndDeleteElements();
    m_mapSpectatorToTargetMap.RemoveAll();
}

void CHudSpectatorInfo::SpectatorUpdate(const CSteamID& person, const CSteamID& target)
{
    auto index = m_mapSpectatorToTargetMap.Find(person.ConvertToUint64());
    if (m_mapSpectatorToTargetMap.IsValidIndex(index))
    {
        uint64 previousTarget = m_mapSpectatorToTargetMap[index];

        // Remove me from previous target's spectator list (if valid)
        if (previousTarget > 1)
        {
            const auto prevTargetIndex = m_mapTargetToSpectateList.Find(previousTarget);
            if (m_mapTargetToSpectateList.IsValidIndex(prevTargetIndex))
            {
                m_mapTargetToSpectateList[prevTargetIndex]->m_vecSpectators.FindAndRemove(person.ConvertToUint64());
            }
        }

        m_mapSpectatorToTargetMap[index] = target.ConvertToUint64();
    }
    else
    {
        m_mapSpectatorToTargetMap.Insert(person.ConvertToUint64(), target.ConvertToUint64());
    }

    if (target.ConvertToUint64() > 1)
    {
        index = m_mapTargetToSpectateList.Find(target.ConvertToUint64());
        const bool found = m_mapTargetToSpectateList.IsValidIndex(index);

        if (found)
        {
            m_mapTargetToSpectateList[index]->m_vecSpectators.AddToTail(person.ConvertToUint64());
        }
        else
        {
            SpecList *list = new SpecList;
            list->m_vecSpectators.AddToTail(person.ConvertToUint64());
            m_mapTargetToSpectateList.Insert(target.ConvertToUint64(), list);
        }
    }
}
