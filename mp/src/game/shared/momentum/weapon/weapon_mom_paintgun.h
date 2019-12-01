#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#include "PaintGunPanel.h"
#define CMomentumPaintGun C_MomentumPaintGun
#else
extern ConVar mom_paintgun_shoot_sound;
#endif

class CMomentumPaintGun : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumPaintGun, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumPaintGun();
    ~CMomentumPaintGun();

    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_PAINTGUN; }
    static float GetPrimaryCycleTime() { return 0.1f; }
    
    void GetControlPanelInfo(int nPanelIndex, const char*& pPanelName) OVERRIDE;

    void DoMuzzleFlash() OVERRIDE {};

#ifdef GAME_DLL
    bool ShouldShowControlPanels() OVERRIDE;
#endif

#ifdef CLIENT_DLL
   bool IsOverridingViewmodel() OVERRIDE;
   int DrawOverriddenViewmodel(C_BaseViewModel* pViewmodel, int flags) OVERRIDE;
private:
    vgui::PaintGunPanel *m_pSettingsPanel;
    ConVarRef m_cvarDrawPaintgun;
#endif
};
