#include "shaderlib/cshader.h"
class rml_draw_ps20b_Static_Index
{
public:
	rml_draw_ps20b_Static_Index( )
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderStaticTest_rml_draw_ps20b 0
class rml_draw_ps20b_Dynamic_Index
{
private:
	int m_nHAS_TEXTURE;
#ifdef _DEBUG
	bool m_bHAS_TEXTURE;
#endif
public:
	void SetHAS_TEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHAS_TEXTURE = i;
#ifdef _DEBUG
		m_bHAS_TEXTURE = true;
#endif
	}
	void SetHAS_TEXTURE( bool i )
	{
		m_nHAS_TEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bHAS_TEXTURE = true;
#endif
	}
public:
	rml_draw_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bHAS_TEXTURE = false;
#endif // _DEBUG
		m_nHAS_TEXTURE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bHAS_TEXTURE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nHAS_TEXTURE ) + 0;
	}
};
#define shaderDynamicTest_rml_draw_ps20b psh_forgot_to_set_dynamic_HAS_TEXTURE + 0
