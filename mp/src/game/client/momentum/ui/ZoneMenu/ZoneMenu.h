#pragma once

#include "vgui_controls/Frame.h"

class C_MomZoneMenu : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(C_MomZoneMenu, vgui::Frame);

  public: // vgui::Frame
    C_MomZoneMenu();

    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
    MESSAGE_FUNC_PTR_INT(OnButtonChecked, "CheckButtonChecked", panel, state);

protected:
    void OnCommand(const char* command) OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    void OnClose() OVERRIDE;

  public:
    int HandleKeyInput(int down, ButtonCode_t keynum);

  private:
    static void OnZoneInfoThunk(bf_read &msg);
    void OnZoneInfo(bf_read &msg);

    vgui::CvarToggleCheckButton *m_pToggleZoneEdit, *m_pToggleUsePointMethod;

    vgui::Button *m_pCreateNewZoneButton;
    vgui::Button *m_pDeleteZoneButton;
    vgui::Button *m_pEditZoneButton;
    vgui::Button *m_pCancelZoneButton;
    vgui::Button *m_pSaveZonesButton;

    // Track number
    vgui::Label *m_pTrackNumberLabel;
    vgui::CvarTextEntry *m_pTrackNumberEntry;

    // Zone number
    vgui::Label *m_pZoneNumberLabel;
    vgui::CvarTextEntry *m_pZoneNumberEntry;

    // Zone type
    vgui::Label    *m_pZoneTypeLabel;
    vgui::ComboBox *m_pZoneTypeCombo;

    // Grid size setting
    vgui::Label         *m_pGridSizeLabel;
    vgui::CvarSlider    *m_pGridSizeSlider;
    vgui::CvarTextEntry *m_pGridSizeTextEntry;

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