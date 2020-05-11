// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef WORLDVERTEXALPHA_PS30_H
#define WORLDVERTEXALPHA_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class worldvertexalpha_ps30_Static_Index
{
	int m_nPASS : 3;
#ifdef _DEBUG
	bool m_bPASS : 1;
#endif	// _DEBUG
public:
	void SetPASS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPASS = i;
#ifdef _DEBUG
		m_bPASS = true;
#endif	// _DEBUG
	}

	worldvertexalpha_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nPASS = 0;
#ifdef _DEBUG
		m_bPASS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bPASS );
		return ( 1 * m_nPASS ) + 0;
	}
};

#define shaderStaticTest_worldvertexalpha_ps30 psh_forgot_to_set_static_PASS


class worldvertexalpha_ps30_Dynamic_Index
{
public:
	worldvertexalpha_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_worldvertexalpha_ps30 1


#endif	// WORLDVERTEXALPHA_PS30_H