#pragma once

#include "vgui_controls/Button.h"

class Button2D : public vgui::Button
{
    DECLARE_CLASS_SIMPLE(Button2D, vgui::Button);

  public:
    Button2D(vgui::Panel *parent, const char *panelName, const char *text, vgui::Panel *pActionSignalTarget = nullptr,
             const char *pCmd = nullptr);
    Button2D(vgui::Panel *parent, const char *panelName, const wchar_t *text, vgui::Panel *pActionSignalTarget = nullptr,
             const char *pCmd = nullptr);

    virtual void Paint() OVERRIDE;
    virtual void PaintBlurMask();

    virtual bool IsBlur() { return m_bBlurEnabled; }

    virtual bool IsFullyVisible() { return vgui::ipanel()->IsFullyVisible(GetVPanel()); }

  private:
    bool m_bBlurEnabled;
};
