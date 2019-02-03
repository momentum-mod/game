#include "cbase.h"

#include "PaintGunPanel.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CvarSlider.h>
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
    g_pMomentumUtil->UpdatePaintDecalScale(mom_paintgun_scale.GetFloat());
}

PaintGunPanel::PaintGunPanel() : BaseClass(g_pClientMode->GetViewport(), "PaintGunPanel")
{
    SetSize(2, 2);
    SetProportional(false);
    SetScheme("ClientScheme");
    SetMouseInputEnabled(true);

    surface()->CreatePopup(GetVPanel(), false, false, false, true, false);

    m_pToggleViewmodel = new CvarToggleCheckButton(this, "ToggleViewmodel", "#MOM_PaintGunPanel_Viewmodel", "mom_paintgun_drawmodel");
    m_pToggleViewmodel->AddActionSignalTarget(this);

    m_pToggleSound = new CvarToggleCheckButton(this, "ToggleSound", "#MOM_PaintGunPanel_Sound", "mom_paintgun_shoot_sound");
    m_pToggleSound->AddActionSignalTarget(this);

    m_pPickColorButton = new Button(this, "PickColorButton", "", this, "picker");
    m_pSliderScale = new CvarSlider(this, "SliderScale");

    m_pTextSliderScale = new TextEntry(this, "TextSliderScale");
    m_pTextSliderScale->AddActionSignalTarget(this);
    m_pTextSliderScale->SetAllowNumericInputOnly(true);

    m_pLabelSliderScale = new Label(this, "LabelSliderScale", "#MOM_PaintGunPanel_SliderText");
    m_pLabelColorButton = new Label(this, "LabelColorButton", "#MOM_PaintGunPanel_Color");

    LoadControlSettings("resource/ui/PaintGunPanel.res");

    Color TextureColor;
    if (g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
    {
        m_pPickColorButton->SetDefaultColor(TextureColor, TextureColor);
        m_pPickColorButton->SetArmedColor(TextureColor, TextureColor);
        m_pPickColorButton->SetSelectedColor(TextureColor, TextureColor);
    }

    SetLabelText();
    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);

    SetVisible(false);
    ListenForGameEvent("paintgun_panel");
}

PaintGunPanel::~PaintGunPanel() {}

void PaintGunPanel::SetLabelText() const
{
    if (m_pSliderScale && m_pTextSliderScale)
    {
        mom_paintgun_scale.SetValue(m_pSliderScale->GetSliderValue());

        char buf[64];
        Q_snprintf(buf, sizeof(buf), "%.2f", m_pSliderScale->GetSliderValue());
        m_pTextSliderScale->SetText(buf);

        m_pSliderScale->ApplyChanges();
    }
}

void PaintGunPanel::OnControlModified(Panel *p)
{
    if (p == m_pSliderScale && m_pSliderScale->HasBeenModified())
    {
        SetLabelText();
    }
    else if (p == m_pToggleViewmodel || p == m_pToggleSound)
    {
        m_pToggleViewmodel->ApplyChanges();
        m_pToggleSound->ApplyChanges();
    }
}

void PaintGunPanel::OnTextChanged(Panel *p)
{
    if (p == m_pTextSliderScale)
    {
        char buf[64];
        m_pTextSliderScale->GetText(buf, 64);

        float fValue = float(atof(buf));
        float fMin, fMax;
        if (mom_paintgun_scale.GetMin(fMin) && fValue >= fMin && mom_paintgun_scale.GetMax(fMax) && fValue <= fMax)
        {
            m_pSliderScale->SetSliderValue(fValue);
            m_pSliderScale->ApplyChanges();
        }
    }
}

void PaintGunPanel::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");
    m_pColorPicker->SetPickerColor(selected);

    char buf[64];
    g_pMomentumUtil->GetHexStringFromColor(selected, buf, sizeof(buf));
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
    if (g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
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
        if (g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
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
