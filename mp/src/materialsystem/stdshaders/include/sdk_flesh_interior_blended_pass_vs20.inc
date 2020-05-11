// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $USE_STATIC_CONTROL_FLOW && ( $NUM_LIGHTS > 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_FLESH_INTERIOR_BLENDED_PASS_VS20_H
#define SDK_FLESH_INTERIOR_BLENDED_PASS_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_flesh_interior_blended_pass_vs20_Static_Index
{
	int m_nHALFLAMBERT : 3;
	int m_nUSE_STATIC_CONTROL_FLOW : 3;
#ifdef _DEBUG
	bool m_bHALFLAMBERT : 1;
	bool m_bUSE_STATIC_CONTROL_FLOW : 1;
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

	void SetUSE_STATIC_CONTROL_FLOW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSE_STATIC_CONTROL_FLOW = i;
#ifdef _DEBUG
		m_bUSE_STATIC_CONTROL_FLOW = true;
#endif	// _DEBUG
	}

	sdk_flesh_interior_blended_pass_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nHALFLAMBERT = 0;
		m_nUSE_STATIC_CONTROL_FLOW = 0;
#ifdef _DEBUG
		m_bHALFLAMBERT = false;
		m_bUSE_STATIC_CONTROL_FLOW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bHALFLAMBERT && m_bUSE_STATIC_CONTROL_FLOW );
		return ( 96 * m_nHALFLAMBERT ) + ( 192 * m_nUSE_STATIC_CONTROL_FLOW ) + 0;
	}
};

#define shaderStaticTest_sdk_flesh_interior_blended_pass_vs20 vsh_forgot_to_set_static_HALFLAMBERT + vsh_forgot_to_set_static_USE_STATIC_CONTROL_FLOW


class sdk_flesh_interior_blended_pass_vs20_Dynamic_Index
{
	int m_nCOMPRESSED_VERTS : 3;
	int m_nSKINNING : 3;
	int m_nDOWATERFOG : 3;
	int m_nDYNAMIC_LIGHT : 3;
	int m_nSTATIC_LIGHT : 3;
	int m_nNUM_LIGHTS : 3;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
	bool m_bDOWATERFOG : 1;
	bool m_bDYNAMIC_LIGHT : 1;
	bool m_bSTATIC_LIGHT : 1;
	bool m_bNUM_LIGHTS : 1;
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

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif	// _DEBUG
	}

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

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif	// _DEBUG
	}

	sdk_flesh_interior_blended_pass_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nDOWATERFOG = 0;
		m_nDYNAMIC_LIGHT = 0;
		m_nSTATIC_LIGHT = 0;
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
		m_bDOWATERFOG = false;
		m_bDYNAMIC_LIGHT = false;
		m_bSTATIC_LIGHT = false;
		m_bNUM_LIGHTS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING && m_bDOWATERFOG && m_bDYNAMIC_LIGHT && m_bSTATIC_LIGHT && m_bNUM_LIGHTS );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nDOWATERFOG ) + ( 8 * m_nDYNAMIC_LIGHT ) + ( 16 * m_nSTATIC_LIGHT ) + ( 32 * m_nNUM_LIGHTS ) + 0;
	}
};

#define shaderDynamicTest_sdk_flesh_interior_blended_pass_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_DOWATERFOG + vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_STATIC_LIGHT + vsh_forgot_to_set_dynamic_NUM_LIGHTS


#endif	// SDK_FLESH_INTERIOR_BLENDED_PASS_VS20_H