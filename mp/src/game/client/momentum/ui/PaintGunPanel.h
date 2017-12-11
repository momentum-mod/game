#pragma once

#include "cbase.h"

#include "CVarSlider.h"
#include "ColorPicker.h"
#include "SettingsPage.h"
#include "game/client/iviewport.h"
#include "mom_shareddefs.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class PaintGunPanel : public Frame, public IViewPortPanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(PaintGunPanel, Frame);

  public:
    PaintGunPanel(IViewPort *pViewport);

    ~PaintGunPanel() {}

    virtual void Activate() OVERRIDE;
    virtual void OnThink() OVERRIDE;

    virtual const char *GetName(void) OVERRIDE { return PANEL_PAINTGUN; }
    virtual void SetData(KeyValues *data) OVERRIDE {}
    virtual void Reset(void) OVERRIDE{}; // clears internal state, deactivates it
    virtual void Update(void) OVERRIDE{};
    virtual bool NeedsUpdate(void) OVERRIDE { return false; }
    virtual bool HasInputElements(void) OVERRIDE { return true; }
    // VGUI functions:
    virtual vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }
    virtual bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }; // true if panel is visible
    virtual void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); };

    virtual void ShowPanel(bool state) OVERRIDE; // activate VGUI Frame

    virtual void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    virtual void OnCommand(const char *pCommand) OVERRIDE;

    void SetLabelText() const;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);

    // When the slider changes, we want to update the text panel
    MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);

    // When the text entry updates, we want to update the slider
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

    ColorPicker *m_pColorPicker;
    CCvarSlider *m_pSliderScale;
    vgui::TextEntry *m_pTextSliderScale;
    IViewPort *m_pViewport;
    Label *m_pLabelSliderScale;
    Label *m_pLabelColorButton;
    Label *m_pLabelIgnoreZ;
    ToggleButton *m_pToggleIgnoreZ;
    Button *m_pPickColorButton;
    ConVarRef *m_pCvarIgnoreZ;
};

extern PaintGunPanel *PaintGunUI;