// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef WATERCHEAP_PS30_H
#define WATERCHEAP_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class watercheap_ps30_Static_Index
{
	int m_nMULTITEXTURE : 3;
	int m_nFRESNEL : 3;
	int m_nBLEND : 3;
	int m_nREFRACTALPHA : 3;
	int m_nHDRTYPE : 3;
	int m_nNORMAL_DECODE_MODE : 2;
#ifdef _DEBUG
	bool m_bMULTITEXTURE : 1;
	bool m_bFRESNEL : 1;
	bool m_bBLEND : 1;
	bool m_bREFRACTALPHA : 1;
	bool m_bHDRTYPE : 1;
	bool m_bNORMAL_DECODE_MODE : 1;
#endif	// _DEBUG
public:
	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
#endif	// _DEBUG
	}

	void SetFRESNEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNEL = i;
#ifdef _DEBUG
		m_bFRESNEL = true;
#endif	// _DEBUG
	}

	void SetBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLEND = i;
#ifdef _DEBUG
		m_bBLEND = true;
#endif	// _DEBUG
	}

	void SetREFRACTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACTALPHA = i;
#ifdef _DEBUG
		m_bREFRACTALPHA = true;
#endif	// _DEBUG
	}

	void SetHDRTYPE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nHDRTYPE = i;
#ifdef _DEBUG
		m_bHDRTYPE = true;
#endif	// _DEBUG
	}

	void SetNORMAL_DECODE_MODE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nNORMAL_DECODE_MODE = i;
#ifdef _DEBUG
		m_bNORMAL_DECODE_MODE = true;
#endif	// _DEBUG
	}

	watercheap_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMULTITEXTURE = 0;
		m_nFRESNEL = 0;
		m_nBLEND = 0;
		m_nREFRACTALPHA = 0;
		m_nHDRTYPE = 0;
		m_nNORMAL_DECODE_MODE = 0;
#ifdef _DEBUG
		m_bMULTITEXTURE = false;
		m_bFRESNEL = false;
		m_bBLEND = false;
		m_bREFRACTALPHA = false;
		m_bHDRTYPE = false;
		m_bNORMAL_DECODE_MODE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMULTITEXTURE && m_bFRESNEL && m_bBLEND && m_bREFRACTALPHA && m_bHDRTYPE && m_bNORMAL_DECODE_MODE );
		return ( 4 * m_nMULTITEXTURE ) + ( 8 * m_nFRESNEL ) + ( 16 * m_nBLEND ) + ( 32 * m_nREFRACTALPHA ) + ( 64 * m_nHDRTYPE ) + ( 192 * m_nNORMAL_DECODE_MODE ) + 0;
	}
};

#define shaderStaticTest_watercheap_ps30 psh_forgot_to_set_static_MULTITEXTURE + psh_forgot_to_set_static_FRESNEL + psh_forgot_to_set_static_BLEND + psh_forgot_to_set_static_REFRACTALPHA + psh_forgot_to_set_static_HDRTYPE + psh_forgot_to_set_static_NORMAL_DECODE_MODE


class watercheap_ps30_Dynamic_Index
{
	int m_nHDRENABLED : 3;
	int m_nPIXELFOGTYPE : 3;
#ifdef _DEBUG
	bool m_bHDRENABLED : 1;
	bool m_bPIXELFOGTYPE : 1;
#endif	// _DEBUG
public:
	void SetHDRENABLED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHDRENABLED = i;
#ifdef _DEBUG
		m_bHDRENABLED = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif	// _DEBUG
	}

	watercheap_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nHDRENABLED = 0;
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bHDRENABLED = false;
		m_bPIXELFOGTYPE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bHDRENABLED && m_bPIXELFOGTYPE );
		return ( 1 * m_nHDRENABLED ) + ( 2 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_watercheap_ps30 psh_forgot_to_set_dynamic_HDRENABLED + psh_forgot_to_set_dynamic_PIXELFOGTYPE


#endif	// WATERCHEAP_PS30_H