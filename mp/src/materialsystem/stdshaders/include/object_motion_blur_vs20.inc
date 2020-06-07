// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef OBJECT_MOTION_BLUR_VS20_H
#define OBJECT_MOTION_BLUR_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class object_motion_blur_vs20_Static_Index
{
public:
	object_motion_blur_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_object_motion_blur_vs20 1


class object_motion_blur_vs20_Dynamic_Index
{
public:
	object_motion_blur_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_object_motion_blur_vs20 1


#endif	// OBJECT_MOTION_BLUR_VS20_H