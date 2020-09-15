#pragma once

#include "vgui_controls/PropertyPage.h"

class ProfileStatsPage : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(ProfileStatsPage, vgui::PropertyPage);

    ProfileStatsPage(Panel *pParent);
    ~ProfileStatsPage();

protected:
    MESSAGE_FUNC(OnUserDataUpdate, "UserDataUpdate")
    {
        UpdateDialogVariables();
    }

    void OnResetData() override;

private:
    void UpdateDialogVariables();
};