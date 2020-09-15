#include "cbase.h"

#include "ChatLine.h"


#include "ChatHistory.h"
#include "ChatPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar hud_saytext_time("hud_saytext_time", "12", 0);

ChatLine::ChatLine(ChatPanel *parent) : RichText(parent, "ChatLine")
{
    m_pChat = parent;
    m_nLineCounter = 1;

    m_hFont = 0;
    m_flExpireTime = 0.0f;
    m_flStartTime = 0.0f;
    m_iNameLength = 0;
    m_text = NULL;

    SetPaintBackgroundEnabled(true);

    SetVerticalScrollbar(false);
}

ChatLine::~ChatLine()
{
    if (m_text)
    {
        delete[] m_text;
        m_text = NULL;
    }
}

void ChatLine::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hFont = pScheme->GetFont("Default");

    SetBgColor(Color(0, 0, 0, 100));

    m_clrText = pScheme->GetColor("FgColor", GetFgColor());
    SetFont(m_hFont);
}

void ChatLine::PerformFadeout(void)
{
    // Flash + Extra bright when new
    float curtime = gpGlobals->curtime;

    int lr = m_clrText[0];
    int lg = m_clrText[1];
    int lb = m_clrText[2];

    if (curtime >= m_flStartTime && curtime < m_flStartTime + CHATLINE_FLASH_TIME)
    {
        float frac1 = (curtime - m_flStartTime) / CHATLINE_FLASH_TIME;
        float frac = frac1;

        frac *= CHATLINE_NUM_FLASHES;
        frac *= 2 * M_PI;

        frac = cos(frac);

        frac = clamp(frac, 0.0f, 1.0f);

        frac *= (1.0f - frac1);

        int r = lr, g = lg, b = lb;

        r = r + (255 - r) * frac;
        g = g + (255 - g) * frac;
        b = b + (255 - b) * frac;

        // Draw a right facing triangle in red, faded out over time
        int alpha = 63 + 192 * (1.0f - frac1);
        alpha = clamp(alpha, 0, 255);

        wchar_t wbuf[4096];
        GetText(0, wbuf, sizeof(wbuf));

        SetText("");

        InsertColorChange(Color(r, g, b, 255));
        InsertString(wbuf);
    }
    else if (curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME)
    {
        float frac = (m_flExpireTime - curtime) / CHATLINE_FADE_TIME;

        int alpha = frac * 255;
        alpha = clamp(alpha, 0, 255);

        wchar_t wbuf[4096];
        GetText(0, wbuf, sizeof(wbuf));

        SetText("");

        InsertColorChange(Color(lr * frac, lg * frac, lb * frac, alpha));
        InsertString(wbuf);
    }
    else
    {
        wchar_t wbuf[4096];
        GetText(0, wbuf, sizeof(wbuf));

        SetText("");

        InsertColorChange(Color(lr, lg, lb, 255));
        InsertString(wbuf);
    }

    OnThink();
}

void ChatLine::SetExpireTime()
{
    m_flStartTime = gpGlobals->curtime;
    m_flExpireTime = m_flStartTime + hud_saytext_time.GetFloat();
    m_nCount = m_nLineCounter++;
}

bool ChatLine::IsReadyToExpire()
{
    // Engine disconnected, expire right away
    if (!engine->IsInGame() && !engine->IsConnected())
        return true;

    if (gpGlobals->curtime >= m_flExpireTime)
        return true;

    return false;
}

void ChatLine::Expire() { SetVisible(false); }

inline wchar_t *CloneWString(const wchar_t *str)
{
    const int nLen = V_wcslen(str) + 1;
    wchar_t *cloneStr = new wchar_t[nLen];
    const int nSize = nLen * sizeof(wchar_t);
    V_wcsncpy(cloneStr, str, nSize);
    return cloneStr;
}

void ChatLine::InsertAndColorizeText(wchar_t *buf, int clientIndex)
{
    if (m_text)
    {
        delete[] m_text;
        m_text = NULL;
    }
    m_textRanges.RemoveAll();

    m_text = CloneWString(buf);

    if (!m_pChat)
        return;

    wchar_t *txt = m_text;
    int lineLen = wcslen(m_text);
    Color colCustom;
    if (m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION || m_text[0] == COLOR_NORMAL ||
        m_text[0] == COLOR_ACHIEVEMENT || m_text[0] == COLOR_CUSTOM || m_text[0] == COLOR_HEXCODE ||
        m_text[0] == COLOR_HEXCODE_ALPHA)
    {
        while (txt && *txt)
        {
            TextRange range;
            bool bFoundColorCode = false;
            bool bDone = false;
            int nBytesIn = txt - m_text;

            switch (*txt)
            {
            case COLOR_CUSTOM:
            case COLOR_PLAYERNAME:
            case COLOR_LOCATION:
            case COLOR_ACHIEVEMENT:
            case COLOR_NORMAL:
            {
                // save this start
                range.start = nBytesIn + 1;
                range.color = m_pChat->GetTextColorForClient((TextColor)(*txt), clientIndex);
                range.end = lineLen;
                bFoundColorCode = true;
            }
            ++txt;
            break;
            case COLOR_HEXCODE:
            case COLOR_HEXCODE_ALPHA:
            {
                bool bReadAlpha = (*txt == COLOR_HEXCODE_ALPHA);
                const int nCodeBytes = (bReadAlpha ? 8 : 6);
                range.start = nBytesIn + nCodeBytes + 1;
                range.end = lineLen;
                range.preserveAlpha = bReadAlpha;
                ++txt;

                if (range.end > range.start)
                {
                    int r = V_nibble(txt[0]) << 4 | V_nibble(txt[1]);
                    int g = V_nibble(txt[2]) << 4 | V_nibble(txt[3]);
                    int b = V_nibble(txt[4]) << 4 | V_nibble(txt[5]);
                    int a = 255;

                    if (bReadAlpha)
                    {
                        a = V_nibble(txt[6]) << 4 | V_nibble(txt[7]);
                    }

                    range.color = Color(r, g, b, a);
                    bFoundColorCode = true;

                    txt += nCodeBytes;
                }
                else
                {
                    // Not enough characters remaining for a hex code. Skip the rest of the string.
                    bDone = true;
                }
            }
            break;
            default:
                ++txt;
            }

            if (bDone)
            {
                break;
            }

            if (bFoundColorCode)
            {
                int count = m_textRanges.Count();
                if (count)
                {
                    m_textRanges[count - 1].end = nBytesIn;
                }

                m_textRanges.AddToTail(range);
            }
        }
    }

    if (!m_textRanges.Count() && m_iNameLength > 0 && m_text[0] == COLOR_USEOLDCOLORS)
    {
        TextRange range;
        range.start = 0;
        range.end = m_iNameStart;
        range.color = m_pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = m_iNameStart;
        range.end = m_iNameStart + m_iNameLength;
        range.color = m_pChat->GetTextColorForClient(COLOR_PLAYERNAME, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = range.end;
        range.end = Q_wcslen(m_text);
        range.color = m_pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    if (!m_textRanges.Count())
    {
        TextRange range;
        range.start = 0;
        range.end = Q_wcslen(m_text);
        range.color = m_pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t *start = m_text + m_textRanges[i].start;
        if (*start > 0 && *start < COLOR_MAX)
        {
            Assert(*start != COLOR_HEXCODE && *start != COLOR_HEXCODE_ALPHA);
            m_textRanges[i].start += 1;
        }
    }

    Colorize();
}

//-----------------------------------------------------------------------------
// Purpose: Inserts colored text into the RichText control at the given alpha
//-----------------------------------------------------------------------------
void ChatLine::Colorize(int alpha)
{
    // clear out text
    SetText("");

    m_pChat->GetChatHistory()->InsertString("\n");

    wchar_t wText[4096];

    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t *start = m_text + m_textRanges[i].start;
        int len = m_textRanges[i].end - m_textRanges[i].start + 1;
        if (len > 1 && len <= ARRAYSIZE(wText))
        {
            wcsncpy(wText, start, len);
            wText[len - 1] = 0;
            Color color = m_textRanges[i].color;
            if (!m_textRanges[i].preserveAlpha)
            {
                color[3] = alpha;
            }

            InsertColorChange(color);
            InsertString(wText);

            m_pChat->GetChatHistory()->InsertColorChange(color);
            m_pChat->GetChatHistory()->InsertString(wText);
            m_pChat->GetChatHistory()->InsertFade(hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME);

            if (i == m_textRanges.Count() - 1)
            {
                m_pChat->GetChatHistory()->InsertFade(-1, -1);
            }
        }
    }

    InvalidateLayout(true);
}