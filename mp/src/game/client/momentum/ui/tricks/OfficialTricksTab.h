#pragma once

#include "vgui_controls/PropertyPage.h"

class OfficialTricksTab : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(OfficialTricksTab, PropertyPage);

    OfficialTricksTab(Panel *pParent);

    void OnResetData() override;

protected:
    void OnCommand(const char *command) override;

private:
    vgui::TextEntry *m_pTrickNameFilterEntry;
    vgui::Button *m_pFiltersButton;
    vgui::ListPanel *m_pOfficialTricksList;
};