// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef VR_DISTORT_TEXTURE_VS20_H
#define VR_DISTORT_TEXTURE_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class vr_distort_texture_vs20_Static_Index
{
public:
	vr_distort_texture_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_vr_distort_texture_vs20 1


class vr_distort_texture_vs20_Dynamic_Index
{
public:
	vr_distort_texture_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_vr_distort_texture_vs20 1


#endif	// VR_DISTORT_TEXTURE_VS20_H