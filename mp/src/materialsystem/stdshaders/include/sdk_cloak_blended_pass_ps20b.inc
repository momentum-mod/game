// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SDK_CLOAK_BLENDED_PASS_PS20B_H
#define SDK_CLOAK_BLENDED_PASS_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_cloak_blended_pass_ps20b_Static_Index
{
	int m_nCONVERT_TO_SRGB : 3;
	int m_nBUMPMAP : 3;
#ifdef _DEBUG
	bool m_bBUMPMAP : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
	}

	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	sdk_cloak_blended_pass_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = g_pHardwareConfig->NeedsShaderSRGBConversion();
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBUMPMAP );
		return ( 1 * m_nCONVERT_TO_SRGB ) + ( 2 * m_nBUMPMAP ) + 0;
	}
};

#define shaderStaticTest_sdk_cloak_blended_pass_ps20b psh_forgot_to_set_static_BUMPMAP


class sdk_cloak_blended_pass_ps20b_Dynamic_Index
{
public:
	sdk_cloak_blended_pass_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_sdk_cloak_blended_pass_ps20b 1


#endif	// SDK_CLOAK_BLENDED_PASS_PS20B_H