#pragma once

#include "vgui_controls/Panel.h"

class CMomNUIPanel : public vgui::Panel
{
public:
    DECLARE_CLASS_SIMPLE(CMomNUIPanel, vgui::Panel);

public:
    CMomNUIPanel();
    ~CMomNUIPanel();

public:
    virtual void OnThink() OVERRIDE;
    virtual void Paint() OVERRIDE;
    virtual void OnCursorEntered() OVERRIDE;
    virtual void OnCursorExited() OVERRIDE;
    virtual void OnCursorMoved(int x, int y) OVERRIDE;
    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseDoublePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseWheeled(int delta) OVERRIDE;
    virtual void OnKeyCodePressed(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyTyped(wchar_t unichar) OVERRIDE;
    virtual void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;

private:
    int m_iTextureID;
    int m_iLastWidth;
    int m_iLastHeight;
};

extern CMomNUIPanel *g_pMomNUIPanel;