// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef VR_DISTORT_HUD_PS20B_H
#define VR_DISTORT_HUD_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class vr_distort_hud_ps20b_Static_Index
{
public:
	vr_distort_hud_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_vr_distort_hud_ps20b 1


class vr_distort_hud_ps20b_Dynamic_Index
{
	int m_nCMBO_HUDUNDISTORT : 3;
#ifdef _DEBUG
	bool m_bCMBO_HUDUNDISTORT : 1;
#endif	// _DEBUG
public:
	void SetCMBO_HUDUNDISTORT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCMBO_HUDUNDISTORT = i;
#ifdef _DEBUG
		m_bCMBO_HUDUNDISTORT = true;
#endif	// _DEBUG
	}

	vr_distort_hud_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCMBO_HUDUNDISTORT = 0;
#ifdef _DEBUG
		m_bCMBO_HUDUNDISTORT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCMBO_HUDUNDISTORT );
		return ( 1 * m_nCMBO_HUDUNDISTORT ) + 0;
	}
};

#define shaderDynamicTest_vr_distort_hud_ps20b psh_forgot_to_set_dynamic_CMBO_HUDUNDISTORT


#endif	// VR_DISTORT_HUD_PS20B_H