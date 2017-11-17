#pragma once

#include "cbase.h"

#include "SettingsPage.h"

using namespace vgui;

class CRenderPanel;

class ReplaysSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ReplaysSettingsPage, SettingsPage);

    ReplaysSettingsPage(Panel *pParent);
    ~ReplaysSettingsPage();

    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;
    void OnPageHide() OVERRIDE;
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;
    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

private:
    void UpdateModelSettings();

    Frame *m_pModelPreviewFrame;
    CRenderPanel *m_pModelPreview;

    ConVarRef ghost_color, ghost_bodygroup; // MOM_TODO add the rest of visible things here
};