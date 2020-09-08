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
    SetShouldCenterEmptyListText(true);
    SetAllowUserModificationOfColumns(true);
    SetMultiselectEnabled(false);
    SetAutoTallHeaderToFont(true);
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
                        g_pMapSelector->OnRemoveMapFromLibrary(mapID);
                    else
                        g_pMapSelector->OnAddMapToLibrary(mapID);
                }
                else if (col == HEADER_MAP_IN_FAVORITES)
                {
                    if (pMap->GetInt(KEYNAME_MAP_IN_FAVORITES) == INDX_MAP_IN_FAVORITES)
                        g_pMapSelector->OnRemoveMapFromFavorites(mapID);
                    else
                        g_pMapSelector->OnAddMapToFavorites(mapID);
                }
            }
        }
    }
}

void CMapListPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetRowHeight(GetScaledVal(28));
}