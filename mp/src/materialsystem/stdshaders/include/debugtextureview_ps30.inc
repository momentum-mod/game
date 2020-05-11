// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef DEBUGTEXTUREVIEW_PS30_H
#define DEBUGTEXTUREVIEW_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class debugtextureview_ps30_Static_Index
{
	int m_nSHOWALPHA : 3;
#ifdef _DEBUG
	bool m_bSHOWALPHA : 1;
#endif	// _DEBUG
public:
	void SetSHOWALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSHOWALPHA = i;
#ifdef _DEBUG
		m_bSHOWALPHA = true;
#endif	// _DEBUG
	}

	debugtextureview_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nSHOWALPHA = 0;
#ifdef _DEBUG
		m_bSHOWALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSHOWALPHA );
		return ( 2 * m_nSHOWALPHA ) + 0;
	}
};

#define shaderStaticTest_debugtextureview_ps30 psh_forgot_to_set_static_SHOWALPHA


class debugtextureview_ps30_Dynamic_Index
{
	int m_nISCUBEMAP : 3;
#ifdef _DEBUG
	bool m_bISCUBEMAP : 1;
#endif	// _DEBUG
public:
	void SetISCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nISCUBEMAP = i;
#ifdef _DEBUG
		m_bISCUBEMAP = true;
#endif	// _DEBUG
	}

	debugtextureview_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nISCUBEMAP = 0;
#ifdef _DEBUG
		m_bISCUBEMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bISCUBEMAP );
		return ( 1 * m_nISCUBEMAP ) + 0;
	}
};

#define shaderDynamicTest_debugtextureview_ps30 psh_forgot_to_set_dynamic_ISCUBEMAP


#endif	// DEBUGTEXTUREVIEW_PS30_H