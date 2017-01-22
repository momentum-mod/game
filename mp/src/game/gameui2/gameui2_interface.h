#pragma once

#include "igameui2.h"

#include "GameUI/IGameUI.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "cdll_int.h"
#include "engine/IEngineSound.h"
#include "ienginevgui.h"
#include "ivrenderview.h"
#include "vgui_controls/AnimationController.h"
#include "view_shared.h"

class IVEngineClient;
class IEngineSound;
class IEngineVGui;
class ISoundEmitterSystemBase;
class IVRenderView;
class IGameUI;

#define SCALE(num) vgui::scheme()->GetProportionalScaledValue(num)

class CGameUI2 : public IGameUI2
{
  public:
    void Initialize(CreateInterfaceFn appFactory) OVERRIDE;
    void Shutdown() OVERRIDE;

    void OnInitialize() OVERRIDE;
    void OnShutdown() OVERRIDE;
    void OnUpdate() OVERRIDE;
    void OnLevelInitializePreEntity() OVERRIDE;
    void OnLevelInitializePostEntity() OVERRIDE;
    void OnLevelShutdown() OVERRIDE;

    bool IsInLevel() OVERRIDE;
    bool IsInBackgroundLevel() OVERRIDE;
    bool IsInMultiplayer() OVERRIDE;
    bool IsInLoading() OVERRIDE;

    virtual Vector2D GetViewport() const;
    virtual vgui::VPANEL GetRootPanel() const;
    virtual vgui::VPANEL GetVPanel() const;

    virtual wchar_t* GetLocalizedString(const char* text);

    virtual vgui::AnimationController *GetAnimationController() const { return m_pAnimationController; }

    virtual float GetTime() const { return Plat_FloatTime(); }
    virtual CViewSetup GetView() const { return m_pView; }
    virtual VPlane *GetFrustum() const { return m_pFrustum; }
    virtual ITexture *GetMaskTexture() const { return m_pMaskTexture; }

    void SetView(const CViewSetup &view) OVERRIDE;
    void SetFrustum(VPlane *frustum) OVERRIDE;
    void SetMaskTexture(ITexture *maskTexture) OVERRIDE;

    virtual IVEngineClient* GetEngineClient() const { return m_pEngine; }
    virtual IEngineSound* GetEngineSound() const { return m_pEngineSound; }
    virtual IEngineVGui* GetEngineVGui() const { return m_pEngineVGUI; }
    virtual ISoundEmitterSystemBase* GetSoundEmitterSystemBase() const { return m_pSoundEmitterBase; }
    virtual IVRenderView* GetRenderView() const { return m_pRenderView; }
    virtual IMaterialSystem* GetMaterialSystem() const { return m_pMaterialSystem; }
    virtual IGameUI* GetGameUI() const { return m_pGameUI; }

  private:
    CViewSetup m_pView;
    VPlane *m_pFrustum;
    ITexture *m_pMaskTexture;

    vgui::AnimationController *m_pAnimationController;

    IVEngineClient *m_pEngine;
    IEngineSound *m_pEngineSound;
    IEngineVGui *m_pEngineVGUI;
    ISoundEmitterSystemBase *m_pSoundEmitterBase;
    IGameUI *m_pGameUI;
    IVRenderView *m_pRenderView;
    IMaterialSystem *m_pMaterialSystem;
};

extern CGameUI2 &GameUI2();