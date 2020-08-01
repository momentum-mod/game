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
    bool bActive;
};

#define CREDITS_FILE "scripts/credits.txt"
#define CREDITS_SPEEDUP_FACTOR 0.27f
#define CREDITS_MUSIC_FILEPATH "*#music/clarity.mp3" // need *# to be effected by music volume

class CHudCredits : public Frame
{
    DECLARE_CLASS_SIMPLE(CHudCredits, vgui::Frame);

  public:
    CHudCredits();

    static void Init();

    void OnClose() override;
    void Activate() override;

  protected:
    void Paint() override;

  private:
    void Clear();

    void ReadNames(KeyValues *pKeyValue);
    void ReadParams(KeyValues *pKeyValue);

    void PrepareLine(HFont hFont, char const *pchLine);
    void PrepareCredits(const char *pKeyName);
    void PrepareOutroCredits(void);

    void DrawOutroCreditsName(void);

    CPanelAnimationVar(Color, m_TextColor, "TextColor", "White");

    CUtlVector<creditname_t> m_CreditsList;

    float m_flScrollTime;
    float m_flSeparation;
    float m_flFadeTime;
    bool m_bLastOneInPlace;
    int m_Alpha;

    float m_flCreditsPixelHeight;

    int m_iMusicGUID;
};

using namespace vgui;

static CHudCredits *g_pCreditsDialog = nullptr;

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

        int iStringWidth = UTIL_ComputeStringWidth(pCredit->font, unicode);

        surface()->DrawSetTextPos((GetWide() / 2) - (iStringWidth / 2), pCredit->flYPos);
        surface()->DrawUnicodeString(unicode);
    }
}

void CHudCredits::Paint()
{
    DrawOutroCreditsName();
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
