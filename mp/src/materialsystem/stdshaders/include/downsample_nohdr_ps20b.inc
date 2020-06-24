// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef DOWNSAMPLE_NOHDR_PS20B_H
#define DOWNSAMPLE_NOHDR_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class downsample_nohdr_ps20b_Static_Index
{
	unsigned int m_nBLOOMTYPE : 2;
#ifdef _DEBUG
	bool m_bBLOOMTYPE : 1;
#endif	// _DEBUG
public:
	void SetBLOOMTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLOOMTYPE = i;
#ifdef _DEBUG
		m_bBLOOMTYPE = true;
#endif	// _DEBUG
	}

	downsample_nohdr_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBLOOMTYPE = 0;
#ifdef _DEBUG
		m_bBLOOMTYPE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBLOOMTYPE );
		return ( 2 * m_nBLOOMTYPE ) + 0;
	}
};

#define shaderStaticTest_downsample_nohdr_ps20b psh_forgot_to_set_static_BLOOMTYPE


class downsample_nohdr_ps20b_Dynamic_Index
{
	unsigned int m_nFLOAT_BACK_BUFFER : 2;
#ifdef _DEBUG
	bool m_bFLOAT_BACK_BUFFER : 1;
#endif	// _DEBUG
public:
	void SetFLOAT_BACK_BUFFER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLOAT_BACK_BUFFER = i;
#ifdef _DEBUG
		m_bFLOAT_BACK_BUFFER = true;
#endif	// _DEBUG
	}

	downsample_nohdr_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFLOAT_BACK_BUFFER = 0;
#ifdef _DEBUG
		m_bFLOAT_BACK_BUFFER = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLOAT_BACK_BUFFER );
		return ( 1 * m_nFLOAT_BACK_BUFFER ) + 0;
	}
};

#define shaderDynamicTest_downsample_nohdr_ps20b psh_forgot_to_set_dynamic_FLOAT_BACK_BUFFER


#endif	// DOWNSAMPLE_NOHDR_PS20B_H