// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_CLOAK_BLENDED_PASS_VS20_H
#define SDK_CLOAK_BLENDED_PASS_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_cloak_blended_pass_vs20_Static_Index
{
	int m_nBUMPMAP : 3;
#ifdef _DEBUG
	bool m_bBUMPMAP : 1;
#endif	// _DEBUG
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	sdk_cloak_blended_pass_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif	// _DEBUG
	}

	int GetIndex()
	{
		Assert( m_bBUMPMAP );
		return ( 4 * m_nBUMPMAP ) + 0;
	}
};

#define shaderStaticTest_sdk_cloak_blended_pass_vs20 vsh_forgot_to_set_static_BUMPMAP


class sdk_cloak_blended_pass_vs20_Dynamic_Index
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

	sdk_cloak_blended_pass_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex()
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + 0;
	}
};

#define shaderDynamicTest_sdk_cloak_blended_pass_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING


#endif	// SDK_CLOAK_BLENDED_PASS_VS20_H