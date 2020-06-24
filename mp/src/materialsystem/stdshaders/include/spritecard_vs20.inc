// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SPRITECARD_VS20_H
#define SPRITECARD_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class spritecard_vs20_Static_Index
{
	unsigned int m_nZOOM_ANIMATE_SEQ2 : 2;
	unsigned int m_nDUALSEQUENCE : 2;
	unsigned int m_nEXTRACTGREENALPHA : 2;
	unsigned int m_nUSE_INSTANCING : 2;
#ifdef _DEBUG
	bool m_bZOOM_ANIMATE_SEQ2 : 1;
	bool m_bDUALSEQUENCE : 1;
	bool m_bEXTRACTGREENALPHA : 1;
	bool m_bUSE_INSTANCING : 1;
#endif	// _DEBUG
public:
	void SetZOOM_ANIMATE_SEQ2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nZOOM_ANIMATE_SEQ2 = i;
#ifdef _DEBUG
		m_bZOOM_ANIMATE_SEQ2 = true;
#endif	// _DEBUG
	}

	void SetDUALSEQUENCE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDUALSEQUENCE = i;
#ifdef _DEBUG
		m_bDUALSEQUENCE = true;
#endif	// _DEBUG
	}

	void SetEXTRACTGREENALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEXTRACTGREENALPHA = i;
#ifdef _DEBUG
		m_bEXTRACTGREENALPHA = true;
#endif	// _DEBUG
	}

	void SetUSE_INSTANCING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSE_INSTANCING = i;
#ifdef _DEBUG
		m_bUSE_INSTANCING = true;
#endif	// _DEBUG
	}

	spritecard_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nZOOM_ANIMATE_SEQ2 = 0;
		m_nDUALSEQUENCE = 0;
		m_nEXTRACTGREENALPHA = 0;
		m_nUSE_INSTANCING = 0;
#ifdef _DEBUG
		m_bZOOM_ANIMATE_SEQ2 = false;
		m_bDUALSEQUENCE = false;
		m_bEXTRACTGREENALPHA = false;
		m_bUSE_INSTANCING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bZOOM_ANIMATE_SEQ2 && m_bDUALSEQUENCE && m_bEXTRACTGREENALPHA && m_bUSE_INSTANCING );
		return ( 3 * m_nZOOM_ANIMATE_SEQ2 ) + ( 6 * m_nDUALSEQUENCE ) + ( 12 * m_nEXTRACTGREENALPHA ) + ( 24 * m_nUSE_INSTANCING ) + 0;
	}
};

#define shaderStaticTest_spritecard_vs20 vsh_forgot_to_set_static_ZOOM_ANIMATE_SEQ2 + vsh_forgot_to_set_static_DUALSEQUENCE + vsh_forgot_to_set_static_EXTRACTGREENALPHA + vsh_forgot_to_set_static_USE_INSTANCING


class spritecard_vs20_Dynamic_Index
{
	unsigned int m_nORIENTATION : 2;
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

	spritecard_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
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

#define shaderDynamicTest_spritecard_vs20 vsh_forgot_to_set_dynamic_ORIENTATION


#endif	// SPRITECARD_VS20_H