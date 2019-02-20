#ifndef ZONEMENU_H
#define ZONEMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "hudelement.h"
#include "c_mom_triggers.h"

class C_MomZoneMenu : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(C_MomZoneMenu, vgui::Frame);

  public: // vgui::Frame
    C_MomZoneMenu(vgui::Panel *pParentPanel);
    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnClose() OVERRIDE;

	MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

    MESSAGE_FUNC(OnCreateNewZone, "CreateNewZone");
    MESSAGE_FUNC(OnDeleteZone, "DeleteZone");
    MESSAGE_FUNC(OnEditZone, "EditZone");
    MESSAGE_FUNC(OnCancelZone, "CancelZone");
    MESSAGE_FUNC(OnSaveZones, "SaveZones");

  public:
    bool ShouldBindKeys() const { return m_bBindKeys; }
    int HandleKeyInput(int down, ButtonCode_t keynum);

  private:
    static void OnZoneInfoThunk(bf_read &msg);
    void OnZoneInfo(bf_read &msg);

    void CancelZoning();

  private:
    vgui::Label *m_pEditorTitleLabel;

    vgui::Button *m_pCreateNewZoneButton;
    vgui::Button *m_pDeleteZoneButton;
    vgui::Button *m_pEditZoneButton;
    vgui::Button *m_pCancelZoneButton;
    vgui::Button *m_pSaveZonesButton;

    // Zone type
    vgui::Label    *m_pZoneTypeLabel;
    vgui::ComboBox *m_pZoneTypeCombo;

    // Grid size setting
    vgui::Label         *m_pGridSizeLabel;
    vgui::CvarSlider    *m_pGridSizeSlider;
    vgui::CvarTextEntry *m_pGridSizeTextEntry;
    bool                 m_bUpdateGridSizeSlider;

    // Whether or not menu should bind mouse/keyboard input to zoning commands
    bool m_bBindKeys;
    
    enum ZoneAction
    {
        ZONEACTION_NONE =  0,
        ZONEACTION_DELETE,
        ZONEACTION_EDIT
    };
    // What action to execute when mom_zone_info returns the info we want
    ZoneAction m_eZoneAction;
};

extern C_MomZoneMenu *g_pZoneMenu;
#endif