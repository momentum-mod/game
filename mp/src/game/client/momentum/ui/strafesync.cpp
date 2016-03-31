#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include <math.h>
#include "vphysics_interface.h"
#include "mom_player_shared.h"
#include "hud_fillablebar.h"

using namespace vgui;

static ConVar strafesync_draw("mom_showstrafesync", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Toggles displaying the speedmeter.\n", true, 0, true, 1);

//////////////////////////////////////////
//           CHudStrafeSyncDisplay        //
//////////////////////////////////////////
class CHudStrafeSyncDisplay : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncDisplay, CHudNumericDisplay);

public:
    CHudStrafeSyncDisplay(const char *pElementName);
    void OnThink();
    bool ShouldDraw()
    {
        return strafesync_draw.GetBool() && CHudElement::ShouldDraw();
    }
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        SetShouldDisplaySecondaryValue(true);
    }
    void Paint();
};

DECLARE_HUDELEMENT(CHudStrafeSyncDisplay);

CHudStrafeSyncDisplay::CHudStrafeSyncDisplay(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "CHudSyncMeter")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    SetLabelText(L"Sync");
}
void CHudStrafeSyncDisplay::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());

    //MOM_TODO: Make this value float with 2 digits precision. IDK how to do this for CHudNumericDisplay
    SetDisplayValue((pPlayer->m_flStrafeSync));
    SetDisplayValue(clampedStrafeSync);
    SetSecondaryValue((clampedStrafeSync - Floor2Int(clampedStrafeSync)) * 100);
}
void CHudStrafeSyncDisplay::Paint()
{
    if (ShouldDraw())
        BaseClass::Paint();
}
//////////////////////////////////////////
//           CHudStrafeSyncBar          //
//////////////////////////////////////////
class CHudStrafeSyncBar : public CHudFillableBar
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncBar, CHudFillableBar);
public:
    CHudStrafeSyncBar(const char *pElementName);
    void OnThink();
    void Paint();
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
    }
};

DECLARE_HUDELEMENT(CHudStrafeSyncBar);

CHudStrafeSyncBar::CHudStrafeSyncBar(const char *pElementName) : CHudFillableBar("CHudSyncBar")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudStrafeSyncBar::Paint()
{
    if (strafesync_draw.GetBool())
        BaseClass::Paint();
}
void CHudStrafeSyncBar::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    SetValue(pPlayer->m_flStrafeSync);
}
