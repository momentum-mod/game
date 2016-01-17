#ifndef MAPSELECTOR_H
#define MAPSELECTOR_H

#ifdef _WIN32
#pragma once
#endif

class CMapSelectorDialog;

class CMapSelector : public IMapSelector
{
public:
    CMapSelector();
    ~CMapSelector();

    void Create(vgui::VPANEL parent);
    void Destroy();
    void Activate();

    void Open();

    void CloseAllMapInfoDialogs();

private:
    vgui::DHANDLE<CMapSelectorDialog> m_hMapsDlg;
    bool m_bfirstTimeOpening;
};

extern IMapSelector* mapselector;

#endif