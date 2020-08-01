//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Frame.h>
#include "filesystem.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

struct creditname_t
{
    char szCreditName[256];
    HFont font;
    float flYPos;
    float flXPos;
    bool bActive;
    float flTime;
    float flTimeAdd;
    float flTimeStart;
    int iSlot;
};

#define CREDITS_FILE "scripts/credits.txt"
#define CREDITS_SPEEDUP_FACTOR 0.27f
#define CREDITS_MUSIC_FILEPATH "*#music/clarity.mp3" // need *# to be effected by music volume

enum
{
    LOGO_FADEIN = 0,
    LOGO_FADEHOLD,
    LOGO_FADEOUT,
    LOGO_FADEOFF,
};

//-----------------------------------------------------------------------------
// Purpose: Shows the flashlight icon
//-----------------------------------------------------------------------------
class CHudCredits : public Frame
{
    DECLARE_CLASS_SIMPLE(CHudCredits, vgui::Frame);

  public:
    CHudCredits();

    static void Init();

    int GetStringPixelWidth(wchar_t *pString, HFont hFont);

    void OnClose() override;
    void Activate() override;

  protected:
    void Paint() override;
    void ApplySchemeSettings(IScheme *pScheme) override;

  private:
    void Clear();

    void ReadNames(KeyValues *pKeyValue);
    void ReadParams(KeyValues *pKeyValue);
    void PrepareCredits(const char *pKeyName);
    void DrawOutroCreditsName(void);
    void DrawIntroCreditsName(void);
    void DrawLogo(void);

    void PrepareLogo(float flTime);
    void PrepareOutroCredits(void);
    void PrepareIntroCredits(void);

    float FadeBlend(float fadein, float fadeout, float hold, float localTime);

    void PrepareLine(HFont hFont, char const *pchLine);

    CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "White");

    CUtlVector<creditname_t> m_CreditsList;

    float m_flScrollTime;
    float m_flSeparation;
    float m_flFadeTime;
    bool m_bLastOneInPlace;
    int m_Alpha;

    int m_iCreditsType;
    int m_iLogoState;

    float m_flFadeInTime;
    float m_flFadeHoldTime;
    float m_flFadeOutTime;
    float m_flNextStartTime;
    float m_flPauseBetweenWaves;

    float m_flLogoTimeMod;
    float m_flLogoTime;
    float m_flLogoDesiredLength;

    float m_flX;
    float m_flY;

    char m_szLogo[256];
    char m_szLogo2[256];

    Color m_cColor;

    float m_flCreditsPixelHeight;

    HFont m_hTFont;

    int m_iMusicGUID;
};

using namespace vgui;

static CHudCredits *g_pCreditsDialog = nullptr;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudCredits::CHudCredits() : BaseClass(nullptr, "HudCredits")
{
    g_pCreditsDialog = this;

    SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    SetProportional(true);

    // for mouse speedup
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetCursor(null);

    SetMoveable(false);
    SetSizeable(false);
    SetDeleteSelfOnClose(false);
    SetMinimizeToSysTrayButtonVisible(false);
    SetSysMenu(nullptr);

    SetTitleBarVisible(false);
    SetMenuButtonVisible(false);
    SetMenuButtonResponsive(false);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(false);
    SetPaintBorderEnabled(false);

    int iWidth, iTall;
    surface()->GetScreenSize(iWidth, iTall);
    SetBounds(0, 0, iWidth, iTall);
}

void CHudCredits::PrepareCredits(const char *pKeyName)
{
    Clear();

    KeyValues *pKV = new KeyValues("CreditsFile");
    if (!pKV->LoadFromFile(filesystem, CREDITS_FILE, "MOD"))
    {
        pKV->deleteThis();

        Assert(!"Credits couldn't be initialized!");
        CloseModal();
        return;
    }

    KeyValues *pKVSubkey;
    if (pKeyName)
    {
        pKVSubkey = pKV->FindKey(pKeyName);
        ReadNames(pKVSubkey);
    }

    pKVSubkey = pKV->FindKey("CreditsParams");
    ReadParams(pKVSubkey);

    pKV->deleteThis();
}

void CHudCredits::Init()
{
    if (g_pCreditsDialog)
        return;

    g_pCreditsDialog = new CHudCredits;
}

void CHudCredits::Clear(void)
{
    m_CreditsList.RemoveAll();
    m_bLastOneInPlace = false;
    m_Alpha = m_TextColor.a();
    m_iLogoState = LOGO_FADEOFF;
    m_flCreditsPixelHeight = 0.0f;
}

void CHudCredits::OnClose()
{
    enginesound->StopSoundByGuid(m_iMusicGUID);

    BaseClass::OnClose();
}

void CHudCredits::Activate()
{
    PrepareOutroCredits();

    enginesound->EmitAmbientSound(CREDITS_MUSIC_FILEPATH, 1.0f);
    m_iMusicGUID = enginesound->GetGuidForLastSoundEmitted();

    BaseClass::Activate();
}

void CHudCredits::ReadNames(KeyValues *pKeyValue)
{
    if (pKeyValue == nullptr)
    {
        Assert(!"CHudCredits couldn't be initialized!");
        return;
    }

    // Now try and parse out each act busy anim
    KeyValues *pKVNames = pKeyValue->GetFirstSubKey();

    while (pKVNames)
    {
        creditname_t Credits;
        V_strcpy_safe(Credits.szCreditName, pKVNames->GetName());

        const auto hScheme = scheme()->GetScheme("ClientScheme");
        const auto pScheme = scheme()->GetIScheme(hScheme);
        Credits.font = pScheme->GetFont(pKeyValue->GetString(Credits.szCreditName, "Default"), true);

        m_CreditsList.AddToTail(Credits);
        pKVNames = pKVNames->GetNextKey();
    }
}

void CHudCredits::ReadParams(KeyValues *pKeyValue)
{
    if (pKeyValue == nullptr)
    {
        Assert(!"CHudCredits couldn't be initialized!");
        return;
    }

    m_flScrollTime = pKeyValue->GetFloat("scrolltime", 57);
    m_flSeparation = pKeyValue->GetFloat("separation", 5);

    m_flFadeInTime = pKeyValue->GetFloat("fadeintime", 1);
    m_flFadeHoldTime = pKeyValue->GetFloat("fadeholdtime", 3);
    m_flFadeOutTime = pKeyValue->GetFloat("fadeouttime", 2);
    m_flNextStartTime = pKeyValue->GetFloat("nextfadetime", 2);
    m_flPauseBetweenWaves = pKeyValue->GetFloat("pausebetweenwaves", 2);

    m_flLogoTimeMod = pKeyValue->GetFloat("logotime", 2);

    m_flX = pKeyValue->GetFloat("posx", 2);
    m_flY = pKeyValue->GetFloat("posy", 2);

    m_cColor = pKeyValue->GetColor("color");

    Q_strncpy(m_szLogo, pKeyValue->GetString("logo", "HALF-LIFE'"), sizeof(m_szLogo));
    Q_strncpy(m_szLogo2, pKeyValue->GetString("logo2", ""), sizeof(m_szLogo2));
}

int CHudCredits::GetStringPixelWidth(wchar_t *pString, HFont hFont)
{
    int iLength = 0;

    for (wchar_t *wch = pString; *wch != 0; wch++)
    {
        iLength += surface()->GetCharacterWidth(hFont, *wch);
    }

    return iLength;
}

void CHudCredits::DrawOutroCreditsName(void)
{
    if (m_CreditsList.Count() == 0)
        return;

    float flDesiredScrollTime = m_flScrollTime;
    if (input()->IsMouseDown(MOUSE_RIGHT))
    {
        flDesiredScrollTime = m_flScrollTime * CREDITS_SPEEDUP_FACTOR;
    }

    FOR_EACH_VEC(m_CreditsList, i)
    {
        creditname_t *pCredit = &m_CreditsList[i];

        if (pCredit == nullptr)
            continue;

        int iFontTall = surface()->GetFontTall(pCredit->font);

        if (pCredit->flYPos < -iFontTall || pCredit->flYPos > GetTall())
        {
            pCredit->bActive = false;
        }
        else
        {
            pCredit->bActive = true;
        }

        Color cColor = m_TextColor;

        // HACKHACK
        // Last one stays on screen and fades out
        if (i == m_CreditsList.Count() - 1)
        {
            if (m_bLastOneInPlace)
            {
                if (m_flFadeTime <= gpGlobals->curtime)
                {
                    if (m_Alpha > 0)
                    {
                        m_Alpha -= gpGlobals->frametime * (flDesiredScrollTime * 2.0f);

                        if (m_Alpha <= 0)
                        {
                            pCredit->bActive = false;
                            CloseModal();
                            return;
                        }
                    }
                }

                cColor[3] = MAX(0, m_Alpha);
            }
            else
            {
                pCredit->flYPos -= gpGlobals->frametime * (m_flCreditsPixelHeight / flDesiredScrollTime);

                if (int(pCredit->flYPos) + (iFontTall / 2) <= GetTall() / 2)
                {
                    m_bLastOneInPlace = true;

                    m_flFadeTime = gpGlobals->curtime + 5.0f;
                }
            }
        }
        else
        {
            pCredit->flYPos -= gpGlobals->frametime * (m_flCreditsPixelHeight / flDesiredScrollTime);
        }

        if (pCredit->bActive == false)
            continue;

        surface()->DrawSetTextFont(pCredit->font);
        surface()->DrawSetTextColor(cColor);

        wchar_t unicode[256];

        if (pCredit->szCreditName[0] == '#')
        {
            g_pVGuiLocalize->ConstructString(unicode, sizeof(unicode), g_pVGuiLocalize->Find(pCredit->szCreditName), 0);
        }
        else
        {
            g_pVGuiLocalize->ConvertANSIToUnicode(pCredit->szCreditName, unicode, sizeof(unicode));
        }

        int iStringWidth = GetStringPixelWidth(unicode, pCredit->font);

        surface()->DrawSetTextPos((GetWide() / 2) - (iStringWidth / 2), pCredit->flYPos);
        surface()->DrawUnicodeString(unicode);
    }
}

void CHudCredits::DrawLogo(void)
{
    if (m_iLogoState == LOGO_FADEOFF)
    {
        return;
    }

    switch (m_iLogoState)
    {
    case LOGO_FADEIN:
    {
        float flDeltaTime = (m_flFadeTime - gpGlobals->curtime);

        m_Alpha = MAX(0, RemapValClamped(flDeltaTime, 5.0f, 0, -128, 255));

        if (flDeltaTime <= 0.0f)
        {
            m_iLogoState = LOGO_FADEHOLD;
            m_flFadeTime = gpGlobals->curtime + m_flLogoDesiredLength;
        }

        break;
    }

    case LOGO_FADEHOLD:
    {
        if (m_flFadeTime <= gpGlobals->curtime)
        {
            m_iLogoState = LOGO_FADEOUT;
            m_flFadeTime = gpGlobals->curtime + 2.0f;
        }
        break;
    }

    case LOGO_FADEOUT:
    {
        float flDeltaTime = (m_flFadeTime - gpGlobals->curtime);

        m_Alpha = RemapValClamped(flDeltaTime, 0.0f, 2.0f, 0, 255);

        if (flDeltaTime <= 0.0f)
        {
            m_iLogoState = LOGO_FADEOFF;
        }

        break;
    }
    }

    const auto iFontTall = surface()->GetFontTall(m_hTFont);

    Color cColor = m_TextColor;
    cColor[3] = m_Alpha;

    surface()->DrawSetTextFont(m_hTFont);
    surface()->DrawSetTextColor(cColor);

    wchar_t unicode[256];
    g_pVGuiLocalize->ConvertANSIToUnicode(m_szLogo, unicode, sizeof(unicode));

    int iStringWidth = GetStringPixelWidth(unicode, m_hTFont);

    surface()->DrawSetTextPos((GetWide() / 2) - (iStringWidth / 2), (GetTall() / 2) - (iFontTall / 2));
    surface()->DrawUnicodeString(unicode);

    if (Q_strlen(m_szLogo2) > 0)
    {
        g_pVGuiLocalize->ConvertANSIToUnicode(m_szLogo2, unicode, sizeof(unicode));

        iStringWidth = GetStringPixelWidth(unicode, m_hTFont);

        surface()->DrawSetTextPos((GetWide() / 2) - (iStringWidth / 2), (GetTall() / 2) + (iFontTall / 2));
        surface()->DrawUnicodeString(unicode);
    }
}

float CHudCredits::FadeBlend(float fadein, float fadeout, float hold, float localTime)
{
    float fadeTime = fadein + hold;
    float fadeBlend;

    if (localTime < 0)
        return 0;

    if (localTime < fadein)
    {
        fadeBlend = 1 - ((fadein - localTime) / fadein);
    }
    else if (localTime > fadeTime)
    {
        if (fadeout > 0)
            fadeBlend = 1 - ((localTime - fadeTime) / fadeout);
        else
            fadeBlend = 0;
    }
    else
        fadeBlend = 1;

    if (fadeBlend < 0)
        fadeBlend = 0;

    return fadeBlend;
}

void CHudCredits::DrawIntroCreditsName(void)
{
    if (m_CreditsList.Count() == 0)
        return;

    FOR_EACH_VEC(m_CreditsList, i)
    {
        creditname_t *pCredit = &m_CreditsList[i];

        if (pCredit == nullptr)
            continue;

        if (pCredit->bActive == false)
            continue;

        float localTime = gpGlobals->curtime - pCredit->flTimeStart;

        surface()->DrawSetTextFont(pCredit->font);
        surface()->DrawSetTextColor(m_cColor[0], m_cColor[1], m_cColor[2],
            FadeBlend(m_flFadeInTime, m_flFadeOutTime, m_flFadeHoldTime + pCredit->flTimeAdd, localTime) * m_cColor[3]);

        wchar_t unicode[256];
        g_pVGuiLocalize->ConvertANSIToUnicode(pCredit->szCreditName, unicode, sizeof(unicode));

        surface()->DrawSetTextPos(XRES(pCredit->flXPos), YRES(pCredit->flYPos));
        surface()->DrawUnicodeString(unicode);

        if (m_flLogoTime > gpGlobals->curtime)
            continue;

        if (pCredit->flTime - m_flNextStartTime <= gpGlobals->curtime)
        {
            if (m_CreditsList.IsValidIndex(i + 3))
            {
                creditname_t *pNextCredits = &m_CreditsList[i + 3];

                if (pNextCredits && pNextCredits->flTime == 0.0f)
                {
                    pNextCredits->bActive = true;

                    if (i < 3)
                    {
                        pNextCredits->flTimeAdd = (i + 1.0f);
                        pNextCredits->flTime = gpGlobals->curtime + m_flFadeInTime + m_flFadeOutTime +
                                               m_flFadeHoldTime + pNextCredits->flTimeAdd;
                    }
                    else
                    {
                        pNextCredits->flTimeAdd = m_flPauseBetweenWaves;
                        pNextCredits->flTime = gpGlobals->curtime + m_flFadeInTime + m_flFadeOutTime +
                                               m_flFadeHoldTime + pNextCredits->flTimeAdd;
                    }

                    pNextCredits->flTimeStart = gpGlobals->curtime;

                    pNextCredits->iSlot = pCredit->iSlot;
                }
            }
        }

        if (pCredit->flTime <= gpGlobals->curtime)
        {
            pCredit->bActive = false;

            if (i == m_CreditsList.Count() - 1)
            {
                Clear();
            }
        }
    }
}

void CHudCredits::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hTFont = pScheme->GetFont(hl2_episodic.GetBool() ? "ClientTitleFont" : "WeaponIcons", true);
}

void CHudCredits::Paint()
{
    DrawOutroCreditsName();
}

void CHudCredits::PrepareLogo(float flTime)
{
    // Only showing the logo. Just load the CreditsParams section.
    PrepareCredits(nullptr);

    m_Alpha = 0;
    m_flLogoDesiredLength = flTime;
    m_flFadeTime = gpGlobals->curtime + 5.0f;
    m_iLogoState = LOGO_FADEIN;
}

void CHudCredits::PrepareLine(HFont hFont, char const *pchLine)
{
    Assert(pchLine);

    wchar_t unicode[256];

    if (pchLine[0] == '#')
    {
        g_pVGuiLocalize->ConstructString(unicode, sizeof(unicode), g_pVGuiLocalize->Find(pchLine), 0);
    }
    else
    {
        g_pVGuiLocalize->ConvertANSIToUnicode(pchLine, unicode, sizeof(unicode));
    }

    surface()->PrecacheFontCharacters(hFont, unicode);
}

void CHudCredits::PrepareOutroCredits(void)
{
    PrepareCredits("OutroCreditsNames");

    if (m_CreditsList.Count() == 0)
        return;

    int iHeight = GetTall();

    FOR_EACH_VEC(m_CreditsList, i)
    {
        creditname_t *pCredit = &m_CreditsList[i];

        if (pCredit == nullptr)
            continue;

        pCredit->flYPos = iHeight;
        pCredit->bActive = false;

        iHeight += surface()->GetFontTall(pCredit->font) + m_flSeparation;

        PrepareLine(pCredit->font, pCredit->szCreditName);
    }

    m_flCreditsPixelHeight = float(iHeight);
}

CON_COMMAND_F(mom_credits_show, "Shows the credits.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN)
{
    if (!g_pCreditsDialog)
    {
        CHudCredits::Init();
    }

    g_pCreditsDialog->DoModal();
}
