#include "cbase.h"
#include "CrosshairSettingsPreview.h"
#include "hud_crosshair.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>

#include "mom_player_shared.h"
#include "mom_shareddefs.h" //check if these are necessary
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

// DECLARE_NAMED_HUDELEMENT(C_CrosshairPreview, CHudCrosshairPreview);
static CHudElement *Create_CHudCrosshairPreview(void)
{
    auto pPanel = new C_CrosshairPreview("CHudCrosshairPreview");
    g_pMOMCrosshairPreview = pPanel;
    return pPanel;
};
static CHudElementHelper g_CHudCrosshairPreview_Helper(Create_CHudCrosshairPreview, 50);

C_CrosshairPreview *g_pMOMCrosshairPreview = nullptr;

C_CrosshairPreview::C_CrosshairPreview(const char *pElementName, Panel *pParent /* = nullptr*/)
    : CHudElement(pElementName), Panel(pParent ? pParent : g_pClientMode->GetViewport(), pElementName)
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
}

C_CrosshairPreview::~C_CrosshairPreview()
{
    //todo?
}

void C_CrosshairPreview::Paint()
{
    int x, y, wid, tal;
    this->GetBounds(x, y, wid, tal);
    CHudCrosshair *pCrosshair = GET_HUDELEMENT(CHudCrosshair);
    pCrosshair->DrawCrosshair(nullptr, true, x + wid / 2, y + tal / 2);
}