// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef PARTICLESPHERE_VS20_H
#define PARTICLESPHERE_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class particlesphere_vs20_Static_Index
{
public:
	particlesphere_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_particlesphere_vs20 1


class particlesphere_vs20_Dynamic_Index
{
	unsigned int m_nFOGTYPE : 2;
#ifdef _DEBUG
	bool m_bFOGTYPE : 1;
#endif	// _DEBUG
public:
	void SetFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFOGTYPE = i;
#ifdef _DEBUG
		m_bFOGTYPE = true;
#endif	// _DEBUG
	}

	particlesphere_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFOGTYPE = 0;
#ifdef _DEBUG
		m_bFOGTYPE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFOGTYPE );
		return ( 1 * m_nFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_particlesphere_vs20 vsh_forgot_to_set_dynamic_FOGTYPE


#endif	// PARTICLESPHERE_VS20_H