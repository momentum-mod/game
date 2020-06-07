// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef RENDERTARGETBLIT_VS20_H
#define RENDERTARGETBLIT_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class rendertargetblit_vs20_Static_Index
{
public:
	rendertargetblit_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_rendertargetblit_vs20 1


class rendertargetblit_vs20_Dynamic_Index
{
public:
	rendertargetblit_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_rendertargetblit_vs20 1


#endif	// RENDERTARGETBLIT_VS20_H