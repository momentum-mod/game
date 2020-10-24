#pragma once

#include "vgui_controls/PropertyPage.h"

class MapTeleTab : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(MapTeleTab, PropertyPage);

    MapTeleTab(Panel *pParent);

    void OnResetData() override;

    void ParseTrickData();

protected:
    MESSAGE_FUNC(OnItemSelected, "ItemSelected");

private:
    vgui::ListPanel *m_pMapTeleList;
};