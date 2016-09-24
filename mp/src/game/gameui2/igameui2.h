#pragma once

#ifdef GAMEUI2_DLL
#include "tier1/interface.h"
#else
#include "interface.h"
#endif
#include "vgui/VGUI.h"
#include "mathlib/vector.h"
#include "ivrenderview.h"

abstract_class IGameUI2 : public IBaseInterface
{
public:
	virtual void		Initialize(CreateInterfaceFn appFactory) = 0;
	virtual void		Shutdown() = 0;

	virtual void		OnInitialize() = 0;
	virtual void		OnShutdown() = 0;
	virtual void		OnUpdate() = 0;
	virtual void		OnLevelInitializePreEntity() = 0;
	virtual void		OnLevelInitializePostEntity() = 0;
	virtual void		OnLevelShutdown() = 0;

	virtual bool		IsInLevel() = 0;
	virtual bool		IsInBackgroundLevel() = 0;
	virtual bool		IsInMultiplayer() = 0;
	virtual bool		IsInLoading() = 0;

	virtual Vector2D	GetViewport() = 0;
	virtual float		GetTime() = 0;
	virtual vgui::VPANEL GetRootPanel() = 0;
	virtual	vgui::VPANEL GetVPanel() = 0;
	virtual CViewSetup	GetView() = 0;
	virtual VPlane*		GetFrustum() = 0;
	virtual ITexture*	GetMaskTexture() = 0;
	virtual wchar_t*	GetLocalizedString(const char* text) = 0;

	virtual void		SetView(const CViewSetup& view) = 0;
	virtual void		SetFrustum(VPlane* frustum) = 0;
	virtual void		SetMaskTexture(ITexture* maskTexture) = 0;
};

#define GAMEUI2_DLL_INTERFACE_VERSION "GAMEUI2_V002"
