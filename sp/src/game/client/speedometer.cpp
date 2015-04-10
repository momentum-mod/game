#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "vphysics_interface.h"
#include "c_prop_vehicle.h"

using namespace vgui;

static ConVar gh_speedmeter_hvel("gh_speedmeter_hvel", "0", (FCVAR_CLIENTDLL | FCVAR_ARCHIVE), "If set to 1, doesn't take the vertical velocity component into account.");

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
		SetLabelText(L"UPS");
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

		SetDisplayValue((int)velocity.Length());
	}
}