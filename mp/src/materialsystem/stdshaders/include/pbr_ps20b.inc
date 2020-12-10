// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($PIXELFOGTYPE == 0) && ($WRITEWATERFOGTODESTALPHA != 0)
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTDEPTHFILTERMODE != 0 )
// !$HSV && $HSV_BLEND
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
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef PBR_PS20B_H
#define PBR_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class pbr_ps20b_Static_Index
{
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
	unsigned int m_nLIGHTMAPPED : 2;
	unsigned int m_nEMISSIVE : 2;
	unsigned int m_nWVT : 2;
	unsigned int m_nHSV : 2;
	unsigned int m_nHSV_BLEND : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHT : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
	bool m_bLIGHTMAPPED : 1;
	bool m_bEMISSIVE : 1;
	bool m_bWVT : 1;
	bool m_bHSV : 1;
	bool m_bHSV_BLEND : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTDEPTHFILTERMODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFLASHLIGHTDEPTHFILTERMODE = i;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif	// _DEBUG
	}

	void SetLIGHTMAPPED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAPPED = i;
#ifdef _DEBUG
		m_bLIGHTMAPPED = true;
#endif	// _DEBUG
	}

	void SetEMISSIVE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEMISSIVE = i;
#ifdef _DEBUG
		m_bEMISSIVE = true;
#endif	// _DEBUG
	}

	void SetWVT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWVT = i;
#ifdef _DEBUG
		m_bWVT = true;
#endif	// _DEBUG
	}

	void SetHSV( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHSV = i;
#ifdef _DEBUG
		m_bHSV = true;
#endif	// _DEBUG
	}

	void SetHSV_BLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHSV_BLEND = i;
#ifdef _DEBUG
		m_bHSV_BLEND = true;
#endif	// _DEBUG
	}

	pbr_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nFLASHLIGHT = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
		m_nLIGHTMAPPED = 0;
		m_nEMISSIVE = 0;
		m_nWVT = 0;
		m_nHSV = 0;
		m_nHSV_BLEND = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
		m_bLIGHTMAPPED = false;
		m_bEMISSIVE = false;
		m_bWVT = false;
		m_bHSV = false;
		m_bHSV_BLEND = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHT && m_bFLASHLIGHTDEPTHFILTERMODE && m_bLIGHTMAPPED && m_bEMISSIVE && m_bWVT && m_bHSV && m_bHSV_BLEND );
		return ( 80 * m_nFLASHLIGHT ) + ( 160 * m_nFLASHLIGHTDEPTHFILTERMODE ) + ( 480 * m_nLIGHTMAPPED ) + ( 960 * m_nEMISSIVE ) + ( 1920 * m_nWVT ) + ( 3840 * m_nHSV ) + ( 7680 * m_nHSV_BLEND ) + 0;
	}
};

#define shaderStaticTest_pbr_ps20b psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + psh_forgot_to_set_static_LIGHTMAPPED + psh_forgot_to_set_static_EMISSIVE + psh_forgot_to_set_static_WVT + psh_forgot_to_set_static_HSV + psh_forgot_to_set_static_HSV_BLEND


class pbr_ps20b_Dynamic_Index
{
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bNUM_LIGHTS : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
#endif	// _DEBUG
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
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

	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	pbr_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nPIXELFOGTYPE = 0;
		m_nNUM_LIGHTS = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
		m_nFLASHLIGHTSHADOWS = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bPIXELFOGTYPE = false;
		m_bNUM_LIGHTS = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
		m_bFLASHLIGHTSHADOWS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bNUM_LIGHTS && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS );
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nNUM_LIGHTS ) + ( 20 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 40 * m_nFLASHLIGHTSHADOWS ) + 0;
	}
};

#define shaderDynamicTest_pbr_ps20b psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS


#endif	// PBR_PS20B_H