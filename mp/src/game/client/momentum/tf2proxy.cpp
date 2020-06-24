//================================================================================================================//
// Purpose: Fakes material proxies from TF2 to stop models from turning pitch black and remove console error spew
//================================================================================================================//
#include "cbase.h"

#include <KeyValues.h>

#include "baseanimatedtextureproxy.h"
#include "functionproxy.h"

#include "tier0/memdbgon.h"

class CYellowLevelProxy : public CResultProxy
{
  public:
    void OnBind(void *pC_BaseEntity)
    {
        Assert(m_pResult);

        if (!pC_BaseEntity)
            return;

        C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
        if (!pEntity)
            return;

        // stop stuff from going black
        m_pResult->SetVecValue(1.0f, 1.0f, 1.0f);
    }
};

EXPOSE_INTERFACE(CYellowLevelProxy, IMaterialProxy, "YellowLevel" IMATERIAL_PROXY_INTERFACE_VERSION);

class CBurnLevelProxy : public CResultProxy
{
  public:
    void OnBind(void *pC_BaseEntity)
    {
        m_pResult->SetFloatValue(0.0f);
    }
};

EXPOSE_INTERFACE(CBurnLevelProxy, IMaterialProxy, "BurnLevel" IMATERIAL_PROXY_INTERFACE_VERSION);

class CInvulnLevelProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CInvulnLevelProxy, IMaterialProxy, "InvulnLevel" IMATERIAL_PROXY_INTERFACE_VERSION);

class CSpyInvisProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CSpyInvisProxy, IMaterialProxy, "spy_invis" IMATERIAL_PROXY_INTERFACE_VERSION);

class CWeaponInvisProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CWeaponInvisProxy, IMaterialProxy, "weapon_invis" IMATERIAL_PROXY_INTERFACE_VERSION);

class CInvisProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CInvisProxy, IMaterialProxy, "invis" IMATERIAL_PROXY_INTERFACE_VERSION);

class CModelGlowColorProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CModelGlowColorProxy, IMaterialProxy, "ModelGlowColor" IMATERIAL_PROXY_INTERFACE_VERSION);

class CItemTintColorProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CItemTintColorProxy, IMaterialProxy, "ItemTintColor" IMATERIAL_PROXY_INTERFACE_VERSION);

class CStickybombGlowColorProxy : public CResultProxy
{
  public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CStickybombGlowColorProxy, IMaterialProxy, "StickybombGlowColor" IMATERIAL_PROXY_INTERFACE_VERSION);

class CAnimatedWeaponSheenProxy : public CBaseAnimatedTextureProxy
{
  public:
    CAnimatedWeaponSheenProxy() {}
    ~CAnimatedWeaponSheenProxy() {}

    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) { return true; }
    void OnBind(void *pC_BaseEntity) {}
    float GetAnimationStartTime(void *pBaseEntity) { return 0; }
    IMaterial *GetMaterial() { return nullptr; }
};

EXPOSE_INTERFACE(CAnimatedWeaponSheenProxy, IMaterialProxy, "AnimatedWeaponSheen" IMATERIAL_PROXY_INTERFACE_VERSION);