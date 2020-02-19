#pragma once

#include <vgui_controls/Panel.h>
#include <hudelement.h>

class C_CrosshairPreview : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(C_CrosshairPreview, Panel);

public:
    C_CrosshairPreview(const char *pElementName, Panel *pParent = nullptr);
    ~C_CrosshairPreview();

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void Paint() OVERRIDE;

    void SetPanelSize(int wide, int tall)
    {
        SetSize(wide, tall);
        PostActionSignal(new KeyValues("OnSizeChange", "wide", wide, "tall", tall));
    }

private:
    int m_iDefaultWidth, m_iDefaultTall, m_iDefaultXPos, m_iDefaultYPos;
};

extern C_CrosshairPreview *g_pMOMCrosshairPreview;