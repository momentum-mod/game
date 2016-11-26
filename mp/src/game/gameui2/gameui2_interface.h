#pragma once

#include "igameui2.h"

#include "cdll_int.h"
#include "engine/IEngineSound.h"
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
    void		Initialize(CreateInterfaceFn appFactory) OVERRIDE;
    void		Shutdown() OVERRIDE;

    void		OnInitialize() OVERRIDE;
    void		OnShutdown() OVERRIDE;
    void		OnUpdate() OVERRIDE;
    void		OnLevelInitializePreEntity() OVERRIDE;
    void		OnLevelInitializePostEntity() OVERRIDE;
    void		OnLevelShutdown() OVERRIDE;

    bool		IsInLevel() OVERRIDE;
    bool		IsInBackgroundLevel() OVERRIDE;
    bool		IsInMultiplayer() OVERRIDE;
    bool		IsInLoading() OVERRIDE;

    Vector2D	GetViewport() OVERRIDE;
    float		GetTime() OVERRIDE;
    vgui::VPANEL GetRootPanel() OVERRIDE;
    vgui::VPANEL GetVPanel() OVERRIDE;
    CViewSetup	GetView() OVERRIDE;
    VPlane*		GetFrustum() OVERRIDE;
    ITexture*	GetMaskTexture() OVERRIDE;
    wchar_t*	GetLocalizedString(const char* text) OVERRIDE;

    void		SetView(const CViewSetup& view) OVERRIDE;
    void		SetFrustum(VPlane* frustum) OVERRIDE;
    void		SetMaskTexture(ITexture* maskTexture) OVERRIDE;

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
