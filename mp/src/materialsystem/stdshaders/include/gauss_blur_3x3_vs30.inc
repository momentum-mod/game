// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef GAUSS_BLUR_3X3_VS30_H
#define GAUSS_BLUR_3X3_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class gauss_blur_3x3_vs30_Static_Index
{
public:
	gauss_blur_3x3_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_gauss_blur_3x3_vs30 1


class gauss_blur_3x3_vs30_Dynamic_Index
{
public:
	gauss_blur_3x3_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_gauss_blur_3x3_vs30 1


#endif	// GAUSS_BLUR_3X3_VS30_H