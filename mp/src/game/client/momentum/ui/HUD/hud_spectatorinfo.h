#pragma once

#include "vgui_controls/Panel.h"
#include "hudelement.h"

struct SpecList
{
    CUtlVector<uint64> m_vecSpectators;
};

class CHudSpectatorInfo : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudSpectatorInfo, vgui::Panel);

    CHudSpectatorInfo(const char *pName);
    ~CHudSpectatorInfo();

    bool ShouldDraw() OVERRIDE;
    void Paint() OVERRIDE;
    void LevelShutdown() override;

    void SpectatorUpdate(const CSteamID &person, const CSteamID &target);

protected:
    CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");

private:
    CUtlMap<uint64, SpecList*> m_mapTargetToSpectateList;
    CUtlMap<uint64, uint64> m_mapSpectatorToTargetMap;

    uint64 m_CurrentSpecTargetSteamID;
    int m_iSpecCount;
};