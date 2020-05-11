// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef WATERCHEAP_VS30_H
#define WATERCHEAP_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class watercheap_vs30_Static_Index
{
	int m_nBLEND : 3;
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

	watercheap_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
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

#define shaderStaticTest_watercheap_vs30 vsh_forgot_to_set_static_BLEND


class watercheap_vs30_Dynamic_Index
{
public:
	watercheap_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_watercheap_vs30 1


#endif	// WATERCHEAP_VS30_H