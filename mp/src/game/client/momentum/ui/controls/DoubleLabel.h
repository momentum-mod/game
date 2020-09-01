#pragma once

#include "vgui_controls/Label.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Class for displaying 2 labels together horizontally
//-----------------------------------------------------------------------------
class DoubleLabel : public Label
{
    DECLARE_CLASS_SIMPLE(DoubleLabel, Label);

  public:
    DoubleLabel(Panel *parent, const char *panelName);
    DoubleLabel(Panel *parent, const char *panelName, const char *secondPanelName);
    DoubleLabel(Panel *parent, const char *panelName, const char *text1, const char *text2);
    DoubleLabel(Panel *parent, const char *panelName, const char *secondPanelName, const char *text1, const char *text2);

    void PerformLayout() override;
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

    // Sets to both labels
    void SetFont(HFont font) override;
    void SetYPos(int y);
    void SetAlpha(int alpha);
    void SetWide(int wide);

    void SetText(const char *textPrimary, const char *textSecondary);
    void SetText(const wchar_t *unicodeStringPrimary, const wchar_t *unicodeStringSecondary,
                 bool bClearUnlocalizedSymbolPrimary = false, bool bClearUnlocalizedSymbolSecondary = false);
    void SetPrimaryText(const char *text) { BaseClass::SetText(text); };
    void SetSecondaryText(const char *text) { m_pSecondaryLabel->SetText(text); }
    void SetPrimaryText(const wchar_t *unicodeString, bool bClearUnlocalizedSymbol = false) { BaseClass::SetText(unicodeString, bClearUnlocalizedSymbol); }
    void SetSecondaryText(const wchar_t *unicodeString, bool bClearUnlocalizedSymbol = false) { m_pSecondaryLabel->SetText(unicodeString, bClearUnlocalizedSymbol); }
    void GetText(OUT_Z_BYTECAP(bufferLenPrimary) char *textOutPrimary, int bufferLenPrimary,
                 OUT_Z_BYTECAP(bufferLenSecondary) char *textOutSecondary, int bufferLenSecondary);
    void GetPrimaryText(OUT_Z_BYTECAP(bufferLen) char *textOut, int bufferLen) { BaseClass::GetText(textOut, bufferLen); }
    void GetSecondaryText(OUT_Z_BYTECAP(bufferLen) char *textOut, int bufferLen) { m_pSecondaryLabel->GetText(textOut, bufferLen); }

    void SetFgColor(Color colorPrimary, Color colorSecondary);
    void GetFgColor(Color &colorPrimary, Color &colorSecondary);
    void SetPrimaryFgColor(Color color) { BaseClass::SetFgColor(color); }
    Color GetPrimaryFgColor() { return BaseClass::GetFgColor(); }
    void SetSecondaryFgColor(Color color) { m_pSecondaryLabel->SetFgColor(color); }
    Color GetSecondaryFgColor() { return m_pSecondaryLabel->GetFgColor(); }

    void SetTextInset(int xInsetPrimary, int yInsetPrimary, int xInsetSecondary, int yInsetSecondary);
    void SetPrimaryTextInset(int xInset, int yInset) { BaseClass::SetTextInset(xInset, yInset); }
    void SetSecondaryTextInset(int xInset, int yInset) { m_pSecondaryLabel->SetTextInset(xInset, yInset); }
    void GetPrimaryTextInset(int *xInset, int *yInset) { BaseClass::GetTextInset(xInset, yInset); }
    void GetSecondaryTextInset(int *xInset, int *yInset) { m_pSecondaryLabel->GetTextInset(xInset, yInset); }

    void SetVisible(bool statePrimary, bool stateSecondary);
    void SetPrimaryVisible(bool state) { BaseClass::SetVisible(state); }
    void SetSecondaryVisible(bool state) { m_pSecondaryLabel->SetVisible(state); }
    bool IsPrimaryVisible(bool state) { return BaseClass::IsVisible(); }
    bool IsSecondaryVisible(bool state) { return m_pSecondaryLabel->IsVisible(); }

    // only use our alignment, not the baseclass'
    void SetContentAlignment(Alignment alignment) override { m_TextAlignment = alignment; }
    Alignment GetContentAlignment() const { return m_TextAlignment; }

    Label *GetSecondaryLabel() { return m_pSecondaryLabel; }

  private:
    void SetupSecondaryLabel(Panel *parent, const char *panelName, const char *text);

    Label *m_pSecondaryLabel;

    Alignment m_TextAlignment;
};

}
