#pragma once

#include "vgui_controls/Frame.h"
#include "hudelement.h"
#include "c_mom_triggers.h"

class ZoneMenu : public vgui::Frame, public CGameEventListener
{
  public: // vgui::Frame
    DECLARE_CLASS_SIMPLE(ZoneMenu, vgui::Frame);

    ZoneMenu(vgui::Panel *pParentPanel);
    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;

    MESSAGE_FUNC(OnCreateNewZone, "CreateNewZone");
    MESSAGE_FUNC(OnDeleteZone, "DeleteZone");
    MESSAGE_FUNC(OnEditZone, "DeleteZone");

  public: // CGameEventListener
    void FireGameEvent(IGameEvent *event) OVERRIDE;

  public:
    bool ShouldBindKeys() const { return m_bBindKeys; }
    int HandleKeyInput(int down, ButtonCode_t keynum);

  private:
    vgui::Label *m_pEditorTitleLabel;
    vgui::Label *m_pZoneInfoLabel;

    vgui::Button *m_pCreateNewZoneButton;
    vgui::Button *m_pDeleteZoneButton;
    vgui::Button *m_pEditZoneButton;

    // Whether or not menu should bind mouse/keyboard input to zoning commands
    bool m_bBindKeys;
    // Ent index of zone that player is currently standing in (or -1 if not in zone)
    int m_iCurrentZone;
};

extern ZoneMenu *g_pZoneMenu;