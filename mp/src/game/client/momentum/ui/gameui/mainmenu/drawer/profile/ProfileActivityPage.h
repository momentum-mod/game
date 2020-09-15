#pragma once

#include "vgui_controls/PropertyPage.h"

struct UserActivity;

class ProfileActivityPage : public vgui::PropertyPage
{
public:
    DECLARE_CLASS_SIMPLE(ProfileActivityPage, vgui::PropertyPage);

    ProfileActivityPage(Panel *pParent);
    ~ProfileActivityPage();

protected:
    void PerformLayout() override;
    void OnResetData() override;

    MESSAGE_FUNC(OnUserDataUpdate, "UserDataUpdate");

    void OnUserActivityReceived(KeyValues *pKv);

private:
    void FetchRecentActivity();
    void UpdateRecentActivity(const CUtlVector<UserActivity> &vecActivity);

    vgui::ListPanel *m_pActivityPanel;
};