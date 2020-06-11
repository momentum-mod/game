#pragma once

#include "vgui_controls/Frame.h"

class GroupPanel;
class SettingsPanel;

class CMomentumSettingsDialog : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CMomentumSettingsDialog, vgui::Frame);

    CMomentumSettingsDialog();
    ~CMomentumSettingsDialog();

    static void Init();

    void OnClose() override;
    void Activate() override;

protected:
    void OnThink() override;
    void OnReloadControls() override;
    void OnScreenSizeChanged(int iOldWide, int iOldTall) override;

    void OnCommand(const char *command) override;

private:
    void SetActivePanel(SettingsPanel *pPanel);

    vgui::ScrollableEditablePanel *m_pScrollableSettingsPanel;
    SettingsPanel *m_pCurrentSettingsPage;

    SettingsPanel *m_pInputPage, *m_pAudioPage, *m_pVideoPage, *m_pOnlinePage, *m_pGameplayPage, *m_pHUDPage;

    vgui::Button *m_pInputButton, *m_pAudioButton, *m_pVideoButton, *m_pOnlineButton, *m_pGameplayButton, *m_pHUDButton;

    GroupPanel *m_pButtonGroup;
};