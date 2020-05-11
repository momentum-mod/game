// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef COLORPICKER_BLEND_VS20_H
#define COLORPICKER_BLEND_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class colorpicker_blend_vs20_Static_Index
{
public:
	colorpicker_blend_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_colorpicker_blend_vs20 1


class colorpicker_blend_vs20_Dynamic_Index
{
public:
	colorpicker_blend_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_colorpicker_blend_vs20 1


#endif	// COLORPICKER_BLEND_VS20_H