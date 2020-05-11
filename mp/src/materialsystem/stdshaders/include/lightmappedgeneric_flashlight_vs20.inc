// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H
#define LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class lightmappedgeneric_flashlight_vs20_Static_Index
{
	int m_nNORMALMAP : 3;
	int m_nWORLDVERTEXTRANSITION : 3;
	int m_nSEAMLESS : 3;
	int m_nDETAIL : 3;
#ifdef _DEBUG
	bool m_bNORMALMAP : 1;
	bool m_bWORLDVERTEXTRANSITION : 1;
	bool m_bSEAMLESS : 1;
	bool m_bDETAIL : 1;
#endif	// _DEBUG
public:
	void SetNORMALMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMALMAP = i;
#ifdef _DEBUG
		m_bNORMALMAP = true;
#endif	// _DEBUG
	}

	void SetWORLDVERTEXTRANSITION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWORLDVERTEXTRANSITION = i;
#ifdef _DEBUG
		m_bWORLDVERTEXTRANSITION = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif	// _DEBUG
	}

	void SetDETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAIL = i;
#ifdef _DEBUG
		m_bDETAIL = true;
#endif	// _DEBUG
	}

	lightmappedgeneric_flashlight_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nNORMALMAP = 0;
		m_nWORLDVERTEXTRANSITION = 0;
		m_nSEAMLESS = 0;
		m_nDETAIL = 0;
#ifdef _DEBUG
		m_bNORMALMAP = false;
		m_bWORLDVERTEXTRANSITION = false;
		m_bSEAMLESS = false;
		m_bDETAIL = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bNORMALMAP && m_bWORLDVERTEXTRANSITION && m_bSEAMLESS && m_bDETAIL );
		return ( 2 * m_nNORMALMAP ) + ( 4 * m_nWORLDVERTEXTRANSITION ) + ( 8 * m_nSEAMLESS ) + ( 16 * m_nDETAIL ) + 0;
	}
};

#define shaderStaticTest_lightmappedgeneric_flashlight_vs20 vsh_forgot_to_set_static_NORMALMAP + vsh_forgot_to_set_static_WORLDVERTEXTRANSITION + vsh_forgot_to_set_static_SEAMLESS + vsh_forgot_to_set_static_DETAIL


class lightmappedgeneric_flashlight_vs20_Dynamic_Index
{
	int m_nDOWATERFOG : 3;
#ifdef _DEBUG
	bool m_bDOWATERFOG : 1;
#endif	// _DEBUG
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif	// _DEBUG
	}

	lightmappedgeneric_flashlight_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = 0;
#ifdef _DEBUG
		m_bDOWATERFOG = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDOWATERFOG );
		return ( 1 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_lightmappedgeneric_flashlight_vs20 vsh_forgot_to_set_dynamic_DOWATERFOG


#endif	// LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H