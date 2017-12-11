#include "cbase.h"

#include "PaintGunPanel.h"
#include "clientmode_shared.h"
#include "ienginevgui.h"
#include "materialsystem/imaterialvar.h"
#include "util/mom_util.h"
#include "weapon/weapon_csbase.h"

PaintGunPanel *PaintGunUI = nullptr;

void PaintGunScaleCallback(IConVar *var, const char *pOldValue, float flOldValue);

static ConVar mom_paintgun_color("mom_paintgun_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                 "Amount of red on the painting textures");

static ConVar mom_paintgun_scale("mom_paintgun_scale", "1.0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                 "Amount of scale on the painting textures", PaintGunScaleCallback);

void PaintGunScaleCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    IMaterial *material = materials->FindMaterial("decals/paintgun", TEXTURE_GROUP_DECAL);
    if (material != nullptr)
    {
        static unsigned int nScaleCache = 0;
        IMaterialVar *VarScale = material->FindVarFast("$decalscale", &nScaleCache);
        if (VarScale != nullptr)
        {
            VarScale->SetFloatValue(mom_paintgun_scale.GetFloat());
        }
    }
}

CON_COMMAND(mom_paintgunui_show, "Shows Paint Gun UI")
{
    CBaseCombatWeapon *Weapon = GetActiveWeapon();

    if (Weapon == nullptr)
        return;

    CWeaponCSBase *WeaponBase = dynamic_cast<CWeaponCSBase *>(Weapon);

    if (WeaponBase == nullptr)
        return;

    if (WeaponBase->GetWeaponID() != WEAPON_PAINTGUN)
        return;

    if (PaintGunUI == nullptr)
        PaintGunUI = new PaintGunPanel(dynamic_cast<CBaseViewport *>(g_pClientMode->GetViewport()));

    if (PaintGunUI != nullptr)
    {
        PaintGunUI->Activate();
    }
    else
        DevMsg("Failed to init PaintGunUI\n");
};

CON_COMMAND(mom_paintgunui_close, "close Paint Gun UI")
{
    CBaseCombatWeapon *Weapon = GetActiveWeapon();

    if (Weapon == nullptr)
        return;

    CWeaponCSBase *WeaponBase = dynamic_cast<CWeaponCSBase *>(Weapon);

    if (WeaponBase == nullptr)
        return;

    if (WeaponBase->GetWeaponID() != WEAPON_PAINTGUN)
        return;

    if (PaintGunUI != nullptr)
    {
        PaintGunUI->Close();
    }
};

PaintGunPanel::PaintGunPanel(IViewPort *pViewport) : BaseClass(nullptr, PANEL_PAINTGUN, false, false)
{
    m_pViewport = pViewport;

    SetProportional(false);
    SetMoveable(true);
    SetSizeable(false);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonResponsive(false);
    SetClipToParent(true); // Needed so we won't go out of bounds

    surface()->CreatePopup(GetVPanel(), false, false, false, true, false);
    LoadControlSettings("resource/ui/PaintGunPanel.res");

    m_pPickColorButton = FindControl<Button>("PickColorButton");

    Color TextureColor;
    if (g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), TextureColor))
    {
        m_pPickColorButton->SetDefaultColor(TextureColor, TextureColor);
        m_pPickColorButton->SetArmedColor(TextureColor, TextureColor);
        m_pPickColorButton->SetSelectedColor(TextureColor, TextureColor);
    }

    m_pSliderScale = FindControl<CCvarSlider>("SliderScale");
    m_pSliderScale->SetValue(mom_paintgun_scale.GetFloat());
    m_pSliderScale->ApplyChanges();

    m_pTextSliderScale = FindControl<TextEntry>("TextSliderScale");

    m_pLabelSliderScale = FindControl<Label>("LabelSliderScale");
    m_pLabelColorButton = FindControl<Label>("LabelColorButton");
    m_pLabelIgnoreZ = FindControl<Label>("LabelIgnoreZ");

    m_pCvarIgnoreZ = new ConVarRef("mom_paintgun_ignorez");

    m_pToggleIgnoreZ = FindControl<ToggleButton>("ToggleIgnoreZ");
    m_pToggleIgnoreZ->SetSelected(m_pCvarIgnoreZ->GetBool());

    SetLabelText();

    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);

    SetScheme("ClientScheme");
}

void PaintGunPanel::SetLabelText() const
{
    if (m_pSliderScale && m_pTextSliderScale)
    {
        mom_paintgun_scale.SetValue(m_pSliderScale->GetSliderValue());

        char buf[64];
        Q_snprintf(buf, sizeof(buf), "%.1f", m_pSliderScale->GetSliderValue());
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
}

void PaintGunPanel::OnTextChanged(Panel *p)
{
    if (p == m_pTextSliderScale)
    {
        char buf[64];
        m_pTextSliderScale->GetText(buf, 64);

        float fValue = float(atof(buf));
        if (fValue >= 0.01f && fValue <= 10.0f)
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

void PaintGunPanel::Activate()
{
    BaseClass::Activate();
    ShowPanel(true);
}

void PaintGunPanel::Close()
{
    BaseClass::Close();
    ShowPanel(false);
}

void PaintGunPanel::OnThink()
{
    // HACKHACK for focus, Blame Valve
    if (!m_pColorPicker->IsVisible())
    {
        int x, y;
        input()->GetCursorPosition(x, y);
        SetKeyBoardInputEnabled(IsWithin(x, y));
    }
    else
    {
        m_pColorPicker->OnThink();
    }

    m_pCvarIgnoreZ->SetValue(m_pToggleIgnoreZ->IsSelected());

    if (m_pToggleIgnoreZ->IsSelected())
    {
        m_pToggleIgnoreZ->SetText("#MOM_PaintGunPanel_IgnoreZ");
    }
    else
    {
        m_pToggleIgnoreZ->SetText("#MOM_PaintGunPanel_NIgnoreZ");
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

void PaintGunPanel::ShowPanel(bool state)
{
    SetVisible(state);
    SetMouseInputEnabled(state);
    SetEnabled(state);
}

void PaintGunPanel::FireGameEvent(IGameEvent *pEvent)
{
    if (pEvent->GetBool("restart"))
        ShowPanel(false);
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

    BaseClass::OnCommand(pCommand);
}
