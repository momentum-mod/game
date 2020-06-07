// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef HDRCOMBINETO16BIT_VS20_H
#define HDRCOMBINETO16BIT_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class hdrcombineto16bit_vs20_Static_Index
{
public:
	hdrcombineto16bit_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_hdrcombineto16bit_vs20 1


class hdrcombineto16bit_vs20_Dynamic_Index
{
public:
	hdrcombineto16bit_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_hdrcombineto16bit_vs20 1


#endif	// HDRCOMBINETO16BIT_VS20_H