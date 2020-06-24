// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SHADOW_PS20B_H
#define SHADOW_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class shadow_ps20b_Static_Index
{
	unsigned int m_nBLOBBY_SHADOWS : 2;
#ifdef _DEBUG
	bool m_bBLOBBY_SHADOWS : 1;
#endif	// _DEBUG
public:
	void SetBLOBBY_SHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLOBBY_SHADOWS = i;
#ifdef _DEBUG
		m_bBLOBBY_SHADOWS = true;
#endif	// _DEBUG
	}

	shadow_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBLOBBY_SHADOWS = 0;
#ifdef _DEBUG
		m_bBLOBBY_SHADOWS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBLOBBY_SHADOWS );
		return ( 2 * m_nBLOBBY_SHADOWS ) + 0;
	}
};

#define shaderStaticTest_shadow_ps20b psh_forgot_to_set_static_BLOBBY_SHADOWS


class shadow_ps20b_Dynamic_Index
{
	unsigned int m_nPIXELFOGTYPE : 2;
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	shadow_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
	}

	int GetIndex() const
	{
		return ( 1 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_shadow_ps20b 1


#endif	// SHADOW_PS20B_H