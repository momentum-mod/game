// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef WORLDVERTEXALPHA_PS20B_H
#define WORLDVERTEXALPHA_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class worldvertexalpha_ps20b_Static_Index
{
	int m_nCONVERT_TO_SRGB : 3;
	int m_nPASS : 3;
#ifdef _DEBUG
	bool m_bPASS : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
	}

	void SetPASS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPASS = i;
#ifdef _DEBUG
		m_bPASS = true;
#endif	// _DEBUG
	}

	worldvertexalpha_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = g_pHardwareConfig->NeedsShaderSRGBConversion();
		m_nPASS = 0;
#ifdef _DEBUG
		m_bPASS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bPASS );
		return ( 1 * m_nCONVERT_TO_SRGB ) + ( 2 * m_nPASS ) + 0;
	}
};

#define shaderStaticTest_worldvertexalpha_ps20b psh_forgot_to_set_static_PASS


class worldvertexalpha_ps20b_Dynamic_Index
{
public:
	worldvertexalpha_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_worldvertexalpha_ps20b 1


#endif	// WORLDVERTEXALPHA_PS20B_H