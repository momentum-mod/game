// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SPLINECARD_VS20_H
#define SPLINECARD_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class splinecard_vs20_Static_Index
{
	unsigned int m_nADDBASETEXTURE2 : 1;
	unsigned int m_nEXTRACTGREENALPHA : 1;
	unsigned int m_nDUALSEQUENCE : 1;
	unsigned int m_nDEPTHBLEND : 1;
	unsigned int m_nPACKED_INTERPOLATOR : 1;
	unsigned int m_nANIMBLEND_OR_MAXLUMFRAMEBLEND1 : 2;
#ifdef _DEBUG
	bool m_bADDBASETEXTURE2 : 1;
	bool m_bEXTRACTGREENALPHA : 1;
	bool m_bDUALSEQUENCE : 1;
	bool m_bDEPTHBLEND : 1;
	bool m_bPACKED_INTERPOLATOR : 1;
	bool m_bANIMBLEND_OR_MAXLUMFRAMEBLEND1 : 1;
#endif	// _DEBUG
public:
	void SetADDBASETEXTURE2( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nADDBASETEXTURE2 = i;
#ifdef _DEBUG
		m_bADDBASETEXTURE2 = true;
#endif	// _DEBUG
	}

	void SetEXTRACTGREENALPHA( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nEXTRACTGREENALPHA = i;
#ifdef _DEBUG
		m_bEXTRACTGREENALPHA = true;
#endif	// _DEBUG
	}

	void SetDUALSEQUENCE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nDUALSEQUENCE = i;
#ifdef _DEBUG
		m_bDUALSEQUENCE = true;
#endif	// _DEBUG
	}

	void SetDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nDEPTHBLEND = i;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif	// _DEBUG
	}

	void SetPACKED_INTERPOLATOR( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nPACKED_INTERPOLATOR = i;
#ifdef _DEBUG
		m_bPACKED_INTERPOLATOR = true;
#endif	// _DEBUG
	}

	void SetANIMBLEND_OR_MAXLUMFRAMEBLEND1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nANIMBLEND_OR_MAXLUMFRAMEBLEND1 = i;
#ifdef _DEBUG
		m_bANIMBLEND_OR_MAXLUMFRAMEBLEND1 = true;
#endif	// _DEBUG
	}

	splinecard_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nADDBASETEXTURE2 = 0;
		m_nEXTRACTGREENALPHA = 0;
		m_nDUALSEQUENCE = 0;
		m_nDEPTHBLEND = 0;
		m_nPACKED_INTERPOLATOR = 0;
		m_nANIMBLEND_OR_MAXLUMFRAMEBLEND1 = 0;
#ifdef _DEBUG
		m_bADDBASETEXTURE2 = false;
		m_bEXTRACTGREENALPHA = false;
		m_bDUALSEQUENCE = false;
		m_bDEPTHBLEND = false;
		m_bPACKED_INTERPOLATOR = false;
		m_bANIMBLEND_OR_MAXLUMFRAMEBLEND1 = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bADDBASETEXTURE2 && m_bEXTRACTGREENALPHA && m_bDUALSEQUENCE && m_bDEPTHBLEND && m_bPACKED_INTERPOLATOR && m_bANIMBLEND_OR_MAXLUMFRAMEBLEND1 );
		return ( 4 * m_nADDBASETEXTURE2 ) + ( 4 * m_nEXTRACTGREENALPHA ) + ( 4 * m_nDUALSEQUENCE ) + ( 4 * m_nDEPTHBLEND ) + ( 4 * m_nPACKED_INTERPOLATOR ) + ( 4 * m_nANIMBLEND_OR_MAXLUMFRAMEBLEND1 ) + 0;
	}
};

#define shaderStaticTest_splinecard_vs20 vsh_forgot_to_set_static_ADDBASETEXTURE2 + vsh_forgot_to_set_static_EXTRACTGREENALPHA + vsh_forgot_to_set_static_DUALSEQUENCE + vsh_forgot_to_set_static_DEPTHBLEND + vsh_forgot_to_set_static_PACKED_INTERPOLATOR + vsh_forgot_to_set_static_ANIMBLEND_OR_MAXLUMFRAMEBLEND1


class splinecard_vs20_Dynamic_Index
{
	unsigned int m_nORIENTATION : 3;
#ifdef _DEBUG
	bool m_bORIENTATION : 1;
#endif	// _DEBUG
public:
	void SetORIENTATION( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nORIENTATION = i;
#ifdef _DEBUG
		m_bORIENTATION = true;
#endif	// _DEBUG
	}

	splinecard_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nORIENTATION = 0;
#ifdef _DEBUG
		m_bORIENTATION = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bORIENTATION );
		return ( 1 * m_nORIENTATION ) + 0;
	}
};

#define shaderDynamicTest_splinecard_vs20 vsh_forgot_to_set_dynamic_ORIENTATION


#endif	// SPLINECARD_VS20_H