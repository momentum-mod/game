#pragma once

#include <vgui_controls/Panel.h>
#include <hudelement.h>
#include "hud_crosshair.h"

class C_CrosshairPreview : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(C_CrosshairPreview, Panel);

public:
    C_CrosshairPreview(const char *pElementName, Panel *pParent = nullptr);
    ~C_CrosshairPreview();

    void Paint() OVERRIDE;
};

extern C_CrosshairPreview *g_pMOMCrosshairPreview;