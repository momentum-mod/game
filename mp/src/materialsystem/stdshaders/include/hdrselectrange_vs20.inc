// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef HDRSELECTRANGE_VS20_H
#define HDRSELECTRANGE_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class hdrselectrange_vs20_Static_Index
{
public:
	hdrselectrange_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_hdrselectrange_vs20 1


class hdrselectrange_vs20_Dynamic_Index
{
public:
	hdrselectrange_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_hdrselectrange_vs20 1


#endif	// HDRSELECTRANGE_VS20_H