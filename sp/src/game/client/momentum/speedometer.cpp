#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "vphysics_interface.h"

using namespace vgui;

static ConVar gh_speedmeter_hvel("gh_speedmeter_hvel", "0", (FCVAR_CLIENTDLL | FCVAR_ARCHIVE), "If set to 1, doesn't take the vertical velocity component into account.");

//Would be better if a callback function was added (To change the Label text if it's changed in-game), maybe reusing Reset()?
static ConVar gh_speedmeter_units("gh_speedmeter_units", "1",(FCVAR_DONTRECORD | FCVAR_ARCHIVE | FCVAR_CLIENTDLL),"Changes the units of measure of the speedmeter. \n 1: Units per second. \n 2: Meters per second. \n 3: Inches per second.",true, 1, true, 3);

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
		switch ((int)gh_speedmeter_units.GetFloat()) 
		{
		case 1:
			SetLabelText(L"UPS");
			break;
		case 2:
			SetLabelText(L"m/s");
			break;
		case 3:
			SetLabelText(L"in/s");
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
		if (gh_speedmeter_hvel.GetBool())
		{
			velocity.z = 0;
		}

		//Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
		float vel = (float)velocity.Length();
		switch ((int)gh_speedmeter_units.GetFloat())
		{
		case 1:
			//We do nothing but break out of the switch, as default vel is already in UPS
			break;
		case 2:
			//1 unit = 19.05mm -> 0.01905m --Small enough to approximate to 0.02--
			vel = vel * 0.02;
			break;
		case 3:
			//1 unit = 0.75"
			//I'm not very used to imperial system.Maybe other scale but inches would be better?
			vel = vel * 0.75;
			break;
		default:
			//We do nothing but break out of the switch, as default vel is already in UPS
			break;
		}
		SetDisplayValue((int)vel);
	}
}

