#include "cbase.h"

#include "../../../public/ienginevgui.h"
#include "../../shared/momentum/weapon/weapon_csbase.h"
#include "PaintGunPanel.h"
#include "materialsystem/imaterialvar.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"

void PaintGunCallback(IConVar *var, const char *pOldValue, float flOldValue);

static ConVar r_decal_paintgun_color("r_decal_paintgun_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                     "Amount of red on the painting textures", PaintGunCallback);

static ConVar r_decal_paintgun_scale("r_decal_paintgun_scale", "0.1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                     "Amount of scale on the painting textures", true, 0.0f, true, 10000.0f,
                                     PaintGunCallback);

static ConVar r_decal_paintgun_ignorez("r_decal_paintgun_ignorez", "0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                       "See the painting behind walls (wall-hax)", PaintGunCallback);

void PaintGunCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    ConVarRef cVar(var);

    if (!Q_strcmp(var->GetName(), "r_decal_paintgun_scale"))
    {
        IMaterial *material = materials->FindMaterial("decals/paintgun", TEXTURE_GROUP_DECAL);
        if (material != nullptr)
        {
            static unsigned int nScaleCache = 0;
            IMaterialVar *VarScale = material->FindVarFast("$decalscale", &nScaleCache);
            if (VarScale != nullptr)
            {
                VarScale->SetFloatValue(r_decal_paintgun_scale.GetFloat());
            }
        }
    }
    else if (!Q_strcmp(var->GetName(), "r_decal_paintgun_color"))
    {
        Color MaterialColor;
        if (g_pMomentumUtil->GetColorFromHex(r_decal_paintgun_color.GetString(), MaterialColor))
        {
            IMaterial *material = materials->FindMaterial("decals/paintgun", TEXTURE_GROUP_DECAL);
            if (material != nullptr)
            {
                material->ColorModulate((float)MaterialColor.r() / 255, (float)MaterialColor.g() / 255,
                                        (float)MaterialColor.b() / 255);
                material->AlphaModulate((float)MaterialColor.a() / 255);
            }
        }
    }
    else if (!Q_strcmp(var->GetName(), "r_decal_paintgun_ignorez"))
    {
        IMaterial *material = materials->FindMaterial("decals/paintgun", TEXTURE_GROUP_DECAL);
        if (material != nullptr)
        {
            material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, r_decal_paintgun_ignorez.GetBool());
        }
    }
}

PaintGunPanel *paintgunui = nullptr;

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

    if (paintgunui == nullptr)
    {
        paintgunui = new PaintGunPanel();
    }

    if (paintgunui != nullptr)
    {
        paintgunui->Activate();
    }
    else
        DevMsg("Failed to init paintgunui\n");
};

PaintGunPanel::PaintGunPanel() : BaseClass(nullptr, PANEL_PAINTGUN, false, false)
{
    LoadControlSettings("resource/ui/PaintGunPanel.res");
    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);
}

void PaintGunPanel::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");
    m_pColorPicker->SetPickerColor(selected);

    char buf[64];
    g_pMomentumUtil->GetHexStringFromColor(selected, buf, sizeof(buf));
    r_decal_paintgun_color.SetValue(buf);
}

void PaintGunPanel::Activate()
{
    BaseClass::Activate();

    Color MaterialColor;
    if (g_pMomentumUtil->GetColorFromHex(r_decal_paintgun_color.GetString(), MaterialColor))
    {
        m_pColorPicker->SetPickerColor(MaterialColor);
        m_pColorPicker->Show();
    }
}

void PaintGunPanel::OnThink() { BaseClass::OnThink(); }
