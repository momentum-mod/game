// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef VERTEXLIT_LIGHTING_ONLY_PS20B_H
#define VERTEXLIT_LIGHTING_ONLY_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class vertexlit_lighting_only_ps20b_Static_Index
{
	unsigned int m_nCONVERT_TO_SRGB : 2;
	unsigned int m_nDIFFUSELIGHTING : 2;
	unsigned int m_nHALFLAMBERT : 2;
#ifdef _DEBUG
	bool m_bDIFFUSELIGHTING : 1;
	bool m_bHALFLAMBERT : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
	}

	void SetDIFFUSELIGHTING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDIFFUSELIGHTING = i;
#ifdef _DEBUG
		m_bDIFFUSELIGHTING = true;
#endif	// _DEBUG
	}

	void SetHALFLAMBERT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALFLAMBERT = i;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
#endif	// _DEBUG
	}

	vertexlit_lighting_only_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = g_pHardwareConfig->NeedsShaderSRGBConversion();
		m_nDIFFUSELIGHTING = 0;
		m_nHALFLAMBERT = 0;
#ifdef _DEBUG
		m_bDIFFUSELIGHTING = false;
		m_bHALFLAMBERT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDIFFUSELIGHTING && m_bHALFLAMBERT );
		return ( 10 * m_nCONVERT_TO_SRGB ) + ( 20 * m_nDIFFUSELIGHTING ) + ( 40 * m_nHALFLAMBERT ) + 0;
	}
};

#define shaderStaticTest_vertexlit_lighting_only_ps20b psh_forgot_to_set_static_DIFFUSELIGHTING + psh_forgot_to_set_static_HALFLAMBERT


class vertexlit_lighting_only_ps20b_Dynamic_Index
{
	unsigned int m_nAMBIENT_LIGHT : 2;
	unsigned int m_nNUM_LIGHTS : 3;
#ifdef _DEBUG
	bool m_bAMBIENT_LIGHT : 1;
	bool m_bNUM_LIGHTS : 1;
#endif	// _DEBUG
public:
	void SetAMBIENT_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nAMBIENT_LIGHT = i;
#ifdef _DEBUG
		m_bAMBIENT_LIGHT = true;
#endif	// _DEBUG
	}

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif	// _DEBUG
	}

	vertexlit_lighting_only_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nAMBIENT_LIGHT = 0;
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bAMBIENT_LIGHT = false;
		m_bNUM_LIGHTS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bAMBIENT_LIGHT && m_bNUM_LIGHTS );
		return ( 1 * m_nAMBIENT_LIGHT ) + ( 2 * m_nNUM_LIGHTS ) + 0;
	}
};

#define shaderDynamicTest_vertexlit_lighting_only_ps20b psh_forgot_to_set_dynamic_AMBIENT_LIGHT + psh_forgot_to_set_dynamic_NUM_LIGHTS


#endif	// VERTEXLIT_LIGHTING_ONLY_PS20B_H