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

    void Create(VPANEL parent) override;
    void Destroy() override;
    void Activate() override;
    void Deactivate() override;

    void Open();
    void CloseAllMapInfoDialogs();

private:
    DHANDLE<CMapSelectorDialog> m_hMapsDlg;
};

extern IMapSelector* mapselector;

#endif