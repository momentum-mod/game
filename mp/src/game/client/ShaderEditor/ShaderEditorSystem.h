#ifndef SHEDITSYSTEM_H
#define SHEDITSYSTEM_H

#include "cbase.h"

#include "datacache/imdlcache.h"

#include "iviewrender.h"
#include "view_shared.h"
#include "viewrender.h"


class ShaderEditorHandler : public CAutoGameSystemPerFrame
{
public:
	ShaderEditorHandler( char const *name );
	~ShaderEditorHandler();

    bool Init() override;
    void Shutdown() override;

    void Update( float frametime ) override;
    void PreRender() override;
    void PostRender() override;
	void CustomViewRender( int *viewId, const VisibleFogVolumeInfo_t &fogVolumeInfo, const WaterRenderInfo_t &waterRenderInfo );
	void CustomPostRender() const;
	void UpdateSkymask( bool bCombineMode = false ) const;

	bool IsReady() const;
	int &GetViewIdForModify() const;
	const VisibleFogVolumeInfo_t &GetFogVolumeInfo() const;
	const WaterRenderInfo_t &GetWaterRenderInfo() const;

private:
	bool m_bReady;

	void RegisterCallbacks() const;
	void PrepareCallbackData() const;

	void RegisterViewRenderCallbacks() const;

	int *m_piCurrentViewId;
	VisibleFogVolumeInfo_t m_tFogVolumeInfo;
	WaterRenderInfo_t m_tWaterRenderInfo;
};

extern ShaderEditorHandler *g_ShaderEditorSystem;

#endif