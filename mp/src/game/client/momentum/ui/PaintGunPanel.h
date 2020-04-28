#pragma once

#include <vgui_controls/EditablePanel.h>

namespace vgui
{

class Label;
class CvarSlider;
class ColorPicker;
class CvarToggleCheckButton;

class PaintGunPanel : public EditablePanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(PaintGunPanel, EditablePanel);

  public:
    PaintGunPanel();
    ~PaintGunPanel();

    void OnThink() OVERRIDE;
    void OnCommand(const char *pCommand) OVERRIDE;

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);

    ColorPicker *m_pColorPicker;
    CvarSlider *m_pSliderScale;
    CvarTextEntry *m_pTextSliderScale;
    Label *m_pLabelSliderScale;
    Label *m_pLabelColorButton;
    CvarToggleCheckButton *m_pToggleViewmodel, *m_pToggleSound;
    Button *m_pPickColorButton;
    C_BaseEntity *m_pVguiScreenEntity;
};
}