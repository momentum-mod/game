#pragma once

#include "cbase.h"

#include "ColorPicker.h"
#include "SettingsPage.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class PaintGunPanel : public Frame
{
    DECLARE_CLASS_SIMPLE( PaintGunPanel , Frame );

  public:
    PaintGunPanel();

    ~PaintGunPanel() {}

    void Activate() OVERRIDE;

    void OnThink() OVERRIDE;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);

    ColorPicker *m_pColorPicker;
};

extern PaintGunPanel *paintgunui;
