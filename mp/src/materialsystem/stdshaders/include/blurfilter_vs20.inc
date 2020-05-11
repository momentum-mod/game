// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef BLURFILTER_VS20_H
#define BLURFILTER_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class blurfilter_vs20_Static_Index
{
public:
	blurfilter_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_blurfilter_vs20 1


class blurfilter_vs20_Dynamic_Index
{
public:
	blurfilter_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_blurfilter_vs20 1


#endif	// BLURFILTER_VS20_H