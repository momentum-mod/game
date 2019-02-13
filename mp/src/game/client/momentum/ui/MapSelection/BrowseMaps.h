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

    // property page handlers
    virtual void OnPageShow();

    MapListType_e GetMapListType() OVERRIDE { return MAP_LIST_BROWSE; }

    // gets a new server list
    MESSAGE_FUNC(GetNewMapList, "GetNewMapList");

    enum EMapQueryOutputs
    {
        eNoMapsReturned,
        eNoServerReturn,
        eServerBadReturn,
        eSuccess
    };

    void RefreshComplete(EMapQueryOutputs response);

    void MapsQueryCallback(KeyValues* pKvResponse);

    void FillMapList();

    void GetSearchFilters(KeyValues *pInto);
    // Searches maps using the current filters
    void SearchMaps();
    void ApplyFilters(MapFilters_t filters) OVERRIDE;

protected:
    // vgui overrides
    virtual void PerformLayout();
private:
    MapFilters_t m_PreviousFilters;
    float m_fPrevSearchTime;
    bool m_bRequireUpdate;	// checks whether we need an update upon opening
    bool m_bOfflineMode;
    int m_iCurrentPage;
};