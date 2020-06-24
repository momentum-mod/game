// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEPTH_OF_FIELD_VS20_H
#define DEPTH_OF_FIELD_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class depth_of_field_vs20_Static_Index
{
public:
	depth_of_field_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_depth_of_field_vs20 1


class depth_of_field_vs20_Dynamic_Index
{
public:
	depth_of_field_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_depth_of_field_vs20 1


#endif	// DEPTH_OF_FIELD_VS20_H