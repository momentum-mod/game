#pragma once

#include "IMapSelector.h"
#include "vgui_controls/PHandle.h"

class CMapSelectorDialog;

class CMapSelector : public IMapSelector
{
public:
    CMapSelector();
    ~CMapSelector();

    void Create(vgui::VPANEL parent) OVERRIDE;
    void Destroy() OVERRIDE;
    void Activate() OVERRIDE;
    void Deactivate() OVERRIDE;

    void Open();
    void CloseAllMapInfoDialogs();

private:
    vgui::DHANDLE<CMapSelectorDialog> m_hMapsDlg;
};

extern IMapSelector* mapselector;
