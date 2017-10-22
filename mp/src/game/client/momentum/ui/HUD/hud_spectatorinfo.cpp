#include "cbase.h"
#include "hud_spectatorinfo.h"
#include "clientmode.h"
#include "mom_shareddefs.h"
#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudSpectatorInfo);

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

    return CHudElement::ShouldDraw() && !m_pLeaderboards->IsVisible();
}

void CHudSpectatorInfo::Paint()
{
    int specCount = m_mapNameMap.Count();

    char spectatorCountANSI[128];
    Q_snprintf(spectatorCountANSI, 128, "Spectators: %i", specCount);

    wchar_t spectatorCountUnicode[128];
    ANSI_TO_UNICODE(spectatorCountANSI, spectatorCountUnicode);

    int yPos = 2;
    int fontTall = surface()->GetFontTall(m_hTextFont);

    surface()->DrawSetTextColor(COLOR_WHITE);
    surface()->DrawSetTextPos(2, yPos);
    surface()->DrawPrintText(spectatorCountUnicode, Q_wcslen(spectatorCountUnicode));

    // MOM_TODO: If mom_hud_spectator_show_names
    unsigned short index = m_mapNameMap.FirstInorder();
    // MOM_TODO: Iterate over at most mom_hud_spectator_name_count names
    while (index != m_mapNameMap.InvalidIndex())
    {
        yPos += fontTall + 2;

        surface()->DrawSetTextPos(2, yPos);
        wchar_t *pName = m_mapNameMap.Element(index);
        surface()->DrawPrintText(pName, Q_wcslen(pName));

        index = m_mapNameMap.NextInorder(index);
    }
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
        Q_wcsncpy(pNameCopy, pNameUnicode, 64);
        m_mapNameMap.Insert(person.ConvertToUint64(), pNameCopy);
    }
    else if (found && indx != m_mapNameMap.InvalidIndex())
    {
        wchar_t *pName = m_mapNameMap.Element(indx);
        delete[] pName; // clear the memory we allocated with new
        m_mapNameMap.RemoveAt(indx);
    }
}