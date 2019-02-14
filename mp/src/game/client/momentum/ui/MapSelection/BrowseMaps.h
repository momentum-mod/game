#pragma once

#include "BaseMapsPage.h"

//-----------------------------------------------------------------------------
// Purpose: Internet games list
//-----------------------------------------------------------------------------
class CBrowseMaps : public CBaseMapsPage
{
    DECLARE_CLASS_SIMPLE(CBrowseMaps, CBaseMapsPage);

    CBrowseMaps(Panel *parent);
    ~CBrowseMaps();

    MapListType_e GetMapListType() OVERRIDE { return MAP_LIST_BROWSE; }

    void OnGetNewMapList() OVERRIDE;

    enum EMapQueryOutputs
    {
        eNoMapsReturned,
        eNoServerReturn,
        eServerBadReturn,
        eSuccess
    };

    void RefreshComplete(EMapQueryOutputs response);

    void MapsQueryCallback(KeyValues* pKvResponse);

    void GetSearchFilters(KeyValues *pInto);
    // Searches maps using the current filters
    void SearchMaps();
    void ApplyFilters(MapFilters_t filters) OVERRIDE;
    void OnTabSelected() OVERRIDE;

private:
    MapFilters_t m_PreviousFilters;
    float m_fPrevSearchTime;
};