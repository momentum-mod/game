#include "cbase.h"
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/FileOpenDialog.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/TextEntry.h>
#include <vgui/IInput.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "hud_replayui.h"
#include "../clientmode.h"
#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

bool Activated = false;
CHudReplay *HudReplay = NULL;

char *va(char *format, ...)
{
	va_list		argptr;
	static char	string[8][512];
	static int	curstring = 0;

	curstring = (curstring + 1) % 8;

	va_start(argptr, format);
	Q_vsnprintf(string[curstring], sizeof(string[curstring]), format, argptr);
	va_end(argptr);

	return string[curstring];
}

const char *COM_FormatSeconds(int seconds)
{
	static char string[64];

	int hours = 0;
	int minutes = seconds / 60;

	if (minutes > 0)
	{
		seconds -= (minutes * 60);
		hours = minutes / 60;

		if (hours > 0)
		{
			minutes -= (hours * 60);
		}
	}

	if (hours > 0)
	{
		Q_snprintf(string, sizeof(string), "%2i:%02i:%02i", hours, minutes, seconds);
	}
	else
	{
		Q_snprintf(string, sizeof(string), "%02i:%02i", minutes, seconds);
	}

	return string;
}

CHudReplay::CHudReplay(const char *pElementName) : Frame(0, pElementName)
{
	SetTitle("Reply Playback", true);

	m_pPlayPauseResume = new vgui::ToggleButton(this, "DemoPlayPauseResume", "PlayPauseResume");

	m_pGoStart = new vgui::OnClickButton(this, "DemoGoStart", "Go Start");
	m_pGoEnd = new vgui::OnClickButton(this, "DemoGoEnd", "Go End");
	m_pPrevFrame = new vgui::OnClickButton(this, "DemoPrevFrame", "Prev Frame");
	m_pNextFrame = new vgui::OnClickButton(this, "DemoNextFrame", "Next Frame");
	m_pFastForward = new vgui::OnClickButton(this, "DemoFastForward", "Fast Fwd");
	m_pFastBackward = new vgui::OnClickButton(this, "DemoFastBackward", "Fast Bwd");
	m_pGo = new vgui::OnClickButton(this, "DemoGo", "Go");

	m_pProgress = new vgui::ProgressBar(this, "DemoProgress");
	m_pProgress->SetSegmentInfo(2, 2);

	m_pProgressLabelFrame = new vgui::Label(this, "DemoProgressLabelFrame", "");
	m_pProgressLabelTime = new vgui::Label(this, "DemoProgressLabelTime", "");

	m_pSpeedScale = new vgui::Slider(this, "DemoSpeedScale");
	// 1000 == 10x %
	m_pSpeedScale->SetRange(0, 200);
	m_pSpeedScale->SetValue(TICK_INTERVAL * 1000);
	m_pSpeedScale->AddActionSignalTarget(this);

	vgui::ivgui()->AddTickSignal(BaseClass::GetVPanel(), m_pSpeedScale->GetValue());

	m_pSpeedScaleLabel = new vgui::Label(this, "SpeedScale", "");

	m_pGotoTick = new vgui::TextEntry(this, "DemoGoToTick");

	LoadControlSettings("Resource\\HudReplay.res");
	SetMoveable(true);
	SetSizeable(false);
	SetVisible(false);


	memset(m_nOldCursor, 0, sizeof(m_nOldCursor));
	m_bInputActive = false;
	BaseClass::SetSize(310, 210);
}

void CHudReplay::OnTick()
{
	BaseClass::OnTick();

	char curtime[32];
	char totaltime[32];
	float fProgress = 0.0f;

	// enable/disable all playback control buttons
	m_pPlayPauseResume->SetEnabled(true);
	m_pNextFrame->SetEnabled(true);
	m_pGoStart->SetEnabled(true);
	m_pGoEnd->SetEnabled(true);
	m_pPrevFrame->SetEnabled(true);
	m_pPlayPauseResume->SetEnabled(true);
	m_pFastBackward->SetEnabled(true);
	m_pFastForward->SetEnabled(true);
	m_pGotoTick->SetEnabled(true);
	m_pGo->SetEnabled(true);

	vgui::ivgui()->AddTickSignal(BaseClass::GetVPanel(), m_pSpeedScale->GetValue());

	// set play button text
	if (shared->m_bIsPlaying)
	{
		m_pPlayPauseResume->SetText("Playing");
	}
	else
	{
		m_pPlayPauseResume->SetText("Paused");
	}

	if (m_pFastBackward->IsSelected())
	{
		shared->m_iCurrentTick--;
		shared->m_iTotalTicksT--;
	}

	if (m_pFastForward->IsSelected())
	{
		shared->m_iCurrentTick++;
		shared->m_iTotalTicksT++;
	}

	if (shared->m_iCurrentTick < 0)
	{
		shared->m_iCurrentTick = 0;
	}

	if (shared->m_iCurrentTick > shared->m_iTotalTicks)
	{
		shared->m_iCurrentTick = shared->m_iTotalTicks;
	}

	if (shared->m_iTotalTicksT < 0)
	{
		shared->m_iTotalTicksT = 0;
	}

	fProgress = (float)shared->m_iCurrentTick / (float)shared->m_iTotalTicks;
	fProgress = clamp(fProgress, 0.0f, 1.0f);

	m_pProgress->SetProgress(fProgress);
	m_pProgressLabelFrame->SetText(va("Tick: %i / %i", shared->m_iCurrentTick, shared->m_iTotalTicks));

	Q_strncpy(curtime, COM_FormatSeconds(TICK_INTERVAL * shared->m_iCurrentTick), 32);
	Q_strncpy(totaltime, COM_FormatSeconds(TICK_INTERVAL * shared->m_iTotalTicks), 32);

	m_pProgressLabelTime->SetText(va("Time: %s / %s", curtime, totaltime));
	m_pSpeedScaleLabel->SetText(va("%.1f %%", (float)m_pSpeedScale->GetValue()));
}

// Command issued
void CHudReplay::OnCommand(const char *command)
{
	if (!Q_strcasecmp(command, "play"))
	{
		shared->m_bIsPlaying = !shared->m_bIsPlaying;
		m_pSpeedScale->SetValue(TICK_INTERVAL * 1000);
	}
	else if (!Q_strcasecmp(command, "reload"))
	{
		shared->m_iCurrentTick = 0;
		shared->m_iTotalTicksT = 0;
	}
	else if (!Q_strcasecmp(command, "gotoend"))
	{
		shared->m_iCurrentTick = shared->m_iTotalTicks;
	}
	else if (!Q_strcasecmp(command, "prevframe"))
	{
		shared->m_iTotalTicksT--;
		shared->m_iCurrentTick--;
	}
	else if (!Q_strcasecmp(command, "nextframe"))
	{
		shared->m_iTotalTicksT++;
		shared->m_iCurrentTick++;
	}
	else if (!Q_strcasecmp(command, "gototick"))
	{
		char tick[32];
		m_pGotoTick->GetText(tick, sizeof(tick));
		shared->m_iCurrentTick = atoi(tick);
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void replayui_f()
{
	if (!HudReplay)
	HudReplay = new CHudReplay("HudReplay");

	if (!HudReplay)
		return;

	if (HudReplay->IsVisible())
	{
		HudReplay->Close();
	}
	else
	{
		HudReplay->Activate();
	}
}

static ConCommand replayui("replayui", replayui_f, "Replay Ghost GUI.");
