// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef BUFFERCLEAROBEYSTENCIL_VS20_H
#define BUFFERCLEAROBEYSTENCIL_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class bufferclearobeystencil_vs20_Static_Index
{
	unsigned int m_nUSESCOLOR : 2;
#ifdef _DEBUG
	bool m_bUSESCOLOR : 1;
#endif	// _DEBUG
public:
	void SetUSESCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSESCOLOR = i;
#ifdef _DEBUG
		m_bUSESCOLOR = true;
#endif	// _DEBUG
	}

	bufferclearobeystencil_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nUSESCOLOR = 0;
#ifdef _DEBUG
		m_bUSESCOLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bUSESCOLOR );
		return ( 1 * m_nUSESCOLOR ) + 0;
	}
};

#define shaderStaticTest_bufferclearobeystencil_vs20 vsh_forgot_to_set_static_USESCOLOR


class bufferclearobeystencil_vs20_Dynamic_Index
{
public:
	bufferclearobeystencil_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_bufferclearobeystencil_vs20 1


#endif	// BUFFERCLEAROBEYSTENCIL_VS20_H