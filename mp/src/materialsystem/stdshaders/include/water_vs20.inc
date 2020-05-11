// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MULTITEXTURE && $BASETEXTURE
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef WATER_VS20_H
#define WATER_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class water_vs20_Static_Index
{
	int m_nBASETEXTURE : 3;
	int m_nMULTITEXTURE : 3;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bMULTITEXTURE : 1;
#endif	// _DEBUG
public:
	void SetBASETEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE = i;
#ifdef _DEBUG
		m_bBASETEXTURE = true;
#endif	// _DEBUG
	}

	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
#endif	// _DEBUG
	}

	water_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nMULTITEXTURE = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bMULTITEXTURE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bMULTITEXTURE );
		return ( 1 * m_nBASETEXTURE ) + ( 2 * m_nMULTITEXTURE ) + 0;
	}
};

#define shaderStaticTest_water_vs20 vsh_forgot_to_set_static_BASETEXTURE + vsh_forgot_to_set_static_MULTITEXTURE


class water_vs20_Dynamic_Index
{
public:
	water_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_water_vs20 1


#endif	// WATER_VS20_H