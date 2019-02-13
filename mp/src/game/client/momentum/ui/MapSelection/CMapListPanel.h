#pragma once

#include "vgui_controls/ListPanel.h"

class CBaseMapsPage;

//-----------------------------------------------------------------------------
// Purpose: Acts like a regular ListPanel but forwards enter key presses
// to its outer control.
//-----------------------------------------------------------------------------
class CMapListPanel : public vgui::ListPanel
{
public:
    DECLARE_CLASS_SIMPLE(CMapListPanel, vgui::ListPanel);

    CMapListPanel(CBaseMapsPage *pOuter, const char *pName);

    virtual void OnKeyCodeTyped(vgui::KeyCode code);
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;

    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void SetFont(vgui::HFont font)
    {
        int oldHeight = GetRowHeight();
        BaseClass::SetFont(font);
        SetRowHeight(oldHeight);
    }

protected:

    MESSAGE_FUNC(OnSliderMoved, "ScrollBarSliderMoved");

private:
    CBaseMapsPage *m_pOuter;
};