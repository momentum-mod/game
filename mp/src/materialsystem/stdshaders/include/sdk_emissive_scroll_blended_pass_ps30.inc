// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SDK_EMISSIVE_SCROLL_BLENDED_PASS_PS30_H
#define SDK_EMISSIVE_SCROLL_BLENDED_PASS_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_emissive_scroll_blended_pass_ps30_Static_Index
{
	int m_nCONVERT_TO_SRGB : 3;
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
	}

	sdk_emissive_scroll_blended_pass_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = g_pHardwareConfig->NeedsShaderSRGBConversion();
	}

	int GetIndex()
	{
		return ( 1 * m_nCONVERT_TO_SRGB ) + 0;
	}
};

#define shaderStaticTest_sdk_emissive_scroll_blended_pass_ps30 1


class sdk_emissive_scroll_blended_pass_ps30_Dynamic_Index
{
public:
	sdk_emissive_scroll_blended_pass_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex()
	{
		return 0;
	}
};

#define shaderDynamicTest_sdk_emissive_scroll_blended_pass_ps30 1


#endif	// SDK_EMISSIVE_SCROLL_BLENDED_PASS_PS30_H