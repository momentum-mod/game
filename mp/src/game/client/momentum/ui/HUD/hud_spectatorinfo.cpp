#include "cbase.h"
#include "hud_spectatorinfo.h"
#include "clientmode.h"
#include "mom_shareddefs.h"
#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudSpectatorInfo);

static MAKE_CONVAR(mom_hud_spectator_info_show, "1", FLAG_HUD_CVAR, "Toggles showing the spectator panel. 0 = OFF, 1 = ON (when there are spectators), 2 = ALWAYS ON\n", 0, 2);
static MAKE_TOGGLE_CONVAR(mom_hud_spectator_info_show_names, "1", FLAG_HUD_CVAR, "Toggles showing the names of who is spectating you.\n");
static MAKE_CONVAR(mom_hud_spectator_info_name_count, "5", FLAG_HUD_CVAR, "Controls the max number of names to print of who is spectating you."
    "\n0 = unlimited (as many as the panel can handle)\n", 0, 100);

CHudSpectatorInfo::CHudSpectatorInfo(const char *pName) : CHudElement(pName), BaseClass(g_pClientMode->GetViewport(), pName),
m_pLeaderboards(nullptr)
{
    SetProportional(true);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetDefLessFunc(m_mapNameMap);

    m_idLocal = steamapicontext->SteamUser()->GetSteamID();
}

CHudSpectatorInfo::~CHudSpectatorInfo()
{
    m_pLeaderboards = nullptr;
}

bool CHudSpectatorInfo::ShouldDraw()
{
    if (!m_pLeaderboards)
        m_pLeaderboards = gViewPortInterface->FindPanelByName(PANEL_TIMES);

    m_iSpecCount = m_mapNameMap.Count();
    int showVal = mom_hud_spectator_info_show.GetInt();
    bool showFromCount = showVal == 2 || (showVal == 1 && m_iSpecCount > 0);

    return showFromCount && CHudElement::ShouldDraw() && !m_pLeaderboards->IsVisible();
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
        unsigned short index = m_mapNameMap.FirstInorder();
        int desiredCount = mom_hud_spectator_info_name_count.GetInt() == 0 ? INT_MAX : mom_hud_spectator_info_name_count.GetInt();
        int loopCount = 0;
        while (index != m_mapNameMap.InvalidIndex() && loopCount <= desiredCount)
        {
            yPos += fontTall + 2;
            //MOM_TODO: Allow customizing this text position
            surface()->DrawSetTextPos(2, yPos);
            wchar_t *pName = m_mapNameMap.Element(index);
            surface()->DrawPrintText(pName, Q_wcslen(pName));

            index = m_mapNameMap.NextInorder(index);
            loopCount++;
        }
    }
}

void CHudSpectatorInfo::LevelShutdown()
{
    m_mapNameMap.PurgeAndDeleteElements(true);
}

void CHudSpectatorInfo::SpectatorUpdate(const CSteamID& person, const CSteamID& target)
{
    unsigned short indx = m_mapNameMap.Find(person.ConvertToUint64());
    bool found = indx != m_mapNameMap.InvalidIndex();

    if (target == m_idLocal && !found)
    {
        const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(person);
        wchar_t pNameUnicode[MAX_PLAYER_NAME_LENGTH];
        wchar_t *pNameCopy = new wchar_t[MAX_PLAYER_NAME_LENGTH];
        ANSI_TO_UNICODE(pName, pNameUnicode);
        Q_wcsncpy(pNameCopy, pNameUnicode, MAX_PLAYER_NAME_LENGTH * sizeof(wchar_t));
        m_mapNameMap.Insert(person.ConvertToUint64(), pNameCopy);
    }
    else if (found)
    {
        wchar_t *pName = m_mapNameMap.Element(indx);
        delete[] pName; // clear the memory we allocated with new[]
        m_mapNameMap.RemoveAt(indx);
    }
}