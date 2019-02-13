#include "cbase.h"

#include "CMapListPanel.h"
#include "BaseMapsPage.h"
#include "vgui/IInput.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapListPanel::CMapListPanel(CBaseMapsPage *pOuter, const char *pName) : BaseClass(pOuter, pName)
{
    m_pOuter = pOuter;
    SetRowHeight(80);
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
            uint32 mapID = pMap->GetInt(KEYNAME_MAP_ID);
            if (col == HEADER_MAP_IN_LIBRARY)
            {
                if (pMap->GetInt(KEYNAME_MAP_IN_LIBRARY))
                    m_pOuter->OnRemoveMapFromLibrary(mapID);
                else
                    m_pOuter->OnAddMapToLibrary(mapID);
            }
            else if (col == HEADER_MAP_IN_FAVORITES)
            {
                if (pMap->GetInt(KEYNAME_MAP_IN_FAVORITES))
                    m_pOuter->OnRemoveMapFromFavorites(mapID);
                else
                    m_pOuter->OnAddMapToFavorites(mapID);
            }
        }
    }
}

void CMapListPanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetRowHeight(80);
}

void CMapListPanel::OnSliderMoved()
{
    
}
