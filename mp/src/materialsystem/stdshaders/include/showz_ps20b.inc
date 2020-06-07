// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SHOWZ_PS20B_H
#define SHOWZ_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class showz_ps20b_Static_Index
{
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
	unsigned int m_nDEPTH_IN_ALPHA : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
	bool m_bDEPTH_IN_ALPHA : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHTDEPTHFILTERMODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFLASHLIGHTDEPTHFILTERMODE = i;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif	// _DEBUG
	}

	void SetDEPTH_IN_ALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTH_IN_ALPHA = i;
#ifdef _DEBUG
		m_bDEPTH_IN_ALPHA = true;
#endif	// _DEBUG
	}

	showz_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
		m_nDEPTH_IN_ALPHA = 0;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
		m_bDEPTH_IN_ALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHTDEPTHFILTERMODE && m_bDEPTH_IN_ALPHA );
		return ( 1 * m_nFLASHLIGHTDEPTHFILTERMODE ) + ( 3 * m_nDEPTH_IN_ALPHA ) + 0;
	}
};

#define shaderStaticTest_showz_ps20b psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + psh_forgot_to_set_static_DEPTH_IN_ALPHA


class showz_ps20b_Dynamic_Index
{
public:
	showz_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_showz_ps20b 1


#endif	// SHOWZ_PS20B_H