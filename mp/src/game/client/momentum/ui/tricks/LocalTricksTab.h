#pragma once

#include "vgui_controls/PropertyPage.h"

class ContextMenu;

class LocalTricksTab : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(LocalTricksTab, PropertyPage);

    LocalTricksTab(Panel *pParent);

    void OnPageHide() override;

    void OnResetData() override;

    void ParseTrickData();

protected:
    MESSAGE_FUNC(OnItemSelected, "ItemSelected");
    MESSAGE_FUNC_PTR_INT(OnItemContextMenu, "OpenContextMenu", panel, itemID);
    MESSAGE_FUNC_INT(OnContextTrickTele, "ContextTrickTele", trickID);

private:
    ContextMenu *m_pContextMenu;
    vgui::ListPanel *m_pLocalTricksList;
};
