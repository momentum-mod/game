#pragma once


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

    virtual void ApplySchemeSettings(IScheme *pScheme);
    virtual void SetFont(HFont font)
    {
        int oldHeight = GetRowHeight();
        BaseClass::SetFont(font);
        SetRowHeight(oldHeight);
    }

private:
    CBaseMapsPage *m_pOuter;
};