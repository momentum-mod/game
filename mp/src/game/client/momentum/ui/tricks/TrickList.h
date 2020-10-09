#pragma once

#include "mom_shareddefs.h"
#include "game/client/iviewport.h"
#include "vgui_controls/EditablePanel.h"

namespace vgui
{
    class ContinuousProgressBar;
}

class ContextMenu;
class OfficialTricksTab;
class CommunityTricksTab;
class MapTeleTab;
class LocalTricksTab;

class TrickList : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(TrickList, vgui::EditablePanel);
    TrickList(IViewPort *pViewPort);
    ~TrickList();

    const char *GetName() override { return PANEL_TRICK_LIST; }
    void SetData(KeyValues *data) override {}
    void Reset() override {}
    void Update() override {}
    bool NeedsUpdate() override { return false; }
    void ShowPanel(bool state) override;

    vgui::VPANEL GetVPanel() override { return BaseClass::GetVPanel(); }
    bool IsVisible() override { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) override { BaseClass::SetParent(parent); }
    bool HasInputElements() override { return true; }

    void FireGameEvent(IGameEvent *event) override;

private:
    void ParseTrickList();

    vgui::Label *m_pTrickProgressLabel;
    vgui::ContinuousProgressBar *m_pTrickProgressBar;

    vgui::PropertySheet *m_pTrickTabs;
    OfficialTricksTab *m_pOfficialTricksTab;
    CommunityTricksTab *m_pCommunityTricksTab;
    MapTeleTab *m_pMapTelesTab;
    LocalTricksTab *m_pLocalTricksTab;
};

extern TrickList *g_pTrickEditor;