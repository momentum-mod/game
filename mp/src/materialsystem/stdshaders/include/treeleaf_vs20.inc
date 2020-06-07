// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef TREELEAF_VS20_H
#define TREELEAF_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class treeleaf_vs20_Static_Index
{
	unsigned int m_nHALFLAMBERT : 2;
#ifdef _DEBUG
	bool m_bHALFLAMBERT : 1;
#endif	// _DEBUG
public:
	void SetHALFLAMBERT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALFLAMBERT = i;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
#endif	// _DEBUG
	}

	treeleaf_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nHALFLAMBERT = 0;
#ifdef _DEBUG
		m_bHALFLAMBERT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bHALFLAMBERT );
		return ( 4 * m_nHALFLAMBERT ) + 0;
	}
};

#define shaderStaticTest_treeleaf_vs20 vsh_forgot_to_set_static_HALFLAMBERT


class treeleaf_vs20_Dynamic_Index
{
	unsigned int m_nDYNAMIC_LIGHT : 2;
	unsigned int m_nSTATIC_LIGHT : 2;
#ifdef _DEBUG
	bool m_bDYNAMIC_LIGHT : 1;
	bool m_bSTATIC_LIGHT : 1;
#endif	// _DEBUG
public:
	void SetDYNAMIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDYNAMIC_LIGHT = i;
#ifdef _DEBUG
		m_bDYNAMIC_LIGHT = true;
#endif	// _DEBUG
	}

	void SetSTATIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSTATIC_LIGHT = i;
#ifdef _DEBUG
		m_bSTATIC_LIGHT = true;
#endif	// _DEBUG
	}

	treeleaf_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDYNAMIC_LIGHT = 0;
		m_nSTATIC_LIGHT = 0;
#ifdef _DEBUG
		m_bDYNAMIC_LIGHT = false;
		m_bSTATIC_LIGHT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDYNAMIC_LIGHT && m_bSTATIC_LIGHT );
		return ( 1 * m_nDYNAMIC_LIGHT ) + ( 2 * m_nSTATIC_LIGHT ) + 0;
	}
};

#define shaderDynamicTest_treeleaf_vs20 vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_STATIC_LIGHT


#endif	// TREELEAF_VS20_H