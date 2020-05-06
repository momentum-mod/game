#include "cbase.h"

#include "PaintGunPanel.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include "controls/ColorPicker.h"
#include "clientmode_shared.h"
#include "materialsystem/imaterialvar.h"
#include "util/mom_util.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

void PaintGunScaleCallback(IConVar *var, const char *pOldValue, float flOldValue);

static ConVar mom_paintgun_color("mom_paintgun_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                 "Amount of red on the painting textures");

static MAKE_CONVAR_C(mom_paintgun_scale, "1.0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                     "Scale the size of the paint decal (0.001 to 1.0)\n", 0.001f, 1.0f, PaintGunScaleCallback);

void PaintGunScaleCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    MomUtil::UpdatePaintDecalScale(mom_paintgun_scale.GetFloat());
}

PaintGunPanel::PaintGunPanel() : BaseClass(g_pClientMode->GetViewport(), "PaintGunPanel")
{
    SetSize(2, 2);
    SetProportional(true);
    SetScheme("ClientScheme");
    SetMouseInputEnabled(true);

    surface()->CreatePopup(GetVPanel(), false, false, false, true, false);

    m_pToggleViewmodel = new CvarToggleCheckButton(this, "ToggleViewmodel", "#MOM_PaintGunPanel_Viewmodel", "mom_paintgun_drawmodel");
    m_pToggleViewmodel->AddActionSignalTarget(this);

    m_pToggleSound = new CvarToggleCheckButton(this, "ToggleSound", "#MOM_PaintGunPanel_Sound", "mom_paintgun_shoot_sound");
    m_pToggleSound->AddActionSignalTarget(this);

    m_pPickColorButton = new Button(this, "PickColorButton", "", this, "picker");
    m_pSliderScale = new CvarSlider(this, "SliderScale", "mom_paintgun_scale", 2, true);

    m_pTextSliderScale = new CvarTextEntry(this, "TextSliderScale", "mom_paintgun_scale", 2);
    m_pTextSliderScale->AddActionSignalTarget(this);
    m_pTextSliderScale->SetAllowNumericInputOnly(true);

    m_pLabelSliderScale = new Label(this, "LabelSliderScale", "#MOM_PaintGunPanel_SliderText");
    m_pLabelColorButton = new Label(this, "LabelColorButton", "#MOM_PaintGunPanel_Color");

    LoadControlSettings("resource/ui/PaintGunPanel.res");

    Color TextureColor;
    if (MomUtil::GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
    {
        m_pPickColorButton->SetDefaultColor(TextureColor, TextureColor);
        m_pPickColorButton->SetArmedColor(TextureColor, TextureColor);
        m_pPickColorButton->SetSelectedColor(TextureColor, TextureColor);
    }

    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);

    SetVisible(false);
    ListenForGameEvent("paintgun_panel");
}

PaintGunPanel::~PaintGunPanel() {}

void PaintGunPanel::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");
    m_pColorPicker->SetPickerColor(selected);

    char buf[64];
    MomUtil::GetHexStringFromColor(selected, buf, sizeof(buf));
    mom_paintgun_color.SetValue(buf);
}

void PaintGunPanel::OnThink()
{
    // HACKHACK for focus, Blame Valve
    if (!m_pColorPicker->IsVisible())
    {
        int x, y;
        input()->GetCursorPosition(x, y);
        SetKeyBoardInputEnabled(m_pTextSliderScale->IsWithin(x, y));
    }
    else
    {
        SetKeyBoardInputEnabled(true);
    }

    Color TextureColor;
    if (MomUtil::GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
    {
        m_pPickColorButton->SetDefaultColor(TextureColor, TextureColor);
        m_pPickColorButton->SetArmedColor(TextureColor, TextureColor);
        m_pPickColorButton->SetSelectedColor(TextureColor, TextureColor);
    }

    BaseClass::OnThink();
}

void PaintGunPanel::OnCommand(const char *pCommand)
{
    if (FStrEq(pCommand, "picker"))
    {
        Color TextureColor;
        if (MomUtil::GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
        {
            m_pColorPicker->SetPickerColor(TextureColor);
            m_pColorPicker->Show();
        }
    }
    else if (FStrEq(pCommand, "Close"))
    {
        SetVisible(false);
    }

    BaseClass::OnCommand(pCommand);
}

void PaintGunPanel::FireGameEvent(IGameEvent *event)
{
    if (FStrEq("paintgun_panel", event->GetName()))
    {
        // Center the mouse in the panel
        int x, y, w, h;
        GetBounds(x, y, w, h);
        input()->SetCursorPos(x + (w / 2), y + (h / 2));

        SetVisible(true);
    }
}
