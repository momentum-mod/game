// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef BLURFILTER_PS20B_H
#define BLURFILTER_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class blurfilter_ps20b_Static_Index
{
	unsigned int m_nKERNEL : 3;
	unsigned int m_nCLEAR_COLOR : 2;
#ifdef _DEBUG
	bool m_bKERNEL : 1;
	bool m_bCLEAR_COLOR : 1;
#endif	// _DEBUG
public:
	void SetKERNEL( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nKERNEL = i;
#ifdef _DEBUG
		m_bKERNEL = true;
#endif	// _DEBUG
	}

	void SetCLEAR_COLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCLEAR_COLOR = i;
#ifdef _DEBUG
		m_bCLEAR_COLOR = true;
#endif	// _DEBUG
	}

	blurfilter_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nKERNEL = 0;
		m_nCLEAR_COLOR = 0;
#ifdef _DEBUG
		m_bKERNEL = false;
		m_bCLEAR_COLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bKERNEL && m_bCLEAR_COLOR );
		return ( 1 * m_nKERNEL ) + ( 5 * m_nCLEAR_COLOR ) + 0;
	}
};

#define shaderStaticTest_blurfilter_ps20b psh_forgot_to_set_static_KERNEL + psh_forgot_to_set_static_CLEAR_COLOR


class blurfilter_ps20b_Dynamic_Index
{
public:
	blurfilter_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_blurfilter_ps20b 1


#endif	// BLURFILTER_PS20B_H