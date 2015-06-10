#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include <math.h>

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "vphysics_interface.h"

using namespace vgui;

static ConVar speedmeter_hvel("mom_speedmeter_hvel", "0", (FCVAR_CLIENTDLL | FCVAR_ARCHIVE), "If set to 1, doesn't take the vertical velocity component into account.");

//Would be better if a callback function was added (To change the Label text if it's changed in-game), maybe reusing Reset()?
//Until I figure out how callback functions work (or until someone adds it), I'll set the label text at each Think cycle. It's not optimal, and must be removed sometime
static ConVar speedmeter_units("mom_speedmeter_units", "1",(FCVAR_DONTRECORD | FCVAR_ARCHIVE | FCVAR_CLIENTDLL),"Changes the units of measure of the speedmeter. \n 1: Units per second. \n 2: Kilometers per hour. \n 3: Milles per hour.",true, 1, true, 3);

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

public:
	CHudSpeedMeter(const char *pElementName);
	virtual void Init()
	{
		Reset();
	}
	virtual void VidInit()
	{
		Reset();
	}

	virtual void Reset()
	{
		//We set the proper LabelText based on gh_speedmeter_units value
		switch ((int)speedmeter_units.GetFloat()) 
		{
		case 1:
			SetLabelText(L"UPS");
			break;
		case 2:
			SetLabelText(L"KM/H");
			break;
		case 3:
			SetLabelText(L"MPH");
			break;
		default:
			//If its value is not supported, USP is assumed (Even though this shouln't happen as Max and Min values are set)
			SetLabelText(L"UPS");
			break;
		}	
		SetDisplayValue(0);
	}
	virtual void OnThink();
};

DECLARE_HUDELEMENT(CHudSpeedMeter);



CHudSpeedMeter::CHudSpeedMeter(const char *pElementName) :
CHudElement(pElementName), CHudNumericDisplay(NULL, "HudSpeedMeter")
{
	SetParent(g_pClientMode->GetViewport());
}

void CHudSpeedMeter::OnThink()
{
	Vector velocity(0, 0, 0);
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player) {
		velocity = player->GetLocalVelocity();

		// Remove the vertical component if necessary
		if (speedmeter_hvel.GetBool())
		{
			velocity.z = 0;
		}

		//Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
		float vel = (float)velocity.Length();
		switch ((int)speedmeter_units.GetFloat())
		{
		case 1:
			//We do nothing but break out of the switch, as default vel is already in UPS
			SetLabelText(L"UPS");
			break;
		case 2:
			//1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
			vel = vel * 0.06858;
			SetLabelText(L"KM/H");
			break;
		case 3:
			//1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH 
			vel = vel * 0.04262;
			SetLabelText(L"MPH");
			break;
		default:
			//We do nothing but break out of the switch, as default vel is already in UPS
			SetLabelText(L"UPS");
			break;
		}
		//With this round we ensure that the speed is as precise as possible, insetad of taking the floor value of the float
		SetDisplayValue(round(vel));
	}
}

