#include "cbase.h"

#include "CMapListPanel.h"
#include "BaseMapsPage.h"
#include "MapDownloadProgress.h"
#include "MapSelectorDialog.h"
#include "fmtstr.h"

#include "vgui_controls/ProgressBar.h"
#include "vgui/IInput.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapListPanel::CMapListPanel(CBaseMapsPage *pOuter, const char *pName) : BaseClass(pOuter, pName)
{
    m_pOuter = pOuter;
    SetRowHeight(GetScaledVal(28));
    SetRowHeightOnFontChange(false);
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
            uint32 mapID = GetItemUserData(itemID);
            KeyValues *pMap = GetItem(itemID);
            if (pMap)
            {
                if (col == HEADER_MAP_IN_LIBRARY)
                {
                    if (pMap->GetInt(KEYNAME_MAP_IN_LIBRARY) == INDX_MAP_IN_LIBRARY)
                        MapSelectorDialog().OnRemoveMapFromLibrary(mapID);
                    else
                        MapSelectorDialog().OnAddMapToLibrary(mapID);
                }
                else if (col == HEADER_MAP_IN_FAVORITES)
                {
                    if (pMap->GetInt(KEYNAME_MAP_IN_FAVORITES) == INDX_MAP_IN_FAVORITES)
                        MapSelectorDialog().OnRemoveMapFromFavorites(mapID);
                    else
                        MapSelectorDialog().OnAddMapToFavorites(mapID);
                }
            }
        }
    }
}

Panel* CMapListPanel::GetCellRenderer(int itemID, int column)
{
    // Find the itemID
    uint32 mapID = GetItemUserData(itemID);

    MapDownloadProgress *pOverridePanel = MapSelectorDialog().GetDownloadProgressPanel(mapID);

    if (pOverridePanel && column == HEADER_MAP_NAME)
    {
        if (IsItemSelected(itemID))
        {
            pOverridePanel->SetPaintBackgroundEnabled(true);
            VPANEL focus = input()->GetFocus();
            // if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
            if (HasFocus() || (focus && ipanel()->HasParent(focus, GetVParent())))
            {
                pOverridePanel->SetBgColor(m_SelectionBgColor);
            }
            else
            {
                pOverridePanel->SetBgColor(m_SelectionOutOfFocusBgColor);
            }
        }
        else
        {
            pOverridePanel->SetPaintBackgroundEnabled(false);
        }
        return pOverridePanel;
    }

    return BaseClass::GetCellRenderer(itemID, column);
}

void CMapListPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetRowHeight(GetScaledVal(28));
}