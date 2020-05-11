// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEPTHWRITE_VS20_H
#define DEPTHWRITE_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class depthwrite_vs20_Static_Index
{
	int m_nONLY_PROJECT_POSITION : 2;
	int m_nCOLOR_DEPTH : 3;
#ifdef _DEBUG
	bool m_bONLY_PROJECT_POSITION : 1;
	bool m_bCOLOR_DEPTH : 1;
#endif	// _DEBUG
public:
	void SetONLY_PROJECT_POSITION( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nONLY_PROJECT_POSITION = i;
#ifdef _DEBUG
		m_bONLY_PROJECT_POSITION = true;
#endif	// _DEBUG
	}

	void SetCOLOR_DEPTH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLOR_DEPTH = i;
#ifdef _DEBUG
		m_bCOLOR_DEPTH = true;
#endif	// _DEBUG
	}

	depthwrite_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nONLY_PROJECT_POSITION = 0;
		m_nCOLOR_DEPTH = 0;
#ifdef _DEBUG
		m_bONLY_PROJECT_POSITION = false;
		m_bCOLOR_DEPTH = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bONLY_PROJECT_POSITION && m_bCOLOR_DEPTH );
		return ( 4 * m_nONLY_PROJECT_POSITION ) + ( 4 * m_nCOLOR_DEPTH ) + 0;
	}
};

#define shaderStaticTest_depthwrite_vs20 vsh_forgot_to_set_static_ONLY_PROJECT_POSITION + vsh_forgot_to_set_static_COLOR_DEPTH


class depthwrite_vs20_Dynamic_Index
{
	int m_nCOMPRESSED_VERTS : 3;
	int m_nSKINNING : 3;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
#endif	// _DEBUG
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif	// _DEBUG
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	depthwrite_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + 0;
	}
};

#define shaderDynamicTest_depthwrite_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING


#endif	// DEPTHWRITE_VS20_H