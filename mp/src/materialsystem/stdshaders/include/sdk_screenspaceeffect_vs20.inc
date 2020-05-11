// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_SCREENSPACEEFFECT_VS20_H
#define SDK_SCREENSPACEEFFECT_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_screenspaceeffect_vs20_Static_Index
{
	int m_nX360APPCHOOSER : 3;
public:
	void SetX360APPCHOOSER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nX360APPCHOOSER = i;
	}

	sdk_screenspaceeffect_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nX360APPCHOOSER = 0;
	}

	int GetIndex() const
	{
		return ( 1 * m_nX360APPCHOOSER ) + 0;
	}
};

#define shaderStaticTest_sdk_screenspaceeffect_vs20 1


class sdk_screenspaceeffect_vs20_Dynamic_Index
{
public:
	sdk_screenspaceeffect_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_sdk_screenspaceeffect_vs20 1


#endif	// SDK_SCREENSPACEEFFECT_VS20_H