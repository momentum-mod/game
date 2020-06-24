// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef WATERCHEAP_VS20_H
#define WATERCHEAP_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class watercheap_vs20_Static_Index
{
	unsigned int m_nBLEND : 2;
#ifdef _DEBUG
	bool m_bBLEND : 1;
#endif	// _DEBUG
public:
	void SetBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLEND = i;
#ifdef _DEBUG
		m_bBLEND = true;
#endif	// _DEBUG
	}

	watercheap_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBLEND = 0;
#ifdef _DEBUG
		m_bBLEND = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBLEND );
		return ( 1 * m_nBLEND ) + 0;
	}
};

#define shaderStaticTest_watercheap_vs20 vsh_forgot_to_set_static_BLEND


class watercheap_vs20_Dynamic_Index
{
public:
	watercheap_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_watercheap_vs20 1


#endif	// WATERCHEAP_VS20_H