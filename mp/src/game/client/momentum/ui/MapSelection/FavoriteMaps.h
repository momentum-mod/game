#pragma once

#include "BaseMapsPage.h"

class CFavoriteMaps : public CBaseMapsPage
{
    DECLARE_CLASS_SIMPLE(CFavoriteMaps, CBaseMapsPage);

    CFavoriteMaps(Panel *pParent);

    MapListType_e GetMapListType() OVERRIDE { return MAP_LIST_FAVORITES; }

    void OnTabSelected() OVERRIDE;
    void OnGetNewMapList() OVERRIDE;

    void FireGameEvent(IGameEvent* event) OVERRIDE;

private:
    bool m_bLoadedMaps;
};