#pragma once

#include "vgui_controls/Panel.h"

class CMomNUIPanel :
    public vgui::Panel
{
public:
    DECLARE_CLASS_SIMPLE(CMomNUIPanel, vgui::Panel);

public:
    CMomNUIPanel();
    ~CMomNUIPanel();

public:
    virtual void OnThink() override;
    virtual void Paint() override;
    virtual void OnCursorEntered() override;
    virtual void OnCursorExited() override;
    virtual void OnCursorMoved(int x, int y) override;
    virtual void OnMousePressed(vgui::MouseCode code) override;
    virtual void OnMouseDoublePressed(vgui::MouseCode code) override;
    virtual void OnMouseReleased(vgui::MouseCode code) override;
    virtual void OnMouseWheeled(int delta) override;
    virtual void OnKeyCodePressed(vgui::KeyCode code) override;
    virtual void OnKeyCodeTyped(vgui::KeyCode code) override;
    virtual void OnKeyTyped(wchar_t unichar) override;
    virtual void OnKeyCodeReleased(vgui::KeyCode code) override;

private:
    int m_iTextureID;
    int m_iLastWidth;
    int m_iLastHeight;
};

extern CMomNUIPanel *g_pMomNUIPanel;