// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $FLASHLIGHT != 0 ) && ( $NUM_LIGHTS > 0 )
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $RAYTRACESPHERE == 0 ) && ( $SPHERETEXKILLCOMBO == 1 )
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

#ifndef EYE_REFRACT_PS30_H
#define EYE_REFRACT_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class eye_refract_ps30_Static_Index
{
	int m_nFLASHLIGHT : 3;
	int m_nLIGHTWARPTEXTURE : 3;
	int m_nSPHERETEXKILLCOMBO : 3;
	int m_nRAYTRACESPHERE : 3;
	int m_nFLASHLIGHTDEPTHFILTERMODE : 3;
#ifdef _DEBUG
	bool m_bFLASHLIGHT : 1;
	bool m_bLIGHTWARPTEXTURE : 1;
	bool m_bSPHERETEXKILLCOMBO : 1;
	bool m_bRAYTRACESPHERE : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
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

	void SetLIGHTWARPTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTWARPTEXTURE = i;
#ifdef _DEBUG
		m_bLIGHTWARPTEXTURE = true;
#endif	// _DEBUG
	}

	void SetSPHERETEXKILLCOMBO( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSPHERETEXKILLCOMBO = i;
#ifdef _DEBUG
		m_bSPHERETEXKILLCOMBO = true;
#endif	// _DEBUG
	}

	void SetRAYTRACESPHERE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nRAYTRACESPHERE = i;
#ifdef _DEBUG
		m_bRAYTRACESPHERE = true;
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

	eye_refract_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nFLASHLIGHT = 0;
		m_nLIGHTWARPTEXTURE = 0;
		m_nSPHERETEXKILLCOMBO = 0;
		m_nRAYTRACESPHERE = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
		m_bLIGHTWARPTEXTURE = false;
		m_bSPHERETEXKILLCOMBO = false;
		m_bRAYTRACESPHERE = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHT && m_bLIGHTWARPTEXTURE && m_bSPHERETEXKILLCOMBO && m_bRAYTRACESPHERE && m_bFLASHLIGHTDEPTHFILTERMODE );
		return ( 10 * m_nFLASHLIGHT ) + ( 20 * m_nLIGHTWARPTEXTURE ) + ( 40 * m_nSPHERETEXKILLCOMBO ) + ( 80 * m_nRAYTRACESPHERE ) + ( 160 * m_nFLASHLIGHTDEPTHFILTERMODE ) + 0;
	}
};

#define shaderStaticTest_eye_refract_ps30 psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_LIGHTWARPTEXTURE + psh_forgot_to_set_static_SPHERETEXKILLCOMBO + psh_forgot_to_set_static_RAYTRACESPHERE + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE


class eye_refract_ps30_Dynamic_Index
{
	int m_nNUM_LIGHTS : 4;
	int m_nFLASHLIGHTSHADOWS : 3;
#ifdef _DEBUG
	bool m_bNUM_LIGHTS : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
#endif	// _DEBUG
public:
	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
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

	eye_refract_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nNUM_LIGHTS = 0;
		m_nFLASHLIGHTSHADOWS = 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
		m_bFLASHLIGHTSHADOWS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bNUM_LIGHTS && m_bFLASHLIGHTSHADOWS );
		return ( 1 * m_nNUM_LIGHTS ) + ( 5 * m_nFLASHLIGHTSHADOWS ) + 0;
	}
};

#define shaderDynamicTest_eye_refract_ps30 psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS


#endif	// EYE_REFRACT_PS30_H