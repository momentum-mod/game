#include "cbase.h"

#include "DoubleLabel.h"
#include "util/mom_util.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

DoubleLabel::DoubleLabel(Panel *parent, const char *panelName) : BaseClass(parent, panelName, "")
{
    SetupSecondaryLabel(parent, CFmtStr("%sSecondary", panelName).Get(), "");
}

DoubleLabel::DoubleLabel(Panel *parent, const char *panelName, const char *secondPanelName) : BaseClass(parent, panelName, "")
{
    SetupSecondaryLabel(parent, secondPanelName, "");
}

DoubleLabel::DoubleLabel(Panel *parent, const char *panelName, const char *text1, const char *text2) : BaseClass(parent, panelName, text1)
{
    SetupSecondaryLabel(parent, CFmtStr("%sSecondary", panelName).Get(), text2);
}

DoubleLabel::DoubleLabel(Panel *parent, const char *panelName, const char *secondPanelName,
                               const char *text1, const char *text2) : BaseClass(parent, panelName, text1)
{
    SetupSecondaryLabel(parent, secondPanelName, text2);
}

void DoubleLabel::SetupSecondaryLabel(Panel *parent, const char *panelName, const char *text)
{
    m_pSecondaryLabel = new Label(parent, panelName, text);
    m_pSecondaryLabel->SetAutoTall(true);
}

void DoubleLabel::PerformLayout() 
{
    BaseClass::PerformLayout();

    char szMain[256], szSecondary[256];
    GetText(szMain, sizeof(szMain), szSecondary, sizeof(szSecondary));

    HFont labelFont = GetFont();
    int iPrimaryTextLength = UTIL_ComputeStringWidth(labelFont, szMain);
    int iSecondaryTextLength = UTIL_ComputeStringWidth(labelFont, szSecondary);
    int iCombinedTextLength = iPrimaryTextLength + iSecondaryTextLength;

    // both labels have west content alignment by default
    // this alignment is for this class specifically
    switch (m_TextAlignment)
    {
    case a_northwest:
    case a_west:
    case a_southwest:
        SetPrimaryTextInset(0, 0);
        SetSecondaryTextInset(iPrimaryTextLength, 0);
        break;
    case a_center:
        SetPrimaryTextInset((GetWide() - iCombinedTextLength) / 2, 0);  
        SetSecondaryTextInset((GetWide() + iPrimaryTextLength - iSecondaryTextLength) / 2, 0);
        break;
    case a_northeast:
    case a_east:
    case a_southeast:
        SetPrimaryTextInset(GetWide() - iCombinedTextLength, 0);
        SetSecondaryTextInset(GetWide() - iSecondaryTextLength, 0);
        break;
    default:
        break;
    }
}

void vgui::DoubleLabel::ApplySchemeSettings(vgui::IScheme *pScheme) 
{
    BaseClass::ApplySchemeSettings(pScheme);
    m_pSecondaryLabel->SetPos(GetXPos(), GetYPos());
    m_pSecondaryLabel->SetWide(GetWide());
}

void vgui::DoubleLabel::SetYPos(int y) 
{
    SetPos(GetXPos(), y);
    m_pSecondaryLabel->SetPos(GetXPos(), y);
}

void vgui::DoubleLabel::SetAlpha(int alpha)
{
    BaseClass::SetAlpha(alpha);
    m_pSecondaryLabel->SetAlpha(alpha);
}

void vgui::DoubleLabel::SetWide(int wide) 
{
    BaseClass::SetWide(wide);
    m_pSecondaryLabel->SetWide(wide);
}

void DoubleLabel::SetFont(HFont font)
{
    BaseClass::SetFont(font);
    m_pSecondaryLabel->SetFont(font);
}

void DoubleLabel::SetText(const char *textPrimary, const char *textSecondary) 
{
    BaseClass::SetText(textPrimary);
    m_pSecondaryLabel->SetText(textSecondary);
}

void DoubleLabel::SetText(const wchar_t *unicodeStringPrimary, const wchar_t *unicodeStringSecondary,
                                bool bClearUnlocalizedSymbolPrimary, bool bClearUnlocalizedSymbolSecondary)
{
    BaseClass::SetText(unicodeStringPrimary, bClearUnlocalizedSymbolPrimary);
    m_pSecondaryLabel->SetText(unicodeStringSecondary, bClearUnlocalizedSymbolSecondary);
}

void DoubleLabel::GetText(OUT_Z_BYTECAP(bufferLenPrimary) char *textOutPrimary, int bufferLenPrimary,
                          OUT_Z_BYTECAP(bufferLenSecondary) char *textOutSecondary, int bufferLenSecondary)
{
    BaseClass::GetText(textOutPrimary, bufferLenPrimary);
    m_pSecondaryLabel->GetText(textOutSecondary, bufferLenSecondary);
}

void DoubleLabel::SetFgColor(Color colorPrimary, Color colorSecondary) 
{
    BaseClass::SetFgColor(colorPrimary);
    m_pSecondaryLabel->SetFgColor(colorSecondary);
}

void DoubleLabel::GetFgColor(Color &colorPrimary, Color &colorSecondary) 
{
    colorPrimary = BaseClass::GetFgColor();
    colorSecondary = m_pSecondaryLabel->GetFgColor();
}

void DoubleLabel::SetTextInset(int xInsetPrimary, int yInsetPrimary, int xInsetSecondary, int yInsetSecondary) 
{
    BaseClass::SetTextInset(xInsetPrimary, yInsetPrimary);
    m_pSecondaryLabel->SetTextInset(xInsetSecondary, yInsetSecondary);
}

void vgui::DoubleLabel::SetVisible(bool statePrimary, bool stateSecondary) 
{
    BaseClass::SetVisible(statePrimary);
    m_pSecondaryLabel->SetVisible(stateSecondary);
}

