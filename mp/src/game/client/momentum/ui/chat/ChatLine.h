#pragma once

#include "vgui_controls/RichText.h"

class ChatPanel;

#define CHATLINE_NUM_FLASHES 8.0f
#define CHATLINE_FLASH_TIME 5.0f
#define CHATLINE_FADE_TIME 1.0f

struct TextRange
{
    TextRange() { preserveAlpha = false; }
    int start;
    int end;
    Color color;
    bool preserveAlpha;
};

class ChatLine : public vgui::RichText
{
public:
    DECLARE_CLASS_SIMPLE(ChatLine, RichText);

    ChatLine(ChatPanel *parent);
    ~ChatLine();

    void SetExpireTime();
    bool IsReadyToExpire();
    void Expire();

    float GetStartTime() const { return m_flStartTime; }

    int GetCount() const { return m_nCount; }

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

    vgui::HFont GetFont() const { return m_hFont; }

    Color GetTextColor() const { return m_clrText; }
    void SetNameLength(int iLength) { m_iNameLength = iLength; }
    void SetNameColor(Color cColor) { m_clrNameColor = cColor; }

    void PerformFadeout();
    void InsertAndColorizeText(wchar_t *buf, int clientIndex);
    void Colorize(int alpha = 255); ///< Re-inserts the text in the appropriate colors at the given alpha

    void SetNameStart(int iStart) { m_iNameStart = iStart; }

protected:
    int m_iNameLength;
    vgui::HFont m_hFont;

    Color m_clrText;
    Color m_clrNameColor;

    float m_flExpireTime;

    CUtlVector<TextRange> m_textRanges;
    wchar_t *m_text;

    int m_iNameStart;

private:
    float m_flStartTime;
    int m_nCount;
    int m_nLineCounter;

    ChatPanel *m_pChat;
};