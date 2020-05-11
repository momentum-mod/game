// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MASKED && $BLUR
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

#ifndef REFRACT_PS30_H
#define REFRACT_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class refract_ps30_Static_Index
{
	int m_nBLUR : 3;
	int m_nFADEOUTONSILHOUETTE : 3;
	int m_nCUBEMAP : 3;
	int m_nREFRACTTINTTEXTURE : 3;
	int m_nMASKED : 3;
	int m_nCOLORMODULATE : 3;
	int m_nSECONDARY_NORMAL : 3;
	int m_nNORMAL_DECODE_MODE : 2;
#ifdef _DEBUG
	bool m_bBLUR : 1;
	bool m_bFADEOUTONSILHOUETTE : 1;
	bool m_bCUBEMAP : 1;
	bool m_bREFRACTTINTTEXTURE : 1;
	bool m_bMASKED : 1;
	bool m_bCOLORMODULATE : 1;
	bool m_bSECONDARY_NORMAL : 1;
	bool m_bNORMAL_DECODE_MODE : 1;
#endif	// _DEBUG
public:
	void SetBLUR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLUR = i;
#ifdef _DEBUG
		m_bBLUR = true;
#endif	// _DEBUG
	}

	void SetFADEOUTONSILHOUETTE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFADEOUTONSILHOUETTE = i;
#ifdef _DEBUG
		m_bFADEOUTONSILHOUETTE = true;
#endif	// _DEBUG
	}

	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif	// _DEBUG
	}

	void SetREFRACTTINTTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACTTINTTEXTURE = i;
#ifdef _DEBUG
		m_bREFRACTTINTTEXTURE = true;
#endif	// _DEBUG
	}

	void SetMASKED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMASKED = i;
#ifdef _DEBUG
		m_bMASKED = true;
#endif	// _DEBUG
	}

	void SetCOLORMODULATE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLORMODULATE = i;
#ifdef _DEBUG
		m_bCOLORMODULATE = true;
#endif	// _DEBUG
	}

	void SetSECONDARY_NORMAL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSECONDARY_NORMAL = i;
#ifdef _DEBUG
		m_bSECONDARY_NORMAL = true;
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

	refract_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBLUR = 0;
		m_nFADEOUTONSILHOUETTE = 0;
		m_nCUBEMAP = 0;
		m_nREFRACTTINTTEXTURE = 0;
		m_nMASKED = 0;
		m_nCOLORMODULATE = 0;
		m_nSECONDARY_NORMAL = 0;
		m_nNORMAL_DECODE_MODE = 0;
#ifdef _DEBUG
		m_bBLUR = false;
		m_bFADEOUTONSILHOUETTE = false;
		m_bCUBEMAP = false;
		m_bREFRACTTINTTEXTURE = false;
		m_bMASKED = false;
		m_bCOLORMODULATE = false;
		m_bSECONDARY_NORMAL = false;
		m_bNORMAL_DECODE_MODE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBLUR && m_bFADEOUTONSILHOUETTE && m_bCUBEMAP && m_bREFRACTTINTTEXTURE && m_bMASKED && m_bCOLORMODULATE && m_bSECONDARY_NORMAL && m_bNORMAL_DECODE_MODE );
		return ( 2 * m_nBLUR ) + ( 4 * m_nFADEOUTONSILHOUETTE ) + ( 8 * m_nCUBEMAP ) + ( 16 * m_nREFRACTTINTTEXTURE ) + ( 32 * m_nMASKED ) + ( 64 * m_nCOLORMODULATE ) + ( 128 * m_nSECONDARY_NORMAL ) + ( 256 * m_nNORMAL_DECODE_MODE ) + 0;
	}
};

#define shaderStaticTest_refract_ps30 psh_forgot_to_set_static_BLUR + psh_forgot_to_set_static_FADEOUTONSILHOUETTE + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_REFRACTTINTTEXTURE + psh_forgot_to_set_static_MASKED + psh_forgot_to_set_static_COLORMODULATE + psh_forgot_to_set_static_SECONDARY_NORMAL + psh_forgot_to_set_static_NORMAL_DECODE_MODE


class refract_ps30_Dynamic_Index
{
	int m_nPIXELFOGTYPE : 3;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE : 1;
#endif	// _DEBUG
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif	// _DEBUG
	}

	refract_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bPIXELFOGTYPE );
		return ( 1 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_refract_ps30 psh_forgot_to_set_dynamic_PIXELFOGTYPE


#endif	// REFRACT_PS30_H