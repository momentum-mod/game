// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SPLINEROPE_PS20B_H
#define SPLINEROPE_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class splinerope_ps20b_Static_Index
{
	unsigned int m_nSHADER_SRGB_READ : 1;
	unsigned int m_nSHADOWDEPTH : 2;
#ifdef _DEBUG
	bool m_bSHADER_SRGB_READ : 1;
	bool m_bSHADOWDEPTH : 1;
#endif	// _DEBUG
public:
	void SetSHADER_SRGB_READ( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nSHADER_SRGB_READ = i;
#ifdef _DEBUG
		m_bSHADER_SRGB_READ = true;
#endif	// _DEBUG
	}

	void SetSHADOWDEPTH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSHADOWDEPTH = i;
#ifdef _DEBUG
		m_bSHADOWDEPTH = true;
#endif	// _DEBUG
	}

	splinerope_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nSHADER_SRGB_READ = 0;
		m_nSHADOWDEPTH = 0;
#ifdef _DEBUG
		m_bSHADER_SRGB_READ = false;
		m_bSHADOWDEPTH = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSHADER_SRGB_READ && m_bSHADOWDEPTH );
		return ( 4 * m_nSHADER_SRGB_READ ) + ( 4 * m_nSHADOWDEPTH ) + 0;
	}
};

#define shaderStaticTest_splinerope_ps20b psh_forgot_to_set_static_SHADER_SRGB_READ + psh_forgot_to_set_static_SHADOWDEPTH


class splinerope_ps20b_Dynamic_Index
{
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	splinerope_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_splinerope_ps20b psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA


#endif	// SPLINEROPE_PS20B_H