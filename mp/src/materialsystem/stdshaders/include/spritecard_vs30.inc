// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SPRITECARD_VS30_H
#define SPRITECARD_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class spritecard_vs30_Static_Index
{
public:
	spritecard_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_spritecard_vs30 1


class spritecard_vs30_Dynamic_Index
{
	int m_nORIENTATION : 3;
#ifdef _DEBUG
	bool m_bORIENTATION : 1;
#endif	// _DEBUG
public:
	void SetORIENTATION( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nORIENTATION = i;
#ifdef _DEBUG
		m_bORIENTATION = true;
#endif	// _DEBUG
	}

	spritecard_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
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

#define shaderDynamicTest_spritecard_vs30 vsh_forgot_to_set_dynamic_ORIENTATION


#endif	// SPRITECARD_VS30_H