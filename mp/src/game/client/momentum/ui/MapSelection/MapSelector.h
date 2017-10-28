#ifndef MAPSELECTOR_H
#define MAPSELECTOR_H

#ifdef _WIN32
#pragma once
#endif

using namespace vgui;

class CMapSelectorDialog;

class CMapSelector : public IMapSelector
{
public:
    CMapSelector();
    ~CMapSelector();

    void Create(VPANEL parent) OVERRIDE;
    void Destroy() OVERRIDE;
    void Activate() OVERRIDE;
    void Deactivate() OVERRIDE;

    void Open();
    void CloseAllMapInfoDialogs();

private:
    DHANDLE<CMapSelectorDialog> m_hMapsDlg;
};

extern IMapSelector* mapselector;

#endif