// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef PARTICLESPHERE_PS20B_H
#define PARTICLESPHERE_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class particlesphere_ps20b_Static_Index
{
	unsigned int m_nDEPTHBLEND : 2;
#ifdef _DEBUG
	bool m_bDEPTHBLEND : 1;
#endif	// _DEBUG
public:
	void SetDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTHBLEND = i;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif	// _DEBUG
	}

	particlesphere_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nDEPTHBLEND = 0;
#ifdef _DEBUG
		m_bDEPTHBLEND = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDEPTHBLEND );
		return ( 2 * m_nDEPTHBLEND ) + 0;
	}
};

#define shaderStaticTest_particlesphere_ps20b psh_forgot_to_set_static_DEPTHBLEND


class particlesphere_ps20b_Dynamic_Index
{
	unsigned int m_nPIXELFOGTYPE : 2;
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	particlesphere_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
	}

	int GetIndex() const
	{
		return ( 1 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_particlesphere_ps20b 1


#endif	// PARTICLESPHERE_PS20B_H