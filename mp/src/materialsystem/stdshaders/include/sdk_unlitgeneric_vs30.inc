// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_UNLITGENERIC_VS30_H
#define SDK_UNLITGENERIC_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_unlitgeneric_vs30_Static_Index
{
	int m_nVERTEXCOLOR : 3;
#ifdef _DEBUG
	bool m_bVERTEXCOLOR : 1;
#endif	// _DEBUG
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif	// _DEBUG
	}

	sdk_unlitgeneric_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nVERTEXCOLOR = 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bVERTEXCOLOR );
		return ( 8 * m_nVERTEXCOLOR ) + 0;
	}
};

#define shaderStaticTest_sdk_unlitgeneric_vs30 vsh_forgot_to_set_static_VERTEXCOLOR


class sdk_unlitgeneric_vs30_Dynamic_Index
{
	int m_nCOMPRESSED_VERTS : 3;
	int m_nDOWATERFOG : 3;
	int m_nSKINNING : 3;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bDOWATERFOG : 1;
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

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
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

	sdk_unlitgeneric_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nDOWATERFOG = 0;
		m_nSKINNING = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bDOWATERFOG = false;
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bDOWATERFOG && m_bSKINNING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nDOWATERFOG ) + ( 4 * m_nSKINNING ) + 0;
	}
};

#define shaderDynamicTest_sdk_unlitgeneric_vs30 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_DOWATERFOG + vsh_forgot_to_set_dynamic_SKINNING


#endif	// SDK_UNLITGENERIC_VS30_H