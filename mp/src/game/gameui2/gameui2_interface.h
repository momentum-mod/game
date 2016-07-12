#pragma once

#include "igameui2.h"

#include "cdll_int.h"
#include "engine/ienginesound.h"
#include "ienginevgui.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "ivrenderview.h"
#include "view_shared.h"
#include "GameUI/IGameUI.h"

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
    void		Initialize(CreateInterfaceFn appFactory) override;
    void		Shutdown() override;

    void		OnInitialize() override;
    void		OnShutdown() override;
    void		OnUpdate() override;
    void		OnLevelInitializePreEntity() override;
    void		OnLevelInitializePostEntity() override;
    void		OnLevelShutdown() override;

    bool		IsInLevel() override;
    bool		IsInBackgroundLevel() override;
    bool		IsInMultiplayer() override;
    bool		IsInLoading() override;

    Vector2D	GetViewport() override;
    float		GetTime() override;
    vgui::VPANEL GetRootPanel() override;
    vgui::VPANEL GetVPanel() override;
    CViewSetup	GetView() override;
    VPlane*		GetFrustum() override;
    ITexture*	GetMaskTexture() override;
    wchar_t*	GetLocalizedString(const char* text) override;

    void		SetView(const CViewSetup& view) override;
    void		SetFrustum(VPlane* frustum) override;
    void		SetMaskTexture(ITexture* maskTexture) override;

    void SendMainMenuCommand(const char* cmd);

private:
	CViewSetup			m_pView;
	VPlane*				m_pFrustum;
	ITexture*			m_pMaskTexture;
};

extern CGameUI2& GameUI2();
extern IVEngineClient* engine;
extern IEngineSound* enginesound;
extern IEngineVGui* enginevgui;
extern ISoundEmitterSystemBase* soundemitterbase;