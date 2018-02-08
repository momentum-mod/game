#pragma once

#include "cbase.h"
#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#include "PaintGunPanel.h"
#define CMomentumPaintGun C_MomentumPaintGun
#endif

class CMomentumPaintGun : public CWeaponCSBaseGun
{
  public:
    DECLARE_CLASS(CMomentumPaintGun, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumPaintGun();
    ~CMomentumPaintGun();

    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    CSWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_PAINTGUN; }

    void RifleFire();
    
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
