// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef BLURFILTER_VS20_H
#define BLURFILTER_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class blurfilter_vs20_Static_Index
{
	unsigned int m_nKERNEL : 2;
#ifdef _DEBUG
	bool m_bKERNEL : 1;
#endif	// _DEBUG
public:
	void SetKERNEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nKERNEL = i;
#ifdef _DEBUG
		m_bKERNEL = true;
#endif	// _DEBUG
	}

	blurfilter_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nKERNEL = 0;
#ifdef _DEBUG
		m_bKERNEL = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bKERNEL );
		return ( 1 * m_nKERNEL ) + 0;
	}
};

#define shaderStaticTest_blurfilter_vs20 vsh_forgot_to_set_static_KERNEL


class blurfilter_vs20_Dynamic_Index
{
public:
	blurfilter_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_blurfilter_vs20 1


#endif	// BLURFILTER_VS20_H