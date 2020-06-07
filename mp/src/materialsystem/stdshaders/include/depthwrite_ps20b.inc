#ifndef DEPTHWRITE_PS20B_H
#define DEPTHWRITE_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class depthwrite_ps20b_Static_Index
{
	unsigned int m_nCOLOR_DEPTH : 2;
#ifdef _DEBUG
	bool m_bCOLOR_DEPTH : 1;
#endif	// _DEBUG
public:
	void SetCOLOR_DEPTH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLOR_DEPTH = i;
#ifdef _DEBUG
		m_bCOLOR_DEPTH = true;
#endif	// _DEBUG
	}

	depthwrite_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCOLOR_DEPTH = 0;
#ifdef _DEBUG
		m_bCOLOR_DEPTH = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOLOR_DEPTH );
		return ( 2 * m_nCOLOR_DEPTH ) + 0;
	}
};

#define shaderStaticTest_depthwrite_ps20b psh_forgot_to_set_static_COLOR_DEPTH


class depthwrite_ps20b_Dynamic_Index
{
	unsigned int m_nALPHACLIP : 2;
#ifdef _DEBUG
	bool m_bALPHACLIP : 1;
#endif	// _DEBUG
public:
	void SetALPHACLIP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nALPHACLIP = i;
#ifdef _DEBUG
		m_bALPHACLIP = true;
#endif	// _DEBUG
	}

	depthwrite_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nALPHACLIP = 0;
#ifdef _DEBUG
		m_bALPHACLIP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bALPHACLIP );
		return ( 1 * m_nALPHACLIP ) + 0;
	}
};

#define shaderDynamicTest_depthwrite_ps20b psh_forgot_to_set_dynamic_ALPHACLIP


#endif	// DEPTHWRITE_PS20B_H