#include "cbase.h"

#include "CMapListPanel.h"
#include "BaseMapsPage.h"
#include "MapDownloadProgress.h"
#include "vgui/IInput.h"
#include "mom_map_cache.h"
#include "fmtstr.h"

#include "vgui_controls/ProgressBar.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapListPanel::CMapListPanel(CBaseMapsPage *pOuter, const char *pName) : BaseClass(pOuter, pName)
{
    m_pOuter = pOuter;
    SetRowHeight(80);
    SetDefLessFunc(m_mapDownloads);
}

//-----------------------------------------------------------------------------
// Purpose: Forward KEY_ENTER to the CBaseMapsPage.
//-----------------------------------------------------------------------------
void CMapListPanel::OnKeyCodeTyped(KeyCode code)
{
    // Let the outer class handle it.
    if (code == KEY_ENTER && m_pOuter->OnGameListEnterPressed())
        return;

    BaseClass::OnKeyCodeTyped(code);
}

void CMapListPanel::OnMouseReleased(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        int x, y;
        input()->GetCursorPosition(x, y);
        int row, col;
        if (GetCellAtPos(x, y, row, col))
        {
            int itemID = GetItemIDFromRow(row);
            KeyValues *pMap = GetItem(itemID);
            if (pMap)
            {
                uint32 mapID = pMap->GetInt(KEYNAME_MAP_ID);
                if (col == HEADER_MAP_IN_LIBRARY)
                {
                    if (pMap->GetInt(KEYNAME_MAP_IN_LIBRARY) == INDX_MAP_IN_LIBRARY)
                        m_pOuter->OnRemoveMapFromLibrary(mapID);
                    else
                        m_pOuter->OnAddMapToLibrary(mapID);
                }
                else if (col == HEADER_MAP_IN_FAVORITES)
                {
                    if (pMap->GetInt(KEYNAME_MAP_IN_FAVORITES) == INDX_MAP_IN_FAVORITES)
                        m_pOuter->OnRemoveMapFromFavorites(mapID);
                    else
                        m_pOuter->OnAddMapToFavorites(mapID);
                }
            }
        }
    }
}

Panel* CMapListPanel::GetCellRenderer(int itemID, int column)
{
    // Find the itemID
    auto indx = m_mapDownloads.Find(itemID);
    if (m_mapDownloads.IsValidIndex(indx))
    {
        char name[16];
        if (GetColumnHeaderName(column, name, 16) && FStrEq(name, KEYNAME_MAP_NAME))
        {
            MapDownloadProgress *pPanel = m_mapDownloads[indx].m_pOverridePanel;
            if (IsItemSelected(itemID))
            {
                pPanel->SetPaintBackgroundEnabled(true);
                VPANEL focus = input()->GetFocus();
                // if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
                if (HasFocus() || (focus && ipanel()->HasParent(focus, GetVParent())))
                {
                    pPanel->SetBgColor(m_SelectionBgColor);
                }
                else
                {
                    pPanel->SetBgColor(m_SelectionOutOfFocusBgColor);
                }
            }
            else
            {
                pPanel->SetPaintBackgroundEnabled(false);
            }
            return pPanel;
        }
    }

    return BaseClass::GetCellRenderer(itemID, column);
}

void CMapListPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetRowHeight(80);
}

void CMapListPanel::SetFont(vgui::HFont font)
{
    int oldHeight = GetRowHeight();
    BaseClass::SetFont(font);
    SetRowHeight(oldHeight);
}

void CMapListPanel::OnMapDownloadStart(KeyValues* pKv, MapDisplay_t* pDisplay)
{
    int itemID = pDisplay->m_iListID;

    // First check if we're downloading already
    auto indx = m_mapDownloads.Find(itemID);
    if (m_mapDownloads.IsValidIndex(indx))
    {
        DevLog("Already downloading!\n");
        return;
    }

    int dummy, wide, tall;
    GetCellBounds(itemID, HEADER_MAP_NAME, dummy, dummy, wide, tall);

    uint64 size = pKv->GetUint64("size");
    MapDownloadComponent comp;
    comp.pMap = pDisplay;
    comp.m_pOverridePanel = new MapDownloadProgress(pDisplay->m_pMap->m_szMapName);
    comp.m_ulDownloadSize = size;

    m_mapDownloads.Insert(itemID, comp);
}

void CMapListPanel::OnMapDownloadProgress(KeyValues* pKv, MapDisplay_t* pDisplay)
{
    auto indx = m_mapDownloads.Find(pDisplay->m_iListID);
    if (m_mapDownloads.IsValidIndex(indx))
    {
        MapDownloadComponent *comp = &m_mapDownloads[indx];

        uint32 offset = pKv->GetInt("offset");
        uint32 chunkSize = pKv->GetInt("size");
        offset += chunkSize;

        comp->m_pOverridePanel->SetDownloadProgress(float(double(offset) / double(comp->m_ulDownloadSize)));
    }
    else
    {
        Warning("Map download end with invalid index!\n");
    }
}

void CMapListPanel::OnMapDownloadEnd(KeyValues* pKv, MapDisplay_t* pDisplay)
{
    auto indx = m_mapDownloads.Find(pDisplay->m_iListID);
    if (m_mapDownloads.IsValidIndex(indx))
    {
        MapDownloadComponent *comp = &m_mapDownloads[indx];
        comp->m_pOverridePanel->DeletePanel();

        KeyValues *pKvInto = GetItem(pDisplay->m_iListID);
        pKvInto->SetColor("cellcolor", pKv->GetBool("error") ? COLOR_RED : COLOR_GREEN);
        ApplyItemChanges(pDisplay->m_iListID);

        m_mapDownloads.RemoveAt(indx);
    }
    else
    {
        Warning("Map download end with invalid index!\n");
    }
}
