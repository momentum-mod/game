#pragma once

#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "hudelement.h"
#include "baseviewport.h"

class CHudSpectatorInfo : public CHudElement, public vgui::Panel
{
    
    DECLARE_CLASS_SIMPLE(CHudSpectatorInfo, vgui::Panel);

    CHudSpectatorInfo(const char *pName);
    ~CHudSpectatorInfo();

    bool ShouldDraw() OVERRIDE;

    void Paint() OVERRIDE;
    void LevelShutdown() OVERRIDE;

    void SpectatorUpdate(const CSteamID &person, const CSteamID &target);

protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

private:
    IViewPortPanel *m_pLeaderboards;
    CUtlMap<uint64, wchar_t *> m_mapNameMap;

    CSteamID m_idLocal;
    int m_iSpecCount;
};
