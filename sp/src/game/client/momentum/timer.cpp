#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "vgui_helpers.h"

#define BUFSIZE (sizeof("00:00:00.0000")+1)

static ConVar bla_timer("mom_timer", "1",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Turn the timer display on/off");

static ConVar timer_mode("mom_timer_mode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
	"Set what type of timer you want.\n0 = Generic Timer (no splits)\n1 = Splits by Chapter\n2 = Splits by Level");

class C_Timer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(C_Timer, Panel);
public:
	C_Timer();
	C_Timer(const char *pElementName);
	virtual void Init();
	virtual void Reset();
	virtual bool ShouldDraw()
	{
		return bla_timer.GetBool() && CHudElement::ShouldDraw();
	}
	void MsgFunc_Timer_State(bf_read &msg);
	void MsgFunc_Timer_Reset(bf_read&);

	virtual void Paint();
	int GetCurrentTime();
	bool m_bIsRunning;
	int m_iStartTick;

private:
	int initialTall;
	wchar_t m_pwCurrentTime[BUFSIZE];
	char m_pszString[BUFSIZE];
	CUtlMap<const char*, float> map;
	int m_iTotalTicks;

protected:
	CPanelAnimationVar(float, m_flBlur, "Blur", "0");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");
	CPanelAnimationVar(Color, m_Ammo2Color, "Ammo2Color", "FgColor");

	CPanelAnimationVar(HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(HFont, m_hNumberGlowFont, "NumberGlowFont",
		"HudNumbersGlow");
	CPanelAnimationVar(HFont, m_hSmallNumberFont, "SmallNumberFont",
		"HudNumbersSmall");
	CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8",
		"proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20",
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit_xpos, "digit_xpos", "50",
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit_ypos, "digit_ypos", "2",
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit2_xpos, "digit2_xpos", "98",
		"proportional_float");
	CPanelAnimationVarAliasType(float, digit2_ypos, "digit2_ypos", "16",
		"proportional_float");
};

DECLARE_HUDELEMENT(C_Timer);

DECLARE_HUD_MESSAGE(C_Timer, Timer_State);//TODO add more for checkpoints and ending
DECLARE_HUD_MESSAGE(C_Timer, Timer_Reset);

C_Timer::C_Timer(const char *pElementName) :
CHudElement(pElementName), Panel(NULL, "HudTimer")
{
	SetParent(g_pClientMode->GetViewport());
}

void C_Timer::Init()
{
	HOOK_HUD_MESSAGE(C_Timer, Timer_State);
	HOOK_HUD_MESSAGE(C_Timer, Timer_Reset);
	initialTall = 48;
	m_iTotalTicks = 0;
	//Reset();
}

void C_Timer::Reset()
{
	m_bIsRunning = false;
	m_iTotalTicks = 0;
}

void C_Timer::MsgFunc_Timer_State(bf_read &msg)
{
	bool started = msg.ReadOneBit();
	m_bIsRunning = started;
	m_iStartTick = (int)msg.ReadLong();
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	DevMsg("TODO: run fancy effects for state `%s'\n",
		started ? "started" : "stopped");
	if (started)
	{
		//VGUI_ANIMATE("TimerStart");
		//pPlayer->EmitSound("blamod.StartTimer");
	}
	else // stopped
	{
		// Compare times.
		//VGUI_ANIMATE("TimerStop");
		//pPlayer->EmitSound("blamod.StopTimer");
	}
}

void C_Timer::MsgFunc_Timer_Reset(bf_read &msg) 
{
	Reset();
}

int C_Timer::GetCurrentTime() {
	m_iTotalTicks = gpGlobals->tickcount - m_iStartTick;
	return (m_bIsRunning ? m_iTotalTicks : 0);
}

void C_Timer::Paint(void)
{
	float m_flSecondsTime = ((float)GetCurrentTime()) * gpGlobals->interval_per_tick;

	int hours =        m_flSecondsTime / (60.0f * 60.0f);
	int minutes = fmod(m_flSecondsTime / 60.0f, 60.0f);
	int seconds = fmod(m_flSecondsTime, 60.0f);
	int millis =  fmod(m_flSecondsTime, 1.0f) * 1000.0f;

	Q_snprintf(m_pszString, sizeof(m_pszString), "%02d:%02d:%02d.%03d",
		hours,//hours
		minutes, //minutes
		seconds,//seconds
		millis);//millis

	// msg.ReadString(m_pszString, sizeof(m_pszString));
	g_pVGuiLocalize->ConvertANSIToUnicode(
		m_pszString, m_pwCurrentTime, sizeof(m_pwCurrentTime));

	// Draw the text label.
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	//current map can be found with:    g_pGameRules->MapName()

	//surface()->DrawPrintText(L"TIME", wcslen(L"TIME"));
	// Draw current time.
	surface()->DrawSetTextFont(surface()->GetFontTall(m_hTextFont));
	surface()->DrawSetTextPos(digit_xpos, digit_ypos);
	surface()->DrawPrintText(m_pwCurrentTime, wcslen(m_pwCurrentTime));
}