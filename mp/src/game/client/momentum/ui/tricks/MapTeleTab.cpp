#include "cbase.h"

#include "MapTeleTab.h"

#include "fmtstr.h"

#include "mom_system_tricks.h"
#include "vgui_controls/ListPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

MapTeleTab::MapTeleTab(Panel *pParent): BaseClass(pParent, "MapTeleTab")
{
    SetProportional(true);

    m_pMapTeleList = nullptr;
}

void MapTeleTab::OnResetData()
{
    m_pMapTeleList = new ListPanel(this, "MapTeleList");
    m_pMapTeleList->SetRowHeightOnFontChange(false);
    m_pMapTeleList->SetRowHeight(GetScaledVal(20));
    m_pMapTeleList->SetMultiselectEnabled(false);
    m_pMapTeleList->SetAutoTallHeaderToFont(true);
    m_pMapTeleList->ResetScrollBar();

    LoadControlSettings("resource/ui/tricks/MapTeleTab.res");

    m_pMapTeleList->AddColumnHeader(0, "num", "", GetScaledVal(90), GetScaledVal(30), GetScaledVal(100), 0);
    m_pMapTeleList->AddColumnHeader(1, "name", "#MOM_Name", GetScaledVal(120), 0);

    m_pMapTeleList->SetSortFunc(0, [](ListPanel *pListPanel, const ListPanelItem &item1, const ListPanelItem &item2) -> 
    int 
    {
        auto left = item1.kv->GetInt("num"), right = item2.kv->GetInt("num");
        if (left < right)
            return -1;
        if (left > right)
            return 1;
        return 0;
    });
    m_pMapTeleList->SetSortColumn(0);

    ParseTrickData();
}

void MapTeleTab::ParseTrickData()
{
    m_pMapTeleList->RemoveAll();

    const auto iMapTeleCount = g_pTrickSystem->GetMapTeleCount();
    for (int i = 0; i < iMapTeleCount; i++)
    {
        const auto pMapTele = g_pTrickSystem->GetMapTele(i);

        const auto pKv = new KeyValues("MapTele");
        pKv->SetInt("num", i + 1);
        pKv->SetString("name", pMapTele->m_szName);

        m_pMapTeleList->AddItem(pKv, 0, false, true);
    }
}

void MapTeleTab::OnItemSelected()
{
    const auto selectedItemID = m_pMapTeleList->GetSelectedItem(0);
    const auto pKv = m_pMapTeleList->GetItem(selectedItemID);

    engine->ClientCmd(CFmtStr("mom_tricks_map_tele %i\n", pKv->GetInt("num")).Get());

    m_pMapTeleList->ClearSelectedItems();
}