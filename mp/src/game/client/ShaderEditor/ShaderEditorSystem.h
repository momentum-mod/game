#ifndef SHEDITSYSTEM_H
#define SHEDITSYSTEM_H

#include "datacache/imdlcache.h"

#include "iviewrender.h"
#include "view_shared.h"
#include "viewrender.h"


class ShaderEditorHandler : public CAutoGameSystemPerFrame
{
public:
	ShaderEditorHandler( char const *name );
	~ShaderEditorHandler();

    bool Init() OVERRIDE;
    void Shutdown() OVERRIDE;

    void Update( float frametime ) OVERRIDE;
    void PreRender() OVERRIDE;
    void PostRender() OVERRIDE;
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