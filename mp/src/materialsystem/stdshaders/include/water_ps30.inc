// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MULTITEXTURE && $BASETEXTURE
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef WATER_PS30_H
#define WATER_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class water_ps30_Static_Index
{
	int m_nBASETEXTURE : 3;
	int m_nMULTITEXTURE : 3;
	int m_nREFLECT : 3;
	int m_nREFRACT : 3;
	int m_nABOVEWATER : 3;
	int m_nNORMAL_DECODE_MODE : 2;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bMULTITEXTURE : 1;
	bool m_bREFLECT : 1;
	bool m_bREFRACT : 1;
	bool m_bABOVEWATER : 1;
	bool m_bNORMAL_DECODE_MODE : 1;
#endif	// _DEBUG
public:
	void SetBASETEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE = i;
#ifdef _DEBUG
		m_bBASETEXTURE = true;
#endif	// _DEBUG
	}

	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
#endif	// _DEBUG
	}

	void SetREFLECT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFLECT = i;
#ifdef _DEBUG
		m_bREFLECT = true;
#endif	// _DEBUG
	}

	void SetREFRACT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACT = i;
#ifdef _DEBUG
		m_bREFRACT = true;
#endif	// _DEBUG
	}

	void SetABOVEWATER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nABOVEWATER = i;
#ifdef _DEBUG
		m_bABOVEWATER = true;
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

	water_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nMULTITEXTURE = 0;
		m_nREFLECT = 0;
		m_nREFRACT = 0;
		m_nABOVEWATER = 0;
		m_nNORMAL_DECODE_MODE = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bMULTITEXTURE = false;
		m_bREFLECT = false;
		m_bREFRACT = false;
		m_bABOVEWATER = false;
		m_bNORMAL_DECODE_MODE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bMULTITEXTURE && m_bREFLECT && m_bREFRACT && m_bABOVEWATER && m_bNORMAL_DECODE_MODE );
		return ( 2 * m_nBASETEXTURE ) + ( 4 * m_nMULTITEXTURE ) + ( 8 * m_nREFLECT ) + ( 16 * m_nREFRACT ) + ( 32 * m_nABOVEWATER ) + ( 64 * m_nNORMAL_DECODE_MODE ) + 0;
	}
};

#define shaderStaticTest_water_ps30 psh_forgot_to_set_static_BASETEXTURE + psh_forgot_to_set_static_MULTITEXTURE + psh_forgot_to_set_static_REFLECT + psh_forgot_to_set_static_REFRACT + psh_forgot_to_set_static_ABOVEWATER + psh_forgot_to_set_static_NORMAL_DECODE_MODE


class water_ps30_Dynamic_Index
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

	water_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
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

#define shaderDynamicTest_water_ps30 psh_forgot_to_set_dynamic_PIXELFOGTYPE


#endif	// WATER_PS30_H